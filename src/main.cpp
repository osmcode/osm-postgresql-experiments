
#include <osmium/geom/wkb.hpp>
#include <osmium/handler.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/osm/item_type.hpp>
#include <osmium/visitor.hpp>

#ifndef RAPIDJSON_HAS_STDSTRING
# define RAPIDJSON_HAS_STDSTRING 1
#endif
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include <iostream>
#include <string>

/*

CREATE TYPE rel_member AS (
    objtype CHAR(1),
    id      BIGINT,
    role    TEXT
);

DROP TABLE IF EXISTS osmdata CASCADE;

CREATE TABLE osmdata (
    objtype   CHAR(1),
    id        BIGINT,
    version   INTEGER,
    visible   BOOL,
    changeset INTEGER,
    tstamp    TIMESTAMP (0) WITH TIME ZONE,
    uid       INTEGER,
    username  TEXT,
    tags      JSONB,
    geom      GEOMETRY,
    way_nodes BIGINT[],
    members   rel_member[]
);

-- ---------------------------

\copy osmdata from `data.pgcopy`

-- ---------------------------


CREATE OR REPLACE VIEW osmnodes
    AS SELECT * FROM osmdata WHERE objtype ='n';

CREATE OR REPLACE VIEW osmways
    AS SELECT * FROM osmdata WHERE objtype ='w';

CREATE OR REPLACE VIEW osmrelations
    AS SELECT * FROM osmdata WHERE objtype ='r';

-- ---------------------------

WITH nids AS (
         SELECT unnest(way_nodes) AS nid, id AS wid FROM osmways wu
     ),
     wgeoms AS (
         SELECT ST_MakeLine(n.geom) AS wgeom, i.wid FROM osmnodes n, nids i WHERE n.id = i.nid GROUP BY i.wid
     )
UPDATE osmdata w SET geom = wgeom
    FROM wgeoms
    WHERE objtype = 'w' AND wgeoms.wid =w.id;

-- ---------------------------

DROP TABLE IF EXISTS nodes;

CREATE TABLE nodes (
    id        BIGINT PRIMARY KEY,
    version   INTEGER,
    visible   BOOL,
    changeset INTEGER,
    tstamp    TIMESTAMP (0) WITH TIME ZONE,
    uid       INTEGER,
    username  TEXT,
    tags      JSONB,
    geom      GEOMETRY(POINT, 4326)
);

INSERT INTO nodes
    SELECT id, version, visible, changeset, tstamp, uid, username, tags, geom
        FROM osmnodes WHERE tags != '{}';

-- ---------------------------

DROP TABLE IF EXISTS ways;

CREATE TABLE ways (
    id        BIGINT PRIMARY KEY,
    version   INTEGER,
    visible   BOOL,
    changeset INTEGER,
    tstamp    TIMESTAMP (0) WITH TIME ZONE,
    uid       INTEGER,
    username  TEXT,
    tags      JSONB,
    geom      GEOMETRY(LINESTRING, 4326),
    way_nodes BIGINT[]
);

INSERT INTO ways
    SELECT id, version, visible, changeset, tstamp, uid, username, tags, geom, way_nodes
        FROM osmways;

*/

class Handler : public osmium::handler::Handler {

    osmium::geom::WKBFactory<> m_factory{osmium::geom::wkb_type::ewkb, osmium::geom::out_type::hex};
    std::string m_buffer;

    void finalize() {
        m_buffer += '\n';

        if (m_buffer.size() > 1000 * 1024) {
            std::cout << m_buffer;
            m_buffer.resize(0);
        }
    }

    void append_pg_escaped(const char* str, std::size_t size = std::numeric_limits<std::size_t>::max()) {
        while (size-- > 0 && *str != '\0') {
            switch (*str) {
                case '\\':
                    m_buffer += '\\';
                    m_buffer += '\\';
                    break;
                case '\n':
                    m_buffer += '\\';
                    m_buffer += 'n';
                    break;
                case '\r':
                    m_buffer += '\\';
                    m_buffer += 'r';
                    break;
                case '\t':
                    m_buffer += '\\';
                    m_buffer += 't';
                    break;
                default:
                    m_buffer += *str;
            }
            ++str;
        }
    }

    void add_tags(const osmium::OSMObject& object) {
        rapidjson::StringBuffer stream;
        rapidjson::Writer<rapidjson::StringBuffer> writer{stream};

        writer.StartObject();
        for (const auto& tag : object.tags()) {
            writer.Key(tag.key());
            writer.String(tag.value());
        }
        writer.EndObject();

        append_pg_escaped(stream.GetString(), stream.GetSize());
    }

    void common(const osmium::OSMObject& object) {
        m_buffer.append(std::to_string(object.id()));
        m_buffer += '\t';
        m_buffer.append(std::to_string(object.version()));
        m_buffer += '\t';
        m_buffer += object.visible() ? 't' : 'f';
        m_buffer += '\t';
        m_buffer.append(std::to_string(object.changeset()));
        m_buffer += '\t';
        m_buffer.append(object.timestamp().to_iso());
        m_buffer += '\t';
        m_buffer.append(std::to_string(object.uid()));
        m_buffer += '\t';
        append_pg_escaped(object.user());
        m_buffer += '\t';
        add_tags(object);
        m_buffer += '\t';
    }

    void way_nodes(const osmium::Way& way) {
        m_buffer += '{';
        for (const auto& nr : way.nodes()) {
            m_buffer.append(std::to_string(nr.ref()));
            m_buffer += ',';
        }
        if (m_buffer.back() == ',') {
            m_buffer.back() = '}';
        } else {
            m_buffer += '}';
        }
        m_buffer += '\t';
    }

    void members(const osmium::Relation& relation) {
        m_buffer += '{';
        for (const auto& rm : relation.members()) {
            m_buffer += "\"(";
            m_buffer += osmium::item_type_to_char(rm.type());
            m_buffer += ',';
            m_buffer.append(std::to_string(rm.ref()));
            m_buffer += ",\\\\\"";
            append_pg_escaped(rm.role());
            m_buffer += "\\\\\")\",";
        }
        if (m_buffer.back() == ',') {
            m_buffer.back() = '}';
        } else {
            m_buffer += '}';
        }
    }

public:

    Handler() {
        m_buffer.reserve(1024 * 1024);
    }

    void node(const osmium::Node& node) {
        m_buffer += "n\t";
        common(node);

        // geom
        if (node.location()) {
            m_buffer.append(m_factory.create_point(node));
            m_buffer += '\t';
        } else {
            m_buffer += "\\N\t";
        }

        // nodes
        m_buffer += "\\N\t";

        // members
        m_buffer += "\\N";

        finalize();
    }

    void way(const osmium::Way& way) {
        m_buffer += "w\t";
        common(way);

        // geom
        m_buffer += "\\N\t";

        // nodes
        way_nodes(way);

        // members
        m_buffer += "\\N";

        finalize();
    }

    void relation(const osmium::Relation& relation) {
        m_buffer += "r\t";
        common(relation);

        // geom
        m_buffer += "\\N\t";

        // nodes
        m_buffer += "\\N\t";

        // members
        members(relation);

        finalize();
    }

}; // class Handler

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " INFILE\n";
        std::exit(1);
    }

    const std::string input_filename{argv[1]};

    Handler handler;

    osmium::io::Reader reader{input_filename};

    while (osmium::memory::Buffer buffer = reader.read()) {
        osmium::apply(buffer, handler);
    }

    reader.close();
}

