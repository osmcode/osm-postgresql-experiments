
#include "formatting.hpp"
#include "util.hpp"

void add_tags_json(std::string& buffer, const osmium::TagList& tags) {
    rapidjson::StringBuffer stream;
    rapidjson::Writer<rapidjson::StringBuffer> writer{stream};

    writer.StartObject();
    for (const auto& tag : tags) {
        writer.Key(tag.key());
        writer.String(tag.value());
    }
    writer.EndObject();

    append_pg_escaped(buffer, stream.GetString(), stream.GetSize());
}

void add_way_nodes_array(std::string& buffer, const osmium::WayNodeList& nodes) {
    buffer += '{';
    for (const auto& nr : nodes) {
        buffer.append(std::to_string(nr.ref()));
        buffer += ',';
    }
    if (buffer.back() == ',') {
        buffer.back() = '}';
    } else {
        buffer += '}';
    }
}

// TODO: fix escaping
void add_members_row(std::string& buffer, const osmium::RelationMemberList& members) {
    buffer += '{';
    for (const auto& rm : members) {
        buffer += "\"(";
        buffer += osmium::item_type_to_char(rm.type());
        buffer += ',';
        buffer.append(std::to_string(rm.ref()));
        buffer += ",\\\\\"";
        append_pg_escaped(buffer, rm.role());
        buffer += "\\\\\")\",";
    }
    if (buffer.back() == ',') {
        buffer.back() = '}';
    } else {
        buffer += '}';
    }
}

void add_members_json(std::string& buffer, const osmium::RelationMemberList& members) {
    rapidjson::StringBuffer stream;
    rapidjson::Writer<rapidjson::StringBuffer> writer{stream};

    char typebuffer[2] = "x";
    writer.StartArray();
    for (const auto& rm : members) {
        writer.StartObject();
        writer.Key("type");
        typebuffer[0] = osmium::item_type_to_char(rm.type());
        writer.String(typebuffer);
        writer.Key("ref");
        writer.Int64(rm.ref());
        writer.Key("role");
        writer.String(rm.role());
        writer.EndObject();
    }
    writer.EndArray();

    append_pg_escaped(buffer, stream.GetString(), stream.GetSize());
}

