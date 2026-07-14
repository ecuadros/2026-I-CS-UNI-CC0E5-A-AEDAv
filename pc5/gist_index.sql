-- Índice GiST (Generalized Search Tree)
-- Funcionamiento: es un árbol balanceado genérico que permite implementar operadores
-- de búsqueda personalizados para tipos de datos complejos. A diferencia del BTree,
-- no requiere un orden total; en su lugar usa estrategias de "penalización" y
-- "división" definidas por la clase de operador. Soporta intersección, contención,
-- distancia y proximidad.
--
-- Casos de uso principales:
--   • Datos geométricos y espaciales (points, polygons, lines) con operadores como
--     <@, &&, ~, ~=
--   • Búsqueda por rango con tipos range (daterange, int4range, tsrange)
--   • Búsqueda de texto completo con tsvector y tsquery (alternativa a GIN)
--   • Búsqueda por similitud con pg_trgm (trigramas)
--   • Datos multidimensionales (cube, earthdistance)

CREATE TABLE ubicaciones (
    id SERIAL PRIMARY KEY,
    nombre VARCHAR(100) NOT NULL,
    coordenada POINT NOT NULL
);

INSERT INTO ubicaciones (nombre, coordenada) VALUES
    ('Parque Central',      POINT(-77.0369, -12.0464)),
    ('Plaza Mayor',         POINT(-77.0335, -12.0450)),
    ('Museo de Arte',       POINT(-77.0400, -12.0500)),
    ('Biblioteca Nacional', POINT(-77.0300, -12.0480)),
    ('Estadio Nacional',    POINT(-77.0250, -12.0550));

-- Índice GiST sobre la columna point para habilitar búsquedas espaciales
CREATE INDEX idx_ubicaciones_coordenada ON ubicaciones USING GIST (coordenada);

-- Consulta que se beneficia del índice GiST: puntos contenidos en un rectángulo
EXPLAIN ANALYZE
SELECT nombre, coordenada
FROM ubicaciones
WHERE coordenada <@ BOX(POINT(-77.0500, -12.0600), POINT(-77.0200, -12.0400));
