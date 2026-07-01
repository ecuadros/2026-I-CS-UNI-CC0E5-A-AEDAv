-- pc5
DROP TABLE IF EXISTS autos CASCADE;

-- pc5
CREATE TABLE autos (
    id          SERIAL PRIMARY KEY,
    placa       VARCHAR(12) NOT NULL,
    marca       VARCHAR(50) NOT NULL,
    modelo      VARCHAR(50) NOT NULL,
    anio        INTEGER NOT NULL,
    color       VARCHAR(30) NOT NULL,
    ubicacion   POINT NOT NULL,
    registrado  TIMESTAMP NOT NULL DEFAULT NOW()
);

-- btree
CREATE INDEX idx_autos_anio_btree
ON autos USING btree (anio);

-- hash
CREATE INDEX idx_autos_placa_hash
ON autos USING hash (placa);

-- espacial
CREATE INDEX idx_autos_ubicacion_gist
ON autos USING gist (ubicacion);
