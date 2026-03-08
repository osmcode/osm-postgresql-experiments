#pragma once

#include "formatting.hpp"
#include "options.hpp"
#include "util.hpp"

#include <osmium/geom/wkb.hpp>
#include <osmium/index/id_set.hpp>
#include <osmium/osm.hpp>

#include <cassert>
#include <fcntl.h>
#include <format>
#include <fstream>
#include <memory>
#include <string>
#include <string_view>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <utility>
#include <vector>

enum class stream_type : char
{
    objects = 0,
    tags = 1,
    way_nodes = 2,
    members = 3,
    users = 4,
    changeset = 5,
    changeset_tags = 6,
    changeset_comments = 7
};

std::string print_streams();

struct stream_config_type
{
    std::string stream;
    std::string name;
    std::string without_history;
    std::string with_history;
    stream_type stype;
    osmium::osm_entity_bits::type entities;
};

enum class column_type
{

    objtype,
    id,
    version,
    deleted,
    visible,
    changeset,
    timestamp_iso,
    timestamp_sec,
    timestamp_range,
    uid,
    user,
    tags_jsonb,
    tags_json,
    tags_hstore,
    tag_seq,
    tag_key,
    tag_value,
    tag_kv,
    lon_real,
    lon_int,
    lat_real,
    lat_int,
    quadtile,
    nodes_array,
    node_seq,
    node_ref,
    members_jsonb,
    members_json,
    members_type,
    member_seq,
    member_type_char,
    member_type_enum,
    member_ref,
    member_role,
    orig_id,
    orig_type,
    geometry,
    geometry_point,
    geometry_linestring,
    geometry_polygon,
    redaction,
    num_changes,
    comments_count,
    open,
    created_at_iso,
    closed_at_iso,
    created_at_sec,
    closed_at_sec,
    max_lon_real,
    max_lon_int,
    max_lat_real,
    max_lat_int,
    bounds_box2d,
    bounds_polygon,
    comment_text,

}; // enum class column_type

enum sql_column_config_flags : unsigned int
{
    none = 0x00,
    geom_index = 0x01,
    location_store = 0x02,
    time_range = 0x04,
    nwr_enum = 0x08,
    rel_member = 0x10,
    hstore = 0x20,
    postgis = 0x40,
    assemble_areas = 0x80
};

struct column_config_type
{
    char const *format_string;
    column_type format;
    std::string sql_name;
    std::string sql_type;
    sql_column_config_flags flags;
};

class Table
{

    std::string m_filename{};
    std::string m_columns_string{};
    std::string m_name{};
    std::string m_path{};
    stream_config_type const *m_stream_config;
    sql_column_config_flags m_column_flags = none;
    int m_fd = -1;
    bool m_delimiter = false;

protected:
    std::vector<column_config_type> m_columns;
    std::string m_buffer;

public:
    Table(std::string filename, stream_config_type const &stream_config,
          std::string columns_string);

    Table(Table const &) = delete;
    Table &operator=(Table const &) = delete;

    Table(Table &&) = default;
    Table &operator=(Table &&) = default;

    virtual ~Table() = default;

    virtual void add_row(osmium::OSMObject const & /*object*/,
                         osmium::Timestamp const /*next_version_timestamp*/)
    {
    }

    virtual void add_changeset_row(osmium::Changeset const & /*changeset*/) {}

    void close();

    std::string const &name() const noexcept { return m_name; }

    std::string const &filename() const noexcept { return m_filename; }

    std::string const &stream_name() const noexcept
    {
        return m_stream_config->name;
    }

    virtual osmium::osm_entity_bits::type read_entities() const noexcept
    {
        return m_stream_config->entities;
    }

    bool matches(osmium::item_type const objtype) const noexcept
    {
        return m_stream_config->entities &
               osmium::osm_entity_bits::from_item_type(objtype);
    }

    std::string const &columns_string() const noexcept
    {
        return m_columns_string;
    }

    sql_column_config_flags column_flags() const noexcept
    {
        return m_column_flags;
    }

    void flush();

    void possible_flush()
    {
        if (m_buffer.size() > 1000 * 1024) {
            flush();
        }
    }

