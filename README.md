# Proyecto 6 — Corrector ortográfico y autocompletado

Implementación integrada en C++17: diccionario en tabla hash, trie multivía con compresión radix, normalización de español, sugerencias por Levenshtein y CLI usable. Todo se ejecuta desde la raíz del repositorio.

## Camino rápido

1. Compilar: `make` (o los comandos `g++` del makefile si `make` no está instalado).
2. Verificar: `make test` → cinco suites, todas en verde.
3. Usar: `.\corrector.exe verificar "El perrro corre en la casa"`.

Ejemplos:

```powershell
.\corrector.exe stats
.\corrector.exe verificar "El perrro corre en la casa"
.\corrector.exe sugerir "cabllo"
.\corrector.exe completar "habl"
.\corrector.exe agregar "frutilla"
```

Opciones: `--diccionario=ruta`, `--personal=ruta`, `--frecuencias=ruta`.

## Decisiones

| Tema | Decisión |
|---|---|
| Diccionario | Se carga desde `data/Diccionario.txt`, se normaliza (minúsculas, sin tildes, `ñ` conservada) y se descartan entradas con puntuación. Quedan 71.658 palabras (> 50.000). |
| Estructuras | La tabla hash resuelve `buscar`; el trie resuelve prefijos y sugerencias acotadas. Ambas se alimentan de la misma carga normalizada. |
| Frecuencias | El archivo original **no trae frecuencias**: toda palabra empieza en 1. Las palabras personales empiezan en 2 y `registrarUso` incrementa y persiste en `data/frecuencias.txt`. El orden de autocompletado usa esos valores reales, no datos inventados. |
| Codificación | El normalizador trabaja con puntos de código UTF-8, así que `ñ` cuenta como una letra en Levenshtein y en el trie. |
| Radix | `compactar()` fusiona caminos de un solo hijo. Con el diccionario completo: 133.748 nodos eliminados y 6.419.904 bytes ahorrados; `corrector stats` lo mide en cada ejecución. |

## Análisis de complejidad

Sea `L` la longitud de una palabra en caracteres (puntos de código), `N` el número de palabras y `K` el máximo de resultados.

| Operación | Tiempo | Memoria | Notas |
|---|---|---|---|
| Hash: inserción / búsqueda | O(L) promedio, O(N·L) peor caso | O(N·L) | FNV-1a con encadenamiento y rehash al superar factor de carga 0,75. |
| Trie: inserción / búsqueda | O(L) | O(N·L) sin comprimir | Cada carácter es un nodo; la versión radix reduce nodos, no la cota asintótica. |
| Autocompletar por prefijo | O(L + R·L) | O(K) para el heap | `R` = palabras bajo el prefijo; se mantiene un heap de las `K` más frecuentes. |
| Levenshtein | O(m·n) | O(m·n) con matriz, O(n) con dos filas | `distanciaAcotada` poda cuando el mínimo de la fila supera el límite. |
| Verificar un texto | O(T·L) | O(T) | `T` = tokens del texto; cada token se busca en O(L). |

Comparación práctica: la hash responde una búsqueda exacta sin recorrer el prefijo; el trie paga O(L) igual pero permite enumerar prefijos y podar búsquedas por distancia. La hash guarda la palabra completa por entrada; el trie comparte prefijos y, tras la compresión radix, elimina nodos intermedios a costa de etiquetas más largas.

## Verificación

| Suite | Resultado |
|---|---|
| `pruebas_hash.exe` | todas las aserciones OK |
| `pruebas_levenshtein.exe` | 15063/15063 |
| `pruebas_normalizador.exe` | 43/43 |
| `pruebas_trie.exe` | 52/52 (incluye inserción y búsqueda después de compactar) |
| `pruebas_integradas.exe` | 25/25 (carga, marcado, sugerencias, frecuencias, persistencia) |

## Checklist

- [x] Tabla hash con más de 50.000 palabras cargadas desde archivo.
- [x] Inserción y búsqueda en hash y trie.
- [x] Autocompletado por prefijo ordenado por frecuencia.
- [x] Hasta cinco correcciones por palabra mediante Levenshtein acotado.
- [x] Texto verificado con palabras desconocidas marcadas.
- [x] Diccionario personal persistente.
- [x] Análisis de complejidad y comparación hash/trie.
- [x] Radix tree con medición del ahorro de memoria.

## Archivos principales

- `src/Corrector.cpp` — integración: carga normalizada, verificación, sugerencias y persistencia.
- `src/main.cpp` — CLI.
- `src/Trie.cpp` — trie con etiquetas por arco y compresión radix.
- `src/DiccionarioHash.cpp` — tabla hash con rehash y frecuencias.
- `src/Levenshtein.cpp` / `src/Normalizador.cpp` — distancia y preparación del texto.
- `include/tipos.h` / `include/utf8.h` — tipos compartidos y conversión UTF-8.
