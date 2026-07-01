-- Hash: solo igualdad exacta (=). No sirve para rangos ni ORDER BY.

-- Busqueda exacta por token
SELECT id, zona, codigo FROM sensores WHERE codigo = md5('sensor-uni-001');

-- Otra igualdad
SELECT zona, lectura FROM sensores WHERE codigo = md5('sensor-lima-007');

-- Anti-ejemplo: un rango sobre codigo no usa el hash (cae en Seq Scan).
SELECT count(*) FROM sensores WHERE codigo > md5('sensor-uni-001');
