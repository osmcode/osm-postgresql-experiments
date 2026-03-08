#pragma once

#include <osmium/osm/entity_bits.hpp>

#include <cstddef>
#include <limits>
#include <string>
#include <utility>

/**
 * Split input string at delimiter and return the two parts.
 *
 * If the delimiter is not there, return the whole input string as the first
 * string and use the default_2nd as the second.
 */
std::pair<std::string, std::string> split(std::string const &input,
                                          char delimiter,
                                          std::string const &default_2nd = "");

void append_pg_escaped(std::string &buffer, char const *str, std::size_t size);
void append_pg_escaped(std::string &buffer, char const *str);

std::string list_entities(osmium::osm_entity_bits::type entities);

char const *yes_no(bool choice) noexcept;
