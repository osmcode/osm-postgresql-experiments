
# OSM-PostgreSQL-Experiments

This is some experimental code for importing OSM data into a PostgreSQL/PostGIS
database. It is meant as a super-flexible tool for advanced OSM and database
users who know the ins and outs of the OSM data model and as a test bed for
implementing new ideas in getting OSM data into PostgreSQL/PostGIS. This is
definitely not a tool for casual users.

This program doesn't actually talk to the PostgreSQL database. Instead it
creates files in the PostgreSQL COPY format containing the data and SQL files
with the commands to create the tables and import the data. This allows you to
change anything or add extra steps anywhere along the import process giving you
even more flexibility.

## Build

```
mkdir build
cd build
cmake ..
make
```

## Usage

Generally you call it as: `ope [OPTIONS] OSMFILE OUTPUT-TABLE...`.

Use `src/ope --help` to get usage information and a list of options.

Data is read from `OSMFILE` and written to one or more output tables. Each
output table is specified using the format `FILENAME=STREAM%COLUMNS`. The
`FILENAME` is the filename to be used. If it doesn't have a suffix, `.pgcopy`
is used. A file with suffix `.sql` is also written containing the data
definitions used.

The `STREAM` defines what kind of data should be written into the table. It can
be one of the following:

* o: objects (nodes, ways, and relations)
* n: nodes
* w: ways
* r: relations
* a: areas
* oT: tags from all objects
* nT: tags from nodes
* wT: tags from ways
* rT: tags from relation
* wN: nodes from ways
* rM: members from relations
* u: users
* c: changesets
* cT: tags from changesets
* cC: comments from changesets

The `COLUMNS` define the columns to be written out. Each column is specified
by two characters, the first specifies the column type, the second the format.
For instance `I.` creates an object id (`I`) column in the default format (`.`)
which is a BIGINT. Some column types only have a single format, others allow
multiple formats.

Look for `column_config` in `src/table.cpp` to see all column types/formats.

If the `COLUMNS` is not specified a default for this stream is used. The
default depends on whether the `--with-history/-H` flag is used or not.

The "users" stream is somewhat special. It will generate a row for each unique
user id encountered while generating any of the other tables specified. This
allows you to have user ids in all tables and a lookup table to get the user
names from these.

## Command line options

* `-f, --filter FILTER`: Only import data that matches the filter expresssion.
  Currently the only supported filter is `with-tags`, ie. objects without
  tags are ignored.
* `-h, --help`: Show usage information.
* `-v, --verbose`: Enable verbose mode.
* `-H, --with-history`: The input file contains history data, ie. there can
  be several versions of the same object in it.

## License

Copyright (C) 2020  Jochen Topf (jochen@topf.org)

This program is available under the GNU GENERAL PUBLIC LICENSE Version 3.
See the file LICENSE.txt for the complete text of the license.


## Authors

This program was written and is maintained by Jochen Topf (jochen@topf.org).


