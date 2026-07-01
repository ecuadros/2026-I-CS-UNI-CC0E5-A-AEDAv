-- EXPLAIN ANALYZE: una consulta por indice, para ver el plan real.
-- Index Scan / Bitmap Index Scan = uso el indice; Seq Scan = leyo toda la tabla.

-- B-Tree: rango numerico
EXPLAIN ANALYZE
SELECT id, zona, lectura FROM sensores WHERE lectura > 90000 ORDER BY lectura;

-- B-Tree: rango temporal + ORDER BY
EXPLAIN ANALYZE
SELECT id, zona, instalado_at
FROM sensores
WHERE instalado_at > NOW() - INTERVAL '1 hour'
ORDER BY instalado_at DESC;

-- Hash: igualdad exacta
EXPLAIN ANALYZE
SELECT id, zona, codigo FROM sensores WHERE codigo = md5('sensor-uni-001');

-- GiST: contencion
EXPLAIN ANALYZE
SELECT id, zona, ubicacion FROM sensores WHERE ubicacion <@ BOX(POINT(-5, -5), POINT(5, 5));

-- GiST: KNN
EXPLAIN ANALYZE
SELECT id, zona FROM sensores ORDER BY ubicacion <-> POINT(0, 0) LIMIT 3;

-- Anti-ejemplo: rango sobre columna hash -> Seq Scan
EXPLAIN ANALYZE
SELECT count(*) FROM sensores WHERE codigo > md5('sensor-uni-001');
