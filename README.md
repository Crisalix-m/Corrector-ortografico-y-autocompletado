PROYECTO N°6

// Para ejecutar el hash: 
[Te arroja el resultado de si esstá OK todas las funciones además de el conteo de palaqbras y factor de carga] //
1. g++ -std=c++17 -Wall -Wextra -g -Iinclude src/DiccionarioHash.cpp tests/pruebas_hash.cpp -o pruebas_hash

2. .\pruebas_hash.exe

-------
// CONTEO DE LAS PRIMERAS 10 PALABRAS Y CONTEO DE LAS PALABRAS UNICAS QUE EXISTEN //

1. Get-Content data/diccionario.txt | Select-Object -First 10    
(con UTF-8 QUE ADMITE TILDES)
Get-Content data/diccionario.txt -Encoding UTF8 | Select-Object -First 10       

2. (Get-Content data/diccionario.txt | Sort-Object -Unique).Count  
