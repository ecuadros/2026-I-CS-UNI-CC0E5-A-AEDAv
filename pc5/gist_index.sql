CREATE INDEX idx_estudiantes_ubicacion ON estudiantes_uni USING gist (ubicacion);

-- Contención (<@): Estudiantes que estan dentro de un cuadrante específico.
SELECT nombre, codigo, ubicacion
FROM estudiantes_uni
WHERE ubicacion <@ box(point(100, 100), point(200, 200));

-- K-Nearest Neighbors (KNN) con el operador de distancia (<->)
-- Busca a los 5 estudiantes mas cercanos a una coordenada dada
SELECT nombre, codigo, ubicacion <-> point(500, 500) AS distancia_metros
FROM estudiantes_uni
ORDER BY ubicacion <-> point(500, 500) 
LIMIT 5;

-- Igualdad espacial (~=)
SELECT nombre, codigo
FROM estudiantes_uni
WHERE ubicacion ~= point(750, 250);