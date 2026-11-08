#include "DiccionarioHash.h"
#include <utility>
#include <fstream>
#include <string>

DiccionarioHash::Nodo::Nodo(    //constructor de la estructura Nodo, que está dentro de DiccionarioHash
    const std::string& p,       // p, lo que almacenamos en palabra
    std::size_t f,              // f, lo que almacenamos en frecuencia
    Nodo* s                     // s, es el puntero al siguiente nodo, que por defecto es nullptr
) {
    palabra = p;
    frecuencia = f;
    siguiente = s;
}

//constructor de la clase DiccionarioHash
//Inicializa la tabla hash con una capacidad inicial y establece la cantidad de elementos y el factor máximo de carga 
DiccionarioHash::DiccionarioHash(std::size_t capacidadInicial) {   
    tabla.resize(capacidadInicial, nullptr);

    cantidad = 0;               // Sin palabras insertadas aún

    factorMaximo = 0.75;        // Factor de carga máximo recomendado(SE PUEDE CAMBIAR DESPUÉS)
}

// Destructor para eliminar los nuevos nodos creados en la tabla hash y liberar la memoria
DiccionarioHash::~DiccionarioHash() {

    for (std::size_t i = 0; i < tabla.size(); i++) {

        Nodo* actual = tabla[i];

        while (actual != nullptr) {

            Nodo* temporal = actual;

            actual = actual->siguiente;

            delete temporal;
        }
    }
}

// ===================================================
//  Función Hash: Calcula el índice de la tabla hash para una palabra dada utilizando el algoritmo de hash djb2.
//  Devuelve el índice calculado como un valor de tipo std::size_t.
// ===================================================

std::size_t DiccionarioHash::funcionHash(
    const std::string& palabra
) const {

    std::uint64_t hash = 14695981039346656037ULL;

    for (unsigned char caracter : palabra) {

        hash ^= caracter;

        hash *= 1099511628211ULL;
    }

    return hash % tabla.size();
}

std::size_t DiccionarioHash::funcionHash(
    const std::string& palabra,
    std::size_t capacidad
) const {

    std::uint64_t hash = 14695981039346656037ULL;

    for (unsigned char caracter : palabra) {

        hash ^= caracter;

        hash *= 1099511628211ULL;
    }

    return hash % capacidad;
}

// ===================================================
//  Rehash: Redimensiona la tabla hash a una nueva capacidad y reubica los elementos existentes.
//  Se utiliza cuando el factor de carga supera el factor máximo permitido.
// ===================================================

void DiccionarioHash::rehash(std::size_t nuevaCapacidad) {

    if (nuevaCapacidad == 0) {
        return;
    }

    std::vector<Nodo*> nuevaTabla(nuevaCapacidad, nullptr);

    for (std::size_t i = 0; i < tabla.size(); i++) {

        Nodo* actual = tabla[i];

        while (actual != nullptr) {

            Nodo* siguiente = actual->siguiente;

            std::size_t nuevaPosicion =
                funcionHash(actual->palabra, nuevaCapacidad);

            actual->siguiente = nuevaTabla[nuevaPosicion];

            nuevaTabla[nuevaPosicion] = actual;

            actual = siguiente;
        }
    }

    tabla = std::move(nuevaTabla);
}
// ===================================================
//  Insertar: Inserta una palabra en la tabla hash con su frecuencia.
//  Si la palabra ya existe, incrementa su frecuencia.
//  Si el factor de carga supera el máximo permitido, realiza un rehash.
//  Devuelve true si la inserción fue exitosa, false en caso contrario.
// ===================================================

