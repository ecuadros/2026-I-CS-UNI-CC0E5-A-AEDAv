-- PC5: tabla unica con una columna por tipo de indice.
DROP TABLE IF EXISTS sensores CASCADE;

CREATE TABLE sensores (
    id           SERIAL       PRIMARY KEY,
    codigo       TEXT         NOT NULL,   -- token MD5 -> hash (TEXT, no CHAR: ver nota abajo)
    zona         VARCHAR(50)  NOT NULL,
    lectura      BIGINT       NOT NULL,   -- -> btree
    instalado_at TIMESTAMP    NOT NULL DEFAULT NOW(),  -- -> btree
    ubicacion    POINT        NOT NULL    -- -> gist
);

-- Filas con nombre cerca del origen (objetivos de las demos espaciales).
INSERT INTO sensores (codigo, zona, lectura, instalado_at, ubicacion) VALUES
    (md5('sensor-central-01'), 'Centro', 500,   NOW() - INTERVAL '5 minutes',  POINT(0, 0)),
    (md5('sensor-norte-02'),   'Norte',  800,   NOW() - INTERVAL '12 minutes', POINT(2, 1)),
    (md5('sensor-sur-03'),     'Sur',    150,   NOW() - INTERVAL '20 minutes', POINT(3, -2)),
    (md5('sensor-este-04'),    'Este',   95000, NOW() - INTERVAL '40 minutes', POINT(-4, 4)),
    (md5('sensor-oeste-05'),   'Oeste',  40,    NOW() - INTERVAL '2 hours',    POINT(50, 50)),
    (md5('sensor-lejano-06'),  'Borde',  99999, NOW() - INTERVAL '3 days',     POINT(120, 80)),
    (md5('sensor-uni-001'),    'Rimac',  1200,  NOW() - INTERVAL '10 minutes', POINT(1, 1)),
    (md5('sensor-lima-007'),   'Centro', 33000, NOW() - INTERVAL '1 hour',     POINT(-1, -1));

-- 1000 filas de relleno para que el planificador prefiera los indices.
INSERT INTO sensores (codigo, zona, lectura, instalado_at, ubicacion)
SELECT md5('bulk-' || g),
       'zona-' || (g % 8),
       (g * 7919) % 100000,
       NOW() - (g || ' minutes')::interval,
       POINT((g % 400) - 200, (g % 300) - 150)
FROM generate_series(1, 1000) AS g;

CREATE INDEX idx_sensores_codigo_hash     ON sensores USING hash  (codigo);
CREATE INDEX idx_sensores_lectura_btree   ON sensores USING btree (lectura);
CREATE INDEX idx_sensores_instalado_btree ON sensores USING btree (instalado_at);
CREATE INDEX idx_sensores_ubic_gist       ON sensores USING gist  (ubicacion);

ANALYZE sensores;

SELECT indexname, indexdef
FROM pg_indexes
WHERE tablename = 'sensores'
ORDER BY indexname;
