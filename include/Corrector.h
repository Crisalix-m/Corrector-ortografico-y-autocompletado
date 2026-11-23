#ifndef CORRECTOR_H
#define CORRECTOR_H

#include <cstddef>
#include <string>
#include <vector>

#include "DiccionarioHash.h"
#include "Normalizador.h"
#include "tipos.h"
#include "trie.h"

struct PalabraVerificada {
    Token token;
    bool conocida;
    std::vector<Sugerencia> sugerencias;
};

class Corrector {
public:
    struct Opciones {
        std::string diccionario = "data/Diccionario.txt";
        std::string personal = "data/diccionario_personal.txt";
        std::string frecuencias = "data/frecuencias.txt";
        std::size_t maxSugerencias = 5;
        int distanciaMaxima = 2;
        std::size_t maxAutocompletados = 10;
    };

    Corrector();
    explicit Corrector(Opciones opciones);

    struct Estadisticas {
        std::size_t palabras;
        std::size_t nodosTrie;
        std::size_t nodosComprimidos;
        std::size_t bytesAhorrados;
        std::size_t memoriaTrie;
    };

    bool cargar();
    std::size_t palabrasCargadas() const;
    Estadisticas estadisticas() const;

    std::vector<PalabraVerificada> verificar(const std::string& texto) const;
    std::string marcar(const std::string& texto) const;
    std::vector<Sugerencia> sugerir(const std::string& palabra) const;
    std::vector<Sugerencia> autocompletar(const std::string& prefijo) const;

    bool agregarPersonal(const std::string& palabra);
    bool registrarUso(const std::string& palabra);
    bool guardarFrecuencias() const;

private:
    Opciones opciones_;
    DiccionarioHash hash_;
    Trie trie_;
    std::size_t cargadas_;

    void insertarPalabra(const std::string& palabra, std::size_t frecuencia);
    bool insertarDesdeArchivo(const std::string& ruta,
                              std::size_t frecuencia);
    void aplicarFrecuencias();
};

#endif
