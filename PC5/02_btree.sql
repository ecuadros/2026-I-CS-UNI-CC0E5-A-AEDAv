-- B-Tree: igualdad, rangos, IN y ORDER BY (mantiene las claves ordenadas).

-- Igualdad
SELECT id, zona, lectura FROM sensores WHERE lectura = 1200;

-- Rango (lo que Hash no puede)
SELECT id, zona, lectura FROM sensores WHERE lectura > 90000 ORDER BY lectura;

-- BETWEEN
SELECT id, zona, lectura FROM sensores WHERE lectura BETWEEN 100 AND 1000 ORDER BY lectura;

-- IN
SELECT id, zona, lectura FROM sensores WHERE lectura IN (40, 150, 500, 800);

-- ORDER BY: el indice ya esta ordenado, evita el sort
SELECT id, zona, lectura FROM sensores ORDER BY lectura DESC LIMIT 5;

-- Rango temporal sobre el segundo btree
SELECT id, zona, instalado_at
FROM sensores
WHERE instalado_at > NOW() - INTERVAL '1 hour'
ORDER BY instalado_at DESC;
