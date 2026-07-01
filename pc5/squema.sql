DROP TABLE IF EXISTS estudiantes_uni;

CREATE TABLE estudiantes_uni (
    id          SERIAL PRIMARY KEY,
    nombre      VARCHAR(50) NOT NULL,
    codigo      CHAR(9) UNIQUE NOT NULL,
    promedio    DECIMAL(4,2),
    ubicacion   POINT NOT NULL
);

INSERT INTO estudiantes_uni (nombre, codigo, promedio, ubicacion)
SELECT 
    'Estudiante ' || i,

    -- Año (2000 en adelante) || Examen (0 o 1) || Numero (000-999) || Letra (A-Z)
    (2000 + (i / 20000))::TEXT || 
    (i % 2)::TEXT || 
    LPAD(((i / 2) % 1000)::TEXT, 3, '0') || 
    CHR(65 + (i % 26)),

    (random() * 10 + 10)::DECIMAL(4,2),
    point(random() * 1000, random() * 1000)
FROM generate_series(1, 1000000) as i;