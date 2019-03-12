
#include "util.hpp"

std::pair<std::string, std::string> split(const std::string& input, char delimiter, const std::string& default_2nd) {
    const auto pos = input.find_first_of(delimiter);
    if (pos == std::string::npos) {
        return std::make_pair(input, default_2nd);
    }
    return std::make_pair(input.substr(0, pos), input.substr(pos + 1));
}

void append_pg_escaped(std::string& buffer, const char* str, std::size_t size) {
    while (size-- > 0 && *str != '\0') {
        switch (*str) {
            case '\\':
                buffer += '\\';
                buffer += '\\';
                break;
            case '\n':
                buffer += '\\';
                buffer += 'n';
                break;
            case '\r':
                buffer += '\\';
                buffer += 'r';
                break;
            case '\t':
                buffer += '\\';
                buffer += 't';
                break;
            default:
                buffer += *str;
        }
        ++str;
    }
}

void append_pg_escaped(std::string& buffer, const char* str) {
    while (*str != '\0') {
        switch (*str) {
            case '\\':
                buffer += '\\';
                buffer += '\\';
                break;
            case '\n':
                buffer += '\\';
                buffer += 'n';
                break;
            case '\r':
                buffer += '\\';
                buffer += 'r';
                break;
            case '\t':
                buffer += '\\';
                buffer += 't';
                break;
            default:
                buffer += *str;
        }
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

    assert(output.size() > 2);
    output.resize(output.size() - 2);

    return output;
}

