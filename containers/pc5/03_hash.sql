-- Igualdad por codigo de ambiente (el caso ideal del hash)
SELECT id, pabellon, codigo FROM ambientes WHERE codigo = md5('FIIS-B2-201');

-- Otra igualdad exacta
SELECT id, pabellon, capacidad FROM ambientes WHERE codigo = md5('BIB-CENTRAL-SALA');

-- Anti-ejemplo: un rango sobre la columna hash NO puede usar el indice.
SELECT count(*) FROM ambientes WHERE codigo > md5('FIIS-B2-201');
