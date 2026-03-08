
#include "formatting.hpp"

#include "util.hpp"

#include <format>

void add_null(std::string &buffer)
{
    std::string_view const null{"\\N"};
    buffer.append(null.begin(), null.end());
}

void add_char(std::string &buffer, char c) { buffer.append(&c, std::next(&c)); }

void add_bool(std::string &buffer, bool value, char true_value,
              char false_value)
{
    add_char(buffer, value ? true_value : false_value);
}

void add_tags_json(std::string &buffer, osmium::TagList const &tags)
{
    rapidjson::StringBuffer stream;
    rapidjson::Writer<rapidjson::StringBuffer> writer{stream};

    writer.StartObject();
    for (auto const &tag : tags) {
        writer.Key(tag.key());
        writer.String(tag.value());
    }
    writer.EndObject();

    append_pg_escaped(buffer, stream.GetString(), stream.GetSize());
}

namespace {

std::string escape_hstore(char const *str)
{
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

} // anonymous namespace

void add_tags_hstore(std::string &buffer, osmium::TagList const &tags)
{
    if (tags.empty()) {
        return;
    }
    std::string data;

    for (auto const &tag : tags) {
        data += escape_hstore(tag.key());
        data += "=>";
        data += escape_hstore(tag.value());
        data += ',';
    }
    data.resize(data.size() - 1);

    append_pg_escaped(buffer, data.c_str());
}

void add_way_nodes_array(std::string &buffer, osmium::WayNodeList const &nodes)
{
    add_char(buffer, '{');

    bool delimiter = false;
    for (auto const &nr : nodes) {
        if (delimiter) {
            add_char(buffer, ',');
        } else {
            delimiter = true;
        }
        std::format_to(std::back_inserter(buffer), "{}", nr.ref());
    }

    add_char(buffer, '}');
}

namespace {

bool needs_quoting(char const *str) noexcept
{
    while (*str) {
        if (!std::isalnum(*str) && *str != '_' && *str != ':') {
            return true;
        }
        ++str;
    }
    return false;
}

std::string escape_str(char const *str)
{
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

} // anonymous namespace

void add_members_type(std::string &buffer,
                      osmium::RelationMemberList const &members)
{
    add_char(buffer, '{');

    bool delimiter = false;
    for (auto const &member : members) {
        if (delimiter) {
            add_char(buffer, ',');
        } else {
            delimiter = true;
        }

        std::format_to(std::back_inserter(buffer), "\"({},{},",
                       osmium::item_type_to_char(member.type()), member.ref());
        if (needs_quoting(member.role())) {
            std::format_to(std::back_inserter(buffer), R"FOO(\\")FOO");
            auto const escaped_role = escape_str(member.role());
            append_pg_escaped(buffer, escaped_role.c_str());
            std::format_to(std::back_inserter(buffer), R"FOO(\\")")FOO");
        } else {
            std::format_to(std::back_inserter(buffer), "{})\"", member.role());
        }
    }

    add_char(buffer, '}');
}

void add_members_json(std::string &buffer,
                      osmium::RelationMemberList const &members)
{
    rapidjson::StringBuffer stream;
    rapidjson::Writer<rapidjson::StringBuffer> writer{stream};

    char typebuffer[2] = "x";
    writer.StartArray();
    for (auto const &member : members) {
        writer.StartObject();
        writer.Key("type");
        typebuffer[0] = osmium::item_type_to_char(member.type());
        writer.String(typebuffer);
        writer.Key("ref");
        writer.Int64(member.ref());
        writer.Key("role");
        writer.String(member.role());
        writer.EndObject();
    }
    writer.EndArray();

    append_pg_escaped(buffer, stream.GetString(), stream.GetSize());
}
