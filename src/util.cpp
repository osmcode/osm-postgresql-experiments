
#include "util.hpp"

std::pair<std::string, std::string> split(const std::string& input, char delimiter, const std::string& default_2nd) {
    const auto pos = input.find_first_of(delimiter);
    if (pos == std::string::npos) {
        return std::make_pair(input, default_2nd);
    }
    return std::make_pair(input.substr(0, pos), input.substr(pos + 1));
}

namespace {

void escape_char(std::string& buffer, const char c) noexcept {
    const std::string_view backslash{"\\\\"};
    const std::string_view newline{"\\n"};
    const std::string_view cr{"\\r"};
    const std::string_view tab{"\\t"};

    switch (c) {
        case '\\':
            buffer.append(backslash.begin(), backslash.end());
            break;
        case '\n':
            buffer.append(newline.begin(), newline.end());
            break;
        case '\r':
            buffer.append(cr.begin(), cr.end());
            break;
        case '\t':
            buffer.append(tab.begin(), tab.end());
            break;
        default:
            buffer.append(&c, std::next(&c));
    }
}

} // anonymous namespace

void append_pg_escaped(std::string& buffer, const char* str, std::size_t size) {
    while (size-- > 0 && *str != '\0') {
        escape_char(buffer, *str);
        ++str;
    }
}

void append_pg_escaped(std::string& buffer, const char* str) {
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

