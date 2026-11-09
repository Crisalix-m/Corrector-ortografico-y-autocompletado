
#pragma once        // Asegura que se procese solo una vez durante la compilación 

#include <string>
#include <vector>
#include <cstddef>  // Para std::size_t 

class DiccionarioHash {

private:

    struct Nodo {

        std::string palabra;

        std::size_t frecuencia;

        Nodo* siguiente;

        Nodo(
            const std::string& p,
            std::size_t f,
            Nodo* s = nullptr
        );

    };

    std::vector<Nodo*> tabla;

    std::size_t cantidad;

    double factorMaximo;

    std::size_t funcionHash(
        const std::string& palabra

    ) const;
    
    std::size_t funcionHash(            //otroarrojado
        const std::string& palabra,
        std::size_t capacidad
    ) const;

    void rehash(std::size_t nuevaCapacidad);

public:

    DiccionarioHash(std::size_t capacidadInicial = 101);

    ~DiccionarioHash();

    DiccionarioHash(const DiccionarioHash&) = delete;
    DiccionarioHash& operator=(const DiccionarioHash&) = delete;

    bool insertar(
        const std::string& palabra,
        std::size_t frecuencia = 1
    );

    bool buscar(
        const std::string& palabra
    ) const;

    bool eliminar(
        const std::string& palabra
    );

    bool obtenerFrecuencia(
        const std::string& palabra,
        std::size_t& frecuencia
    ) const;

    bool cargarDesdeArchivo(
        const std::string& ruta
    );

    bool guardarEnArchivo(
        const std::string& ruta
    ) const;

    bool agregarPalabraPersonal(
        const std::string& palabra
    );

    bool agregarYGuardarPalabraPersonal(
        const std::string& palabra,
        const std::string& ruta
    );

    std::size_t tamano() const;

    double factorCarga() const;

};
