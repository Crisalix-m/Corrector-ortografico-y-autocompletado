#ifndef TIPOS_H
#define TIPOS_H

#include <cstddef>
#include <string>
#include <utility>

struct Sugerencia {
    std::string palabra;
    int distancia;
    std::size_t frecuencia;

    Sugerencia() : palabra(), distancia(0), frecuencia(0) {}

    Sugerencia(std::string p, int d = 0, std::size_t f = 0)
        : palabra(std::move(p)), distancia(d), frecuencia(f) {}

    Sugerencia(std::string p, std::size_t f, int d)
        : palabra(std::move(p)), distancia(d), frecuencia(f) {}
};

inline bool masFrecuente(const Sugerencia& a, const Sugerencia& b) {
    if (a.frecuencia != b.frecuencia) return a.frecuencia > b.frecuencia;
    return a.palabra < b.palabra;
}

inline bool mejorCorreccion(const Sugerencia& a, const Sugerencia& b) {
    if (a.distancia != b.distancia) return a.distancia < b.distancia;
    if (a.frecuencia != b.frecuencia) return a.frecuencia > b.frecuencia;
    return a.palabra < b.palabra;
}

#endif
