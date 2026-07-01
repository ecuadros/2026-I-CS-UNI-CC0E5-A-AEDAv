CREATE INDEX idx_estudiantes_promedio ON estudiantes_uni USING btree (promedio);

-- Igualdad
SELECT * FROM estudiantes_uni WHERE promedio = 15.50;

-- Rango Simple
SELECT * FROM estudiantes_uni WHERE promedio > 18.00;

-- BETWEEN
SELECT * FROM estudiantes_uni WHERE promedio BETWEEN 14.00 AND 16.00;

-- Ordenamiento
SELECT nombre, promedio 
FROM estudiantes_uni 
ORDER BY promedio DESC 
LIMIT 10;