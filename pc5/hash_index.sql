-- Índice Hash
-- Funcionamiento: aplica una función hash a la columna indexada para mapear cada valor
-- a una dirección en una tabla hash. Las búsquedas por igualdad son O(1) en promedio,
-- ya que calculan el hash y acceden directamente a la ubicación. No mantiene orden
-- alguno, por lo que no sirve para rangos ni ORDER BY.
--
-- Casos de uso principales:
--   • Búsquedas exclusivamente por igualdad (=)
--   • Columnas con valores únicos o casi únicos (ej. email, username)
--   • Tablas con pocas escrituras concurrentes (el mantenimiento del hash es ligero)
--   • Situaciones donde no se necesitan consultas por rango ni ordenamiento
--
-- Nota: desde PostgreSQL 10 los índices Hash son WAL-logged y crash-safe.

CREATE TABLE usuarios (
    id SERIAL PRIMARY KEY,
    email VARCHAR(200) NOT NULL,
    username VARCHAR(50) NOT NULL,
    password_hash VARCHAR(255) NOT NULL
);

INSERT INTO usuarios (email, username, password_hash) VALUES
    ('alice@example.com', 'alice', 'hash1'),
    ('bob@example.com', 'bob', 'hash2'),
    ('carol@example.com', 'carol', 'hash3'),
    ('dave@example.com', 'dave', 'hash4'),
    ('eve@example.com', 'eve', 'hash5');

-- Índice Hash sobre email, columna que solo se busca por igualdad exacta
CREATE INDEX idx_usuarios_email ON usuarios USING HASH (email);

-- Consulta que se beneficia del índice Hash: búsqueda exacta
EXPLAIN ANALYZE
SELECT id, username
FROM usuarios
WHERE email = 'bob@example.com';
