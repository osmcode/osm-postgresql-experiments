
#include "util.hpp"

std::pair<std::string, std::string>
split(std::string const &input, char delimiter, std::string const &default_2nd)
{
    auto const pos = input.find_first_of(delimiter);
    if (pos == std::string::npos) {
        return std::make_pair(input, default_2nd);
    }
    return std::make_pair(input.substr(0, pos), input.substr(pos + 1));
}

namespace {

void escape_char(std::string &buffer, char const c) noexcept
{
    std::string_view const backslash{"\\\\"};
    std::string_view const newline{"\\n"};
    std::string_view const carriage_return{"\\r"};
    std::string_view const tab{"\\t"};

    switch (c) {
    case '\\':
        buffer.append(backslash.begin(), backslash.end());
        break;
    case '\n':
        buffer.append(newline.begin(), newline.end());
        break;
    case '\r':
        buffer.append(carriage_return.begin(), carriage_return.end());
        break;
    case '\t':
        buffer.append(tab.begin(), tab.end());
        break;
    default:
        buffer.append(&c, std::next(&c));
    }
}

} // anonymous namespace

void append_pg_escaped(std::string &buffer, char const *str, std::size_t size)
{
    while (size-- > 0 && *str != '\0') {
        escape_char(buffer, *str);
        ++str;
    }
}

void append_pg_escaped(std::string &buffer, char const *str)
{
    while (*str != '\0') {
        escape_char(buffer, *str);
        ++str;
    }
}

std::string list_entities(osmium::osm_entity_bits::type const entities)
{
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

char const *yes_no(bool const choice) noexcept
{
    return choice ? "yes\n" : "no\n";
}
