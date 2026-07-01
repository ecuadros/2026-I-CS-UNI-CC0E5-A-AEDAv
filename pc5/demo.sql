-- pc5
INSERT INTO autos (placa, marca, modelo, anio, color, ubicacion) VALUES
    ('V7Q-284', 'Toyota', 'Corolla', 2018, 'gris', POINT(-12.0193, -77.0498)),
    ('B3L-902', 'Hyundai', 'Accent', 2021, 'azul', POINT(-12.0719, -77.0792)),
    ('M9R-417', 'Kia', 'Rio', 2017, 'rojo', POINT(-12.0464, -77.0428)),
    ('T2X-665', 'Nissan', 'Sentra', 2023, 'negro', POINT(-12.1318, -77.0303)),
    ('H8C-031', 'Mazda', 'CX-5', 2020, 'blanco', POINT(-12.0676, -77.0334));

-- pc5
ANALYZE autos;

-- pc5
SET enable_seqscan = off;

-- btree
EXPLAIN ANALYZE
SELECT id, placa, marca, modelo, anio
FROM autos
WHERE anio >= 2020
ORDER BY anio DESC;

-- hash
EXPLAIN ANALYZE
SELECT id, placa, marca, modelo
FROM autos
WHERE placa = 'B3L-902';

-- espacial
EXPLAIN ANALYZE
SELECT id, placa, marca, ubicacion
FROM autos
WHERE ubicacion <@ BOX(POINT(-12.10, -77.08), POINT(-12.00, -77.00));

-- espacial
EXPLAIN ANALYZE
SELECT id, placa, marca, ubicacion, ubicacion <-> POINT(-12.05, -77.04) AS distancia
FROM autos
ORDER BY ubicacion <-> POINT(-12.05, -77.04)
LIMIT 3;

-- pc5
SELECT indexname, indexdef
FROM pg_indexes
WHERE tablename = 'autos'
ORDER BY indexname;
