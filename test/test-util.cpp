
#include <catch.hpp>

#include "util.hpp"

#include <cstring>

TEST_CASE("split with delimiter")
{
    auto const p = split("foo=bar", '=', "default");
    REQUIRE(p.first == "foo");
    REQUIRE(p.second == "bar");
}

TEST_CASE("split without delimiter")
{
    auto const p = split("foo", '=', "default");
    REQUIRE(p.first == "foo");
    REQUIRE(p.second == "default");
}

TEST_CASE("list_entities")
{
    auto const e =
        osmium::osm_entity_bits::node | osmium::osm_entity_bits::relation;

    REQUIRE(list_entities(e) == "nodes, relations");
}

TEST_CASE("yesno")
{
    REQUIRE_FALSE(std::strcmp(yes_no(true), "yes\n"));
    REQUIRE_FALSE(std::strcmp(yes_no(false), "no\n"));
}
