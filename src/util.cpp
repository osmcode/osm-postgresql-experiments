
#include "util.hpp"

std::pair<std::string, std::string> split(const std::string& input, char delimiter, const std::string& default_2nd) {
    const auto pos = input.find_first_of(delimiter);
    if (pos == std::string::npos) {
        return std::make_pair(input, default_2nd);
    }
    return std::make_pair(input.substr(0, pos), input.substr(pos + 1));
}

static void escape_char(fmt::memory_buffer& buffer, const char c) noexcept {
    switch (c) {
        case '\\':
            fmt::format_to(buffer, "\\\\");
            break;
        case '\n':
            fmt::format_to(buffer, "\\n");
            break;
        case '\r':
            fmt::format_to(buffer, "\\r");
            break;
        case '\t':
            fmt::format_to(buffer, "\\t");
            break;
        default:
            buffer.append(&c, std::next(&c));
    }
}

void append_pg_escaped(fmt::memory_buffer& buffer, const char* str, std::size_t size) {
    while (size-- > 0 && *str != '\0') {
        escape_char(buffer, *str);
        ++str;
    }
}

void append_pg_escaped(fmt::memory_buffer& buffer, const char* str) {
    while (*str != '\0') {
        escape_char(buffer, *str);
        ++str;
    }
}

std::string list_entities(const osmium::osm_entity_bits::type entities) {
    std::string output;

    if (entities & osmium::osm_entity_bits::node) {
        output += "nodes, ";
    }
    if (entities & osmium::osm_entity_bits::way) {
        output += "ways, ";
    }
    if (entities & osmium::osm_entity_bits::relation) {
        output += "relations, ";
    }
    if (entities & osmium::osm_entity_bits::changeset) {
        output += "changesets, ";
    }

    assert(output.size() > 2);
    output.resize(output.size() - 2);

    return output;
}

const char* yes_no(const bool choice) noexcept {
    return choice ? "yes\n" : "no\n";
}

