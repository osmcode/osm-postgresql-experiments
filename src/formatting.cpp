
#include "formatting.hpp"

#include "json-writer.hpp"
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
    json_writer writer;

    writer.start_object();
    for (auto const &tag : tags) {
        writer.key(tag.key());
        writer.string(tag.value());
        writer.next();
    }
    writer.end_object();

    append_pg_escaped(buffer, writer.json().c_str());
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
    json_writer writer;

    char typebuffer[2] = "x";
    writer.start_array();
    for (auto const &member : members) {
        writer.start_object();
        writer.key("type");
        typebuffer[0] = osmium::item_type_to_char(member.type());
        writer.string(typebuffer);
        writer.next();
        writer.key("ref");
        writer.number(member.ref());
        writer.next();
        writer.key("role");
        writer.string(member.role());
        writer.end_object();
        writer.next();
    }
    writer.end_array();

    append_pg_escaped(buffer, writer.json().c_str());
}