    void start_column()
    {
        static std::string_view const tab{"\t"};
        if (m_delimiter) {
            m_buffer.append(tab.begin(), tab.end());
        } else {
            m_delimiter = true;
        }
    }

    void end_row()
    {
        static std::string_view const newline{"\n"};
        m_buffer.append(newline.begin(), newline.end());
        m_delimiter = false;
    }

private:
    void setup_columns();

public:
    void sql_data_definition() const;

    virtual std::string sql_primary_key() const;

}; // class Table

class ObjectsTable : public Table
{

    osmium::geom::WKBFactory<> m_factory{osmium::geom::wkb_type::ewkb,
                                         osmium::geom::out_type::hex};

public:
    ObjectsTable(std::string const &filename,
                 stream_config_type const &stream_config,
                 std::string const &columns_string)
    : Table(filename, stream_config, columns_string)
    {
    }

    void add_row(osmium::OSMObject const &object,
                 osmium::Timestamp const next_version_timestamp) override;

}; // class ObjectsTable

class TagsTable : public Table
{

public:
    TagsTable(std::string const &filename,
              stream_config_type const &stream_config,
              std::string const &columns_string)
    : Table(filename, stream_config, columns_string)
    {
    }

    void add_row(osmium::OSMObject const &object,
                 osmium::Timestamp const next_version_timestamp) override;

}; // class TagsTable

class WayNodesTable : public Table
{

public:
    WayNodesTable(std::string const &filename,
                  stream_config_type const &stream_config,
                  std::string const &columns_string)
    : Table(filename, stream_config, columns_string)
    {
    }

    void add_row(osmium::OSMObject const &object,
                 osmium::Timestamp const next_version_timestamp) override;

}; // class WayNodesTable

class MembersTable : public Table
{

public:
    MembersTable(std::string const &filename,
                 stream_config_type const &stream_config,
                 std::string const &columns_string)
    : Table(filename, stream_config, columns_string)
    {
    }

    void add_row(osmium::OSMObject const &object,
                 osmium::Timestamp const next_version_timestamp) override;

}; // class MembersTable

class UsersTable : public Table
{

    osmium::index::IdSetDense<osmium::user_id_type> m_user_ids;

public:
    UsersTable(std::string const &filename,
               stream_config_type const &stream_config,
               std::string const &columns_string)
    : Table(filename, stream_config, columns_string)
    {
    }

    std::string sql_primary_key() const override;

    void add_row(osmium::OSMObject const &object,
                 osmium::Timestamp const next_version_timestamp) override;

    osmium::osm_entity_bits::type read_entities() const noexcept override
    {
        return osmium::osm_entity_bits::nothing;
    }

}; // class UsersTable

class ChangesetsTable : public Table
{

    osmium::geom::WKBFactory<> m_factory{osmium::geom::wkb_type::ewkb,
                                         osmium::geom::out_type::hex};

public:
    ChangesetsTable(std::string const &filename,
                    stream_config_type const &stream_config,
                    std::string const &columns_string)
    : Table(filename, stream_config, columns_string)
    {
    }

    std::string sql_primary_key() const override;

    void add_changeset_row(osmium::Changeset const &changeset) override;

}; // class ChangesetsTable

class ChangesetTagsTable : public Table
{

public:
    ChangesetTagsTable(std::string const &filename,
                       stream_config_type const &stream_config,
                       std::string const &columns_string)
    : Table(filename, stream_config, columns_string)
    {
    }

    std::string sql_primary_key() const override;

    void add_changeset_row(osmium::Changeset const &changeset) override;

}; // class ChangesetTagsTable

class ChangesetCommentsTable : public Table
{

public:
    ChangesetCommentsTable(std::string const &filename,
                           stream_config_type const &stream_config,
                           std::string const &columns_string)
    : Table(filename, stream_config, columns_string)
    {
    }

    std::string sql_primary_key() const override;

    void add_changeset_row(osmium::Changeset const &changeset) override;

}; // class ChangesetCommentsTable

std::unique_ptr<Table> create_table(Options const &opts,
                                    std::string const &config_string);
