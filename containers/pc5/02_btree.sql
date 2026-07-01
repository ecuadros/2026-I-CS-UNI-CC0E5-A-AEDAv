-- Igualdad exacta
SELECT id, pabellon, capacidad FROM ambientes WHERE capacidad = 120;

-- Rango abierto (justo lo que un Hash NO puede hacer)
SELECT id, pabellon, capacidad FROM ambientes WHERE capacidad > 900 ORDER BY capacidad LIMIT 10;

-- Rango cerrado con BETWEEN
SELECT id, pabellon, capacidad FROM ambientes WHERE capacidad BETWEEN 30 AND 60 ORDER BY capacidad LIMIT 10;

-- Conjunto de valores con IN
SELECT id, pabellon, capacidad FROM ambientes WHERE capacidad IN (30, 45, 120, 500) ORDER BY capacidad LIMIT 10;

-- ORDER BY "gratis": el indice ya esta ordenado, el LIMIT no reordena la tabla
SELECT id, pabellon, capacidad FROM ambientes ORDER BY capacidad DESC LIMIT 5;

-- Segundo B-Tree: rango sobre la columna de fecha
SELECT id, pabellon, inaugurado_at
FROM ambientes
WHERE inaugurado_at > NOW() - INTERVAL '30 days'
ORDER BY inaugurado_at DESC
LIMIT 10;
