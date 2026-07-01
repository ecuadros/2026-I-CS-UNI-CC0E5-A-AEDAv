CREATE INDEX idx_estudiantes_codigo ON estudiantes_uni USING hash (codigo);

-- Busqueda exacta
EXPLAIN ANALYZE 
SELECT nombre, codigo, promedio 
FROM estudiantes_uni 
WHERE codigo = '20210345I';

-- limite de la arquitectura Hash
-- hace un Seq Scan.
EXPLAIN ANALYZE 
SELECT nombre, codigo, promedio 
FROM estudiantes_uni 
WHERE codigo LIKE '2021%';