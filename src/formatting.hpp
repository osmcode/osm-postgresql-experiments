#pragma once

#include <osmium/osm/relation.hpp>
#include <osmium/osm/tag.hpp>
#include <osmium/osm/way.hpp>

#ifndef RAPIDJSON_HAS_STDSTRING
#define RAPIDJSON_HAS_STDSTRING 1
#endif
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <string>

void add_null(std::string &buffer);

void add_char(std::string &buffer, char c);

void add_bool(std::string &buffer, bool value, char true_value = 't',
              char false_value = 'f');

void add_tags_json(std::string &buffer, osmium::TagList const &tags);

void add_tags_hstore(std::string &buffer, osmium::TagList const &tags);

void add_way_nodes_array(std::string &buffer, osmium::WayNodeList const &nodes);

void add_members_type(std::string &buffer,
                      osmium::RelationMemberList const &members);
void add_members_json(std::string &buffer,
                      osmium::RelationMemberList const &members);
