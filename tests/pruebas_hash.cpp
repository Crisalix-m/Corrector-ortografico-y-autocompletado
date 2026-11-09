
#include <iostream> // Para la salida de información
#include <cassert>  // Para las aserciones en las pruebas
#include <chrono>   // Para medir el tiempo de ejecución
#include "../include/DiccionarioHash.h" // Incluir la clase DiccionarioHash

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

    assert(frecuencia == 6);

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

    auto inicio = std::chrono::high_resolution_clock::now();
    bool cargado = diccionarioArchivo.cargarDesdeArchivo(
        "data/diccionario.txt"
    );
    auto fin = std::chrono::high_resolution_clock::now();
    auto duracion =
        std::chrono::duration_cast<std::chrono::milliseconds>(
        fin - inicio
    ).count();

    assert(cargado);

    std::cout << "[OK] Archivo cargado\n";

    std::cout << "Tiempo de carga: "
              << duracion << " ms\n";
    
    std::cout << "Palabras almacenadas: "
              << diccionarioArchivo.tamano() << "\n";

    std::cout << "Factor de carga: "
              << diccionarioArchivo.factorCarga() << "\n";


    // PRUEBA 7: Verificar palabras después de rehash

    assert(diccionarioArchivo.buscar("casa"));
    assert(diccionarioArchivo.buscar("perro"));
    assert(diccionarioArchivo.buscar("arbol") == false);
    assert(diccionarioArchivo.buscar("árbol"));

    assert(diccionarioArchivo.tamano() == 93937);

    std::cout << "[OK] Palabras conservadas despues de rehash\n";


    // PRUEBA 8: Verificar frecuencia de una entrada

    std::size_t frecuenciaArchivo = 0;

    assert(diccionarioArchivo.obtenerFrecuencia(
        "casa", frecuenciaArchivo
    ));

    assert(frecuenciaArchivo >= 1);

    std::cout << "[OK] Frecuencias conservadas\n";

    
    // PRUEBA 9: Caracteres especiales

    assert(diccionarioArchivo.buscar("árbol"));
    assert(diccionarioArchivo.buscar("zurubí"));

    std::cout << "[OK] Busqueda de palabras con tildes\n";


    // PRUEBA 10: Entradas con espacios y comas

    assert(diccionarioArchivo.buscar("zurumbático, ca"));

    std::cout << "[OK] Busqueda de entradas con espacios\n";


    // PRUEBA 11: Comparacion exacta de palabras

    assert(!diccionarioArchivo.buscar("arbol"));

    std::cout << "[OK] Comparacion exacta de cadenas\n";

    
    // PRUEBA 12: Rendimiento de busqueda

    const int REPETICIONES = 10000;

    auto inicioBusqueda = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < REPETICIONES; i++) {
        diccionarioArchivo.buscar("casa");
    }

    auto finBusqueda = std::chrono::high_resolution_clock::now();

    auto tiempoBusqueda =
        std::chrono::duration_cast<std::chrono::microseconds>(
            finBusqueda - inicioBusqueda
        ).count();

    std::cout << "[OK] Rendimiento de busqueda exitosa\n";

    std::cout << "Tiempo de "
              << REPETICIONES
              << " busquedas exitosas: "
              << tiempoBusqueda << " microsegundos\n";


    // PRUEBA 13: Busquedas fallidas

    auto inicioFallidas = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < REPETICIONES; i++) {
        diccionarioArchivo.buscar("palabra_inexistente_xyz");
    }

    auto finFallidas = std::chrono::high_resolution_clock::now();

    auto tiempoFallidas =
        std::chrono::duration_cast<std::chrono::microseconds>(
            finFallidas - inicioFallidas
        ).count();

    std::cout << "[OK] Rendimiento de busqueda fallida\n";

    std::cout << "Tiempo de "
              << REPETICIONES
              << " busquedas fallidas: "
              << tiempoFallidas << " microsegundos\n";

    
    
    // PRUEBA 14: Verificar acumulacion de frecuencias

    DiccionarioHash diccionarioFrecuencias;

    diccionarioFrecuencias.insertar("manzana", 1);
    diccionarioFrecuencias.insertar("manzana", 2);
    diccionarioFrecuencias.insertar("manzana", 3);

    std::size_t frecuenciaManzana = 0;

    assert(diccionarioFrecuencias.obtenerFrecuencia(
        "manzana", frecuenciaManzana
    ));

    assert(frecuenciaManzana == 6);

    std::cout << "[OK] Acumulacion de frecuencias\n";

    
    // PRUEBA 15: Conservacion de frecuencia despues de rehash

    for (int i = 0; i < 200; i++) {
        diccionarioFrecuencias.insertar(
            "palabra" + std::to_string(i), 1
        );
    }

    frecuenciaManzana = 0;

    assert(diccionarioFrecuencias.obtenerFrecuencia(
        "manzana", frecuenciaManzana
    ));

    assert(frecuenciaManzana == 6);

    std::cout << "[OK] Frecuencia conservada despues de rehash\n";

    // PRUEBA 16: Guardar diccionario personal

    DiccionarioHash diccionarioPersonal;

    diccionarioPersonal.insertar("ChatGPT");
    diccionarioPersonal.insertar("OpenAI");
    diccionarioPersonal.insertar("programación");
    diccionarioPersonal.insertar("ciberseguridad");

    assert(diccionarioPersonal.guardarEnArchivo(
        "data/diccionario_personal.txt"
    ));

    std::cout << "[OK] Guardado del diccionario personal\n";

    
    // PRUEBA 17: Recuperar diccionario personal

    DiccionarioHash diccionarioRecuperado;

    bool recuperado = diccionarioRecuperado.cargarDesdeArchivo(
        "data/diccionario_personal.txt"
    );

    assert(recuperado);

    assert(diccionarioRecuperado.buscar("ChatGPT"));
    assert(diccionarioRecuperado.buscar("OpenAI"));
    assert(diccionarioRecuperado.buscar("programación"));
    assert(diccionarioRecuperado.buscar("ciberseguridad"));

    assert(diccionarioRecuperado.tamano() == 4);

    std::cout << "[OK] Recuperacion del diccionario personal\n";

    
    // PRUEBA 18: Agregar palabras personales sin duplicados

    DiccionarioHash diccionarioNuevo;

    assert(diccionarioNuevo.agregarPalabraPersonal("Python"));

    assert(diccionarioNuevo.agregarPalabraPersonal("Java"));

    assert(!diccionarioNuevo.agregarPalabraPersonal("Python"));

    assert(diccionarioNuevo.tamano() == 2);

    assert(!diccionarioNuevo.agregarPalabraPersonal(""));

    std::cout << "[OK] Control de duplicados personales\n";

    
    // PRUEBA 19: Agregar y guardar automaticamente

    DiccionarioHash diccionarioAutomatico;

    assert(diccionarioAutomatico.agregarYGuardarPalabraPersonal(
        "algoritmo",
        "data/diccionario_personal.txt"
    ));

    assert(diccionarioAutomatico.buscar("algoritmo"));

    DiccionarioHash diccionarioComprobacion;

    assert(diccionarioComprobacion.cargarDesdeArchivo(
        "data/diccionario_personal.txt"
    ));

    assert(diccionarioComprobacion.buscar("algoritmo"));

    std::cout << "[OK] Guardado automatico de palabras personales\n";

    
    // PRUEBA 20: Conservar palabras anteriores

    DiccionarioHash diccionarioPersistente;

    assert(diccionarioPersistente.cargarDesdeArchivo(
        "data/diccionario_personal.txt"
    ));

    assert(diccionarioPersistente.buscar("algoritmo"));

    assert(diccionarioPersistente.agregarPalabraPersonal(
        "estructuras"
    ));

    assert(diccionarioPersistente.guardarEnArchivo(
        "data/diccionario_personal.txt"
    ));

    DiccionarioHash diccionarioFinal;

    assert(diccionarioFinal.cargarDesdeArchivo(
        "data/diccionario_personal.txt"
    ));

    assert(diccionarioFinal.buscar("algoritmo"));
    assert(diccionarioFinal.buscar("estructuras"));

    std::cout << "[OK] Conservacion de palabras personales\n";

    
    // PRUEBA 21: Cargar dos veces sin duplicar frecuencias

    DiccionarioHash diccionarioDobleCarga;

    assert(diccionarioDobleCarga.cargarDesdeArchivo(
        "data/diccionario_personal.txt"
    ));

    std::size_t frecuenciaInicial = 0;

    assert(diccionarioDobleCarga.obtenerFrecuencia(
        "algoritmo", frecuenciaInicial
    ));

    assert(diccionarioDobleCarga.cargarDesdeArchivo(
        "data/diccionario_personal.txt"
    ));

    std::size_t frecuenciaFinal = 0;

    assert(diccionarioDobleCarga.obtenerFrecuencia(
        "algoritmo", frecuenciaFinal
    ));

    assert(frecuenciaInicial == frecuenciaFinal);

    std::cout << "[OK] Carga repetida sin alterar frecuencias\n";

    
    // PRUEBA 22: Guardar palabras con caracteres especiales

    DiccionarioHash diccionarioEspecial;

    assert(diccionarioEspecial.insertar("árbol"));
    assert(diccionarioEspecial.insertar("niño"));
    assert(diccionarioEspecial.insertar("programación"));

    assert(diccionarioEspecial.guardarEnArchivo(
        "tests/prueba_guardado.txt"
    ));

    DiccionarioHash diccionarioEspecialRecuperado;

    assert(diccionarioEspecialRecuperado.cargarDesdeArchivo(
        "tests/prueba_guardado.txt"
    ));

    assert(diccionarioEspecialRecuperado.buscar("árbol"));
    assert(diccionarioEspecialRecuperado.buscar("niño"));
    assert(diccionarioEspecialRecuperado.buscar("programación"));

    std::cout << "[OK] Guardado y recuperacion de caracteres especiales\n";


    std::cout << "\n=== TODAS LAS PRUEBAS FINALIZADAS ===\n";

    return 0;
}