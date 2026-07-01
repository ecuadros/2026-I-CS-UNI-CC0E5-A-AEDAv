DROP TABLE IF EXISTS ambientes CASCADE;

CREATE TABLE ambientes (
    id            SERIAL      PRIMARY KEY,               -- btree implicito (PK)
    codigo        TEXT        NOT NULL,                  -- HASH. TEXT y no CHAR(n): md5() da text y el hash sobre CHAR arrastra el relleno de espacios.
    pabellon      VARCHAR(50) NOT NULL,
    capacidad     BIGINT      NOT NULL,                  -- BTREE (aforo)
    inaugurado_at TIMESTAMP   NOT NULL DEFAULT NOW(),    -- BTREE (fecha)
    ubicacion     POINT       NOT NULL                   -- GIST (posicion en campus)
);

-- Ambientes "ancla" con nombre, cerca del origen (0,0) = Biblioteca Central.
-- Son los objetivos concretos de las consultas espaciales (04_gist.sql).
INSERT INTO ambientes (codigo, pabellon, capacidad, inaugurado_at, ubicacion) VALUES
    (md5('FC-A1-101'),        'Ciencias', 45,  NOW() - INTERVAL '2 years',  POINT( 0,  0)),
    (md5('FIIS-B2-201'),      'Sistemas', 120, NOW() - INTERVAL '18 days',  POINT( 1,  1)),
    (md5('FC-LAB-COMPUTO'),   'Ciencias', 30,  NOW() - INTERVAL '5 days',   POINT( 3, -2)),
    (md5('BIB-CENTRAL-SALA'), 'Central',  500, NOW() - INTERVAL '6 years',  POINT(-1, -1)),
    (md5('FIM-C3-301'),       'Mecanica', 80,  NOW() - INTERVAL '3 years',  POINT(120, 80));

-- Relleno determinista para dar volumen (el planificador prefiere indices en
-- consultas selectivas cuando la tabla no cabe en un vistazo).
INSERT INTO ambientes (codigo, pabellon, capacidad, inaugurado_at, ubicacion)
SELECT md5('aula-' || g),
       'pab-' || (g % 8),
       20 + (g * 7919) % 980,                             -- aforo 20..999
       NOW() - (g || ' days')::interval,                  -- inauguraciones en 8 anios
       POINT((g % 400) - 200, (g % 300) - 150)            -- malla en [-200,200)x[-150,150)
FROM generate_series(1, 3000) AS g;

CREATE INDEX idx_ambientes_codigo_hash     ON ambientes USING hash  (codigo);
CREATE INDEX idx_ambientes_capacidad_btree ON ambientes USING btree (capacidad);
CREATE INDEX idx_ambientes_inaug_btree     ON ambientes USING btree (inaugurado_at);
CREATE INDEX idx_ambientes_ubic_gist       ON ambientes USING gist  (ubicacion);

ANALYZE ambientes;

-- indices creados sobre la tabla
SELECT indexname, indexdef
FROM pg_indexes
WHERE tablename = 'ambientes'
ORDER BY indexname;
