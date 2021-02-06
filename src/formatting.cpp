
#include "formatting.hpp"
#include "util.hpp"

void add_null(fmt::memory_buffer& buffer) {
    const fmt::string_view null{"\\N"};
    buffer.append(null.begin(), null.end());
}

void add_char(fmt::memory_buffer& buffer, char c) {
    buffer.append(&c, std::next(&c));
}

void add_bool(fmt::memory_buffer& buffer, bool value, char tv, char fv) {
    if (value) {
        add_char(buffer, tv);
    } else {
        add_char(buffer, fv);
    }
}

void add_tags_json(fmt::memory_buffer& buffer, const osmium::TagList& tags) {
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

static std::string escape_hstore(const char* str) {
    std::string result{"\""};

    while (*str) {
        if (*str == '"') {
            result += "\\\"";
        } else if (*str == '\\') {
            result += "\\\\";
        } else {
            result += *str;
        }
        ++str;
    }

    result += "\"";
    return result;
}

void add_tags_hstore(fmt::memory_buffer& buffer, const osmium::TagList& tags) {
    if (tags.empty()) {
        return;

    }
    std::string data;

    for (const auto& tag : tags) {
        data += escape_hstore(tag.key());
        data += "=>";
        data += escape_hstore(tag.value());
        data += ',';
    }
    data.resize(data.size() - 1);

    append_pg_escaped(buffer, data.c_str());
}

void add_way_nodes_array(fmt::memory_buffer& buffer, const osmium::WayNodeList& nodes) {
    add_char(buffer, '{');

    bool delimiter = false;
    for (const auto& nr : nodes) {
        if (delimiter) {
            add_char(buffer, ',');
        } else {
            delimiter = true;
        }
        fmt::format_to(buffer, "{}", nr.ref());
    }

    add_char(buffer, '}');
}

static bool needs_quoting(const char* str) noexcept {
    while (*str) {
        if (!std::isalnum(*str) && *str != '_' && *str != ':') {
            return true;
        }
        ++str;
    }
    return false;
}

static std::string escape_str(const char* str) {
    std::string result;

    while (*str) {
        if (*str == '"') {
            result += R"(\\\")";
        } else if (*str == ',') {
            result += "\\,";
        } else if (*str == '\\') {
            result += R"(\\\\)";
        } else {
            result += *str;
        }
        ++str;
    }

    return result;
}

void add_members_type(fmt::memory_buffer& buffer, const osmium::RelationMemberList& members) {
    add_char(buffer, '{');

    bool delimiter = false;
    for (const auto& rm : members) {
        if (delimiter) {
            add_char(buffer, ',');
        } else {
            delimiter = true;
        }

        fmt::format_to(buffer, "\"({},{},", osmium::item_type_to_char(rm.type()), rm.ref());
        if (needs_quoting(rm.role())) {
            fmt::format_to(buffer, R"FOO(\\")FOO");
            const auto escaped_role = escape_str(rm.role());
            append_pg_escaped(buffer, escaped_role.c_str());
            fmt::format_to(buffer, R"FOO(\\")")FOO");
        } else {
            fmt::format_to(buffer, "{})\"", rm.role());
        }
    }

    add_char(buffer, '}');
}

void add_members_json(fmt::memory_buffer& buffer, const osmium::RelationMemberList& members) {
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

