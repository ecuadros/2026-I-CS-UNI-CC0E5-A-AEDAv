
drop table if exists ventas_unix;

create table ventas_unix (
    id bigserial primary key,
    auto varchar(80) not null,
    fecha_unix bigint not null,
    monto numeric(10,2) not null
);

-- carga de ejemplo muchas filas con fecha guardada como segundos unix
insert into ventas_unix (auto, fecha_unix, monto)
select
    'auto-' || (g % 50),
    extract(epoch from timestamp '2026-01-01' + (g || ' minutes')::interval)::bigint,
    (10000 + (g % 9000))::numeric
from generate_series(1, 200000) as g;

create index idx_ventas_unix_fecha on ventas_unix (fecha_unix);

analyze ventas_unix;


explain analyze
select count(*)
from ventas_unix
where to_timestamp(fecha_unix)::date = date '2026-03-15';


select count(*)
from ventas_unix
where fecha_unix >= extract(epoch from timestamp '2026-03-15 00:00:00')::bigint
  and fecha_unix <  extract(epoch from timestamp '2026-03-16 00:00:00')::bigint;

