-- Índice BTree (Balanced Tree)
-- Funcionamiento: organiza los datos en una estructura de árbol balanceado donde cada nodo
-- contiene un rango de valores y punteros a nodos hijos. Las búsquedas recorren el árbol
-- desde la raíz hasta la hoja en O(log n). Soporta recorridos ordenados porque las hojas
-- están enlazadas secuencialmente.
--
-- Casos de uso principales:
--   • Columnas con alta cardinalidad (muchos valores distintos)
--   • Comparaciones de igualdad (=)
--   • Comparaciones de rango (<, >, <=, >=, BETWEEN)
--   • Ordenamiento (ORDER BY)
--   • Búsquedas por prefijo (LIKE 'abc%')
--   • Único tipo de índice que soporte restricciones UNIQUE y PRIMARY KEY

CREATE TABLE productos (
    id SERIAL PRIMARY KEY,
    nombre VARCHAR(100) NOT NULL,
    precio DECIMAL(10,2) NOT NULL,
    stock INTEGER NOT NULL
);

INSERT INTO productos (nombre, precio, stock) VALUES
    ('Laptop', 2500.00, 10),
    ('Mouse', 45.50, 150),
    ('Teclado', 120.00, 80),
    ('Monitor', 800.00, 25),
    ('Audifonos', 60.00, 200);

-- Índice BTree sobre precio para acelerar consultas por rango
CREATE INDEX idx_productos_precio ON productos USING BTREE (precio);

-- Consulta que se beneficia del índice BTree: escaneo por rango + ordenamiento
EXPLAIN ANALYZE
SELECT nombre, precio
FROM productos
WHERE precio BETWEEN 100 AND 1000
ORDER BY precio;
