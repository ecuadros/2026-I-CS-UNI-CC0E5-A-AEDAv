-- B-Tree: rango numerico
EXPLAIN ANALYZE
SELECT id, pabellon, capacidad FROM ambientes WHERE capacidad > 900 ORDER BY capacidad;

-- B-Tree: rango de fecha + ORDER BY (orden gratis -> Index Scan Backward, sin Sort)
EXPLAIN ANALYZE
SELECT id, pabellon, inaugurado_at
FROM ambientes
WHERE inaugurado_at > NOW() - INTERVAL '30 days'
ORDER BY inaugurado_at DESC;

-- Hash: igualdad exacta
EXPLAIN ANALYZE
SELECT id, pabellon FROM ambientes WHERE codigo = md5('FIIS-B2-201');

-- GiST: contencion en caja
EXPLAIN ANALYZE
SELECT id FROM ambientes WHERE ubicacion <@ BOX(POINT(-5, -5), POINT(5, 5));

-- GiST: KNN (vecinos mas cercanos)
EXPLAIN ANALYZE
SELECT id FROM ambientes ORDER BY ubicacion <-> POINT(0, 0) LIMIT 3;

-- Anti-ejemplo: rango sobre la columna con indice HASH -> Seq Scan (el hash no ayuda)
EXPLAIN ANALYZE
SELECT count(*) FROM ambientes WHERE codigo > md5('FIIS-B2-201');
