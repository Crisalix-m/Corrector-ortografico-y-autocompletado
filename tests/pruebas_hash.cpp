
#include <iostream>
#include <cassert>

#include "../include/DiccionarioHash.h"

int main() {

    DiccionarioHash diccionario;

    std::cout << "=== PRUEBAS TABLA HASH ===\n";

    // PRUEBA 1: Insertar palabras

    assert(diccionario.insertar("casa", 1));
    assert(diccionario.insertar("perro", 1));
    assert(diccionario.insertar("gato", 1));

    assert(diccionario.tamano() == 3);

    std::cout << "[OK] Insercion de palabras\n";


    // PRUEBA 2: Buscar palabras

    assert(diccionario.buscar("casa"));
    assert(diccionario.buscar("perro"));
    assert(!diccionario.buscar("avion"));

    std::cout << "[OK] Busqueda de palabras\n";


    // PRUEBA 3: Insertar duplicados

    assert(!diccionario.insertar("casa", 5));

    assert(diccionario.tamano() == 3);

    std::size_t frecuencia = 0;

    assert(diccionario.obtenerFrecuencia("casa", frecuencia));

    assert(frecuencia == 5);

    std::cout << "[OK] Control de duplicados\n";


    // PRUEBA 4: Eliminar palabras

    assert(diccionario.eliminar("perro"));

    assert(!diccionario.buscar("perro"));

    assert(diccionario.tamano() == 2);

    std::cout << "[OK] Eliminacion de palabras\n";


    // PRUEBA 5: Factor de carga

    double factor = diccionario.factorCarga();

    assert(factor > 0.0);

    std::cout << "[OK] Factor de carga: "
              << factor << "\n";


    // PRUEBA 6: Cargar archivo

    DiccionarioHash diccionarioArchivo;

    bool cargado = diccionarioArchivo.cargarDesdeArchivo(
        "data/diccionario.txt"
    );

    assert(cargado);

    std::cout << "[OK] Archivo cargado\n";

    std::cout << "Palabras unicas: "
              << diccionarioArchivo.tamano() << "\n";

    std::cout << "Factor de carga: "
              << diccionarioArchivo.factorCarga() << "\n";


    std::cout << "\n=== TODAS LAS PRUEBAS FINALIZADAS ===\n";

    return 0;
}