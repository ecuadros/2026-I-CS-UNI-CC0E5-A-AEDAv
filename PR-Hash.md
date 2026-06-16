## Descripción

Implementación de `HashTable<Trait>` como **tabla hash real**: un arreglo de cubetas indexado por `hash(clave) % numCubetas`, donde las colisiones se resuelven por **encadenamiento**, pero en vez de una lista enlazada cada cubeta es un `AVL<Trait>` (búsqueda intra-cubeta en `O(log k)`). Se mantiene el patrón **Templates + Traits + CRTP** del repositorio: el hasher se empaqueta en el Trait, el iterador deriva de `general_iterator` y la concurrencia usa `shared_mutex`.

---

## Arquitectura de clases

```mermaid
classDiagram
    class BaseTrait {
        +value_type
        +Comp
        +Node
    }
    class BaseHashTrait {
        +Hasher
    }
    class DefaultHash {
        +operator()(key) size_t
    }
    class AVL {
        +internal_insert() override
        +balanceFactor()
    }
    class BinaryTree {
        #m_pRoot
        #m_comp
        #find_node()
        +operator[]()
        +contains()
        +insert() virtual
    }
    class HashTable {
        -m_buckets AVL[]
        -m_numBuckets
        -m_hash Hasher
        -m_mtx
        +operator[]()
        +insert()
        +contains()
        +begin() end()
        +operator<<()
        +operator>>()
    }
    class general_iterator {
        #m_pNode
        +operator*()
        +getRef()
    }
    class hash_forward_iterator {
        -m_nodes Node*[]
        -m_idx
        +operator++()
        +operator*() tuple
    }

    BaseTrait <|-- BaseHashTrait
    BaseHashTrait <|-- AscendingHashTrait
    BaseHashTrait <|-- DescendingHashTrait
    DefaultHash <.. BaseHashTrait : Hasher
    BinaryTree <|-- AVL : herencia
    HashTable o-- AVL : m_buckets[] (cubetas)
    HashTable ..> AscendingHashTrait : Trait
    general_iterator <|-- hash_forward_iterator : CRTP
    HashTable *-- hash_forward_iterator : iterator
```

---

## Reparto por la función hash y encadenamiento

```mermaid
flowchart LR
    K["claves: 5,2,8,1,9,13"] --> H["hash(k) % 4"]
    H --> B0["cubeta[0]<br/>AVL: 8"]
    H --> B1["cubeta[1]<br/>AVL: 1,5,9,13"]
    H --> B2["cubeta[2]<br/>AVL: 2"]
    H --> B3["cubeta[3]<br/>vacía"]
    B1 --> C["colisión 5,1,9,13<br/>resuelta por el AVL<br/>O(log k)"]
```

---

## Flujo de `operator[]` (find-or-insert)

```mermaid
flowchart TD
    A["tabla[key]"] --> B["shared_lock(m_mtx)"]
    B --> C["idx = hash(key) % numBuckets"]
    C --> D["m_buckets[idx][key]"]
    D --> E{"AVL::operator[]<br/>find_node(key)?"}
    E -->|existe| F["devuelve node->m_ref&"]
    E -->|no existe| G["internal_insert(key, Ref())<br/>→ AVL rebalancea"]
    G --> F
    F --> H["= value (crea o actualiza)"]
```

> El `shared_lock` protege solo el arreglo de cubetas (que no cambia); el `unique_lock` fino lo toma el AVL de la cubeta, de modo que inserciones en cubetas distintas pueden ocurrir en paralelo.

---

## Flujo de `operator>>` (reutiliza el AVL)

```mermaid
sequenceDiagram
    participant U as Usuario
    participant HT as HashTable::operator>>
    participant T as Bucket tmp (AVL)
    participant I as HashTable::insert

    U->>HT: is >> tabla
    HT->>T: is >> tmp  (reutiliza AVL::operator>>)
    T-->>HT: AVL temporal con [(k,v),...]
    loop por cada nodo de tmp (inorder)
        HT->>I: insert(clave, valor)
        I->>I: m_buckets[hash(k)][k] = v
    end
    HT-->>U: tabla redistribuida por hash
```

---

## Archivos modificados / creados

