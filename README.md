### PROYECTO N.º 6 – INTEGRANTE 1
Implementación de DiccionarioHash
1. Compilación y ejecución de las pruebas de tabla hash

Desde la carpeta raíz del proyecto, ejecutar:

Compilación:

g++ -std=c++17 -Wall -Wextra -g -Iinclude src/DiccionarioHash.cpp tests/pruebas_hash.cpp -o pruebas_hash.exe

Ejecución:

.\pruebas_hash.exe

La ejecución permite verificar las funcionalidades de la tabla hash, incluyendo inserción, búsqueda, eliminación, frecuencias, redimensionamiento, diccionario personal y persistencia.

También muestra el conteo de palabras almacenadas, el factor de carga y los tiempos de ejecución de determinadas pruebas.

2. Consulta del diccionario principal

Mostrar las primeras 10 palabras:

Get-Content data/Diccionario.txt -Encoding UTF8 | Select-Object -First 10

Contar las palabras únicas:

(Get-Content data/Diccionario.txt -Encoding UTF8 | Sort-Object -Unique).Count
3. Organización de archivos de prueba

El archivo auxiliar utilizado para verificar el guardado y recuperación de información se encuentra en:

tests/prueba_guardado.txt

Las pruebas deben ejecutarse desde la carpeta raíz del proyecto para que las rutas relativas funcionen correctamente.