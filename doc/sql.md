
# SQL snippets

Here are some SQL snippets that should be turned into real docs at some point.

## Handling of timezone ranges

```
UPDATE osmdata u SET trange = tstzrange(lower(u.trange), lower(f.trange))
    FROM osmdata f
    WHERE u.objtype = f.objtype AND u.id = f.id AND u.version + 1 = f.version;


-- SELECT count(*) FROM osmdata WHERE upper(trange) IS NULL;
-- SELECT count(*) FROM osmdata WHERE trange @> CAST('2016-01-01 00:00:00+00:00' AS TIMESTAMP WITH TIME ZONE);

CREATE OR REPLACE VIEW osmdata_current
    AS SELECT * FROM osmdata
        WHERE upper(trange) IS NULL;

-- ---------------------------

UPDATE hist h1 SET trange = tstzrange(lower(h1.trange), (SELECT MIN(lower(h2.trange))
    FROM hist h2 WHERE h2.id = h1.id AND h2.objtype = h1.objtype AND h2.version > h1.version AND lower(h2.trange) >= lower(h1.trange)));
```

## Create way geometries from nodes

```
WITH nids AS (
         SELECT unnest(way_nodes) AS nid, id AS wid FROM osmways wu
     ),
     wgeoms AS (
         SELECT ST_MakeLine(n.geom) AS wgeom, i.wid FROM osmnodes n, nids i WHERE n.id = i.nid GROUP BY i.wid
     )
UPDATE osmways w SET geom = wgeom
    FROM wgeoms
    WHERE objtype = 'w' AND wgeoms.wid =w.id;
```

