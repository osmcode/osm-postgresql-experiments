
# OSM-PostgreSQL-Experiments

This is some experimental code for importing OSM data into a PostgreSQL/PostGIS
database.

## Build

```
mkdir build
cd build
cmake ..
make
```

## Usage

Use `src/ope OSMFILE >data.pgcopy` to create a file in PostgreSQL COPY format.
Import with `psql` using the `\copy` command.

See comments in `src/main.cpp` for information on used table formats etc.

## License

Copyright (C) 2019  Jochen Topf (jochen@topf.org)

This program is available under the GNU GENERAL PUBLIC LICENSE Version 3.
See the file LICENSE.txt for the complete text of the license.


## Authors

This program was written and is maintained by Jochen Topf (jochen@topf.org).



