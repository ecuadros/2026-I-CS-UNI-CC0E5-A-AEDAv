-- Contencion en caja: ambientes dentro del rectangulo (-5,-5)-(5,5)
SELECT id, pabellon, ubicacion
FROM ambientes
WHERE ubicacion <@ BOX(POINT(-5, -5), POINT(5, 5))
ORDER BY id;

-- KNN: los 3 ambientes mas cercanos a la biblioteca, ordenados por distancia
SELECT id, pabellon, ubicacion, round((ubicacion <-> POINT(0, 0))::numeric, 3) AS distancia
FROM ambientes
ORDER BY ubicacion <-> POINT(0, 0)
LIMIT 3;

-- Igualdad espacial (~=): el ambiente que esta exactamente en (1,1)
SELECT id, pabellon FROM ambientes WHERE ubicacion ~= POINT(1, 1);

-- Contencion en circulo (@>): ambientes dentro del circulo de radio 10 en el origen
SELECT id, pabellon, ubicacion
FROM ambientes
WHERE CIRCLE(POINT(0, 0), 10) @> ubicacion
ORDER BY id;