| Archivo | Acción | Descripción |
|---------|--------|-------------|
| `containers/hashtable.h` | Reescritura | Tabla hash real: `DefaultHash`, `BaseHashTrait` (empaqueta el Hasher), arreglo de cubetas `AVL<Trait>`, `hash_forward_iterator` (CRTP), Big-Five y persistencia |
| `containers/BinaryTree.h` | Modificado | Se agregan `operator[]` (find-or-insert) y `contains()`, de los que depende cada cubeta para resolver colisiones |
| `containers/HashDemo.cpp` | Modificado | Demo: reparto por hash, contenido por cubeta, recorrido `[clave, valor]`, persistencia y copy/move |
| `Makefile` | Modificado | Agrega `HashDemo.cpp` a `SRCS` y genera dependencias de headers (`-MMD -MP`) |

---

## Checklist de tareas — HashTable

| Tarea | Archivo | Línea |
|-------|---------|-------|
| Constructor copia | `containers/hashtable.h` | 103 |
| Move constructor | `containers/hashtable.h` | 112 |
| `m[5] = 3;` (`operator[]`) | `containers/hashtable.h` | 145 |
| `for (const auto& [key, value] : m)` | `containers/hashtable.h` | 34 |
| `operator<<` (persistencia) | `containers/hashtable.h` | 183 |
| `operator>>` | `containers/hashtable.h` | 198 |
| `operator[]` / `contains` de soporte | `containers/BinaryTree.h` | 347 |

---

## Decisiones de diseño

- **Tabla hash real, no simulada**: en vez de un solo árbol, hay `m_numBuckets` cubetas. El índice es `hash(clave) % numCubetas` y cada cubeta es un `AVL<Trait>` que ordena y balancea las claves que colisionan (búsqueda `O(log k)` en vez de `O(k)` de una lista).
- **El Hasher va en el Trait (patrón Trait/Policy)**: `BaseHashTrait` extiende `BaseTrait` agregando `Hasher`, así `HashTable<Trait>` no necesita un parámetro de plantilla crudo extra. Cambiar la función hash es definir otro Trait.
- **Iterador CRTP que hereda `general_iterator`**: `hash_forward_iterator` toma una instantánea de los nodos de todas las cubetas y la recorre por índice. Su `operator*` devuelve `tuple<value_type, Ref&>`, lo que habilita `for (const auto& [clave, valor] : tabla)`.
- **Lectura reutilizando el AVL**: el formato plano `[(k,v),...]` es el mismo que parsea el `operator>>` heredado de `BinaryTree`; `operator>>` lee en un AVL temporal y redistribuye los nodos por hash. La escritura recorre las cubetas (ningún sub-objeto contiene la tabla completa).
- **Concurrencia de grano fino**: `shared_lock` sobre el arreglo de cubetas + `unique_lock` por cubeta (en el AVL) → inserciones en cubetas distintas en paralelo. `begin()`/`end()` quedan sin lock (como `BinaryTree`); la serialización toma el lock.

---

## Cómo ejecutar

```bash
make clean && make
./main
cat temp.txt   # tabla persistida en disco
```

---

## Salida de la demo

```
--- HashTable (funcion hash + colisiones resueltas con AVL) ---
tabla[5]   = 55
size       = 6, cubetas = 4, carga = 1.5
reparto por la funcion hash:
  hash(5) -> cubeta 1
  hash(2) -> cubeta 2
  hash(8) -> cubeta 0
  hash(1) -> cubeta 1
  hash(9) -> cubeta 1
  hash(13) -> cubeta 1
contenido de cada cubeta (cada una es un AVL):
  cubeta[0] = [(8,80)]
  cubeta[1] = [(13,130),(9,90),(5,55),(1,10)]
  cubeta[2] = [(2,20)]
  cubeta[3] = []
recorrido  : 8=80 13=130 9=90 5=55 1=10 2=20
contains(9)  = 1, contains(7) = 0
serializa  : [(8,80),(13,130),(9,90),(5,55),(1,10),(2,20)]
leida      : [(8,80),(13,130),(9,90),(5,55),(1,10),(2,20)]
original tabla[5] = 55, copia[5] = 999
movida     : [(8,80),(13,130),(9,90),(5,55),(1,10),(2,20)]
```

> En la cubeta 1 colisionan `5,1,9,13`: el AVL las mantiene balanceadas. La copia es independiente del original (deep copy de cada cubeta): al modificar `copia[5]=999` el original sigue en `55`.
