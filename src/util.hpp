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
std::pair<std::string, std::string> split(const std::string& input, char delimiter, const std::string& default_2nd = "");

void append_pg_escaped(std::string& buffer, const char* str, std::size_t size);
void append_pg_escaped(std::string& buffer, const char* str);

std::string list_entities(osmium::osm_entity_bits::type entities);

const char* yes_no(bool choice) noexcept;

