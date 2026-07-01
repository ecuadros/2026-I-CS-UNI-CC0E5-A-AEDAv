-- GiST = el "R-Tree" espacial (Postgres no tiene R-Tree nativo).
-- Opera sobre geometria: contencion, interseccion, distancia.

-- Contencion (<@): dentro de una caja cerca del origen
SELECT id, zona, ubicacion
FROM sensores
WHERE ubicacion <@ BOX(POINT(-5, -5), POINT(5, 5))
ORDER BY id;

-- KNN (<->): los 3 mas cercanos al origen
SELECT id, zona, ubicacion, ubicacion <-> POINT(0, 0) AS distancia
FROM sensores
ORDER BY ubicacion <-> POINT(0, 0)
LIMIT 3;

-- Igualdad espacial (~=): exactamente en (2,1)
SELECT id, zona FROM sensores WHERE ubicacion ~= POINT(2, 1);

-- Contiene (@>): dentro de un circulo de radio 10 en el origen
SELECT id, zona, ubicacion
FROM sensores
WHERE CIRCLE(POINT(0, 0), 10) @> ubicacion
ORDER BY id;
