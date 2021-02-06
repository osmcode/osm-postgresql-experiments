#pragma once

#include "format.hpp"

#include <osmium/osm/relation.hpp>
#include <osmium/osm/tag.hpp>
#include <osmium/osm/way.hpp>

#ifndef RAPIDJSON_HAS_STDSTRING
# define RAPIDJSON_HAS_STDSTRING 1
#endif
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include <string>

void add_tags_json(fmt::memory_buffer& buffer, const osmium::TagList& tags);

void add_tags_hstore(fmt::memory_buffer& buffer, const osmium::TagList& tags);

void add_way_nodes_array(fmt::memory_buffer& buffer, const osmium::WayNodeList& nodes);

void add_members_type(fmt::memory_buffer& buffer, const osmium::RelationMemberList& members);
void add_members_json(fmt::memory_buffer& buffer, const osmium::RelationMemberList& members);