bool DiccionarioHash::insertar(
    const std::string& palabra,
    std::size_t frecuencia
) {

    if (palabra.empty()) {
        return false;
    }

    std::size_t posicion = funcionHash(palabra);

    Nodo* actual = tabla[posicion];

    while (actual != nullptr) {

        if (actual->palabra == palabra) {

            actual->frecuencia += frecuencia;

            return false;
        }

        actual = actual->siguiente;
    }

    Nodo* nuevo = new Nodo(palabra, frecuencia);

    nuevo->siguiente = tabla[posicion];

    tabla[posicion] = nuevo;

    cantidad++;

    if (factorCarga() > factorMaximo) {

        rehash(tabla.size() * 2 + 1);

    }

    return true;
}

// ===================================================
//  Buscar: Busca una palabra en la tabla hash.
//  Devuelve true si la palabra se encuentra, false en caso contrario.
// ===================================================


bool DiccionarioHash::buscar(
    const std::string& palabra
) const {

    if (palabra.empty()) {
        return false;
    }

    std::size_t posicion = funcionHash(palabra);

    Nodo* actual = tabla[posicion];

    while (actual != nullptr) {

        if (actual->palabra == palabra) {
            return true;
        }

        actual = actual->siguiente;
    }

    return false;
}

//===================================================
//  Eliminar: Elimina una palabra de la tabla hash.
//  Devuelve true si la palabra fue eliminada, false si no se encontró.
//===================================================


bool DiccionarioHash::eliminar(
    const std::string& palabra
) {

    if (palabra.empty()) {
        return false;
    }

    std::size_t posicion = funcionHash(palabra);

    Nodo* actual = tabla[posicion];

    Nodo* anterior = nullptr;

    while (actual != nullptr) {

        if (actual->palabra == palabra) {

            if (anterior == nullptr) {

                tabla[posicion] = actual->siguiente;

            } else {

                anterior->siguiente = actual->siguiente;

            }

            delete actual;

            cantidad--;

            return true;
        }

        anterior = actual;

        actual = actual->siguiente;
    }

    return false;
}

// ===================================================
//  Obtener Frecuencia: Obtiene la frecuencia de una palabra en la tabla hash.
//  Devuelve true si la palabra se encuentra y actualiza la frecuencia, false en caso contrario.
// ===================================================


bool DiccionarioHash::obtenerFrecuencia(
    const std::string& palabra,
    std::size_t& frecuencia
) const {

    if (palabra.empty()) {
        return false;
    }

    std::size_t posicion = funcionHash(palabra);

    Nodo* actual = tabla[posicion];

    while (actual != nullptr) {

        if (actual->palabra == palabra) {

            frecuencia = actual->frecuencia;

            return true;
        }

        actual = actual->siguiente;
    }

    return false;
}

// ===================================================
//  Cargar Desde Archivo: Carga palabras y frecuencias desde un archivo de texto.
//  Cada línea del archivo debe contener una palabra y su frecuencia separadas por un espacio.
//  Devuelve true si la carga fue exitosa, false en caso contrario.
// ===================================================



bool DiccionarioHash::cargarDesdeArchivo(
    const std::string& ruta
) {
    std::ifstream archivo(ruta);

    if (!archivo.is_open()) {
        return false;
    }

    std::string palabra;

    while (std::getline(archivo, palabra)) {

        // Eliminar retorno de carro de Windows
        if (!palabra.empty() && palabra.back() == '\r') {
            palabra.pop_back();
        }

        // Ignorar líneas vacías
        if (palabra.empty()) {
            continue;
        }

        // Insertar la entrada en la tabla hash
        insertar(palabra, 1);
    }

    archivo.close();

    return true;
}

// ===================================================
//  Tamaño: Devuelve la cantidad de palabras almacenadas en la tabla hash.
// ===================================================

std::size_t DiccionarioHash::tamano() const {

    return cantidad;

}

// ===================================================
//  Factor de Carga: Calcula y devuelve el factor de carga actual de la tabla hash.
//  El factor de carga se calcula como la cantidad de elementos dividida por la capacidad de la tabla.
// ===================================================

double DiccionarioHash::factorCarga() const {

    if (tabla.empty()) {
        return 0.0;
    }

    return static_cast<double>(cantidad) / tabla.size();

}
