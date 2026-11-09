#ifndef TRIE_H
#define TRIE_H

#include <cstddef>
#include <string>
#include <vector>

#include "tipos.h"

class Trie {
public:
    Trie();

    void insertar(const std::u32string& palabra, unsigned frecuencia);
    bool contiene(const std::u32string& palabra) const;
    unsigned frecuencia(const std::u32string& palabra) const;
    bool eliminar(const std::u32string& palabra);

    std::vector<Sugerencia> autocompletar(const std::u32string& prefijo, std::size_t maximo) const;
    std::vector<Sugerencia> similares(const std::u32string& objetivo, int limite, std::size_t maximo) const;
    std::size_t contarConPrefijo(const std::u32string& prefijo) const;

    std::size_t palabras() const { return palabras_; }
    std::size_t nodos() const { return nodos_.size(); }
    std::size_t aristas() const { return nodos_.size() - 1; }
    std::size_t profundidadMaxima() const;
    double ramificacionPromedio() const;
    std::size_t nodosVisitados() const { return visitados_; }
    std::size_t memoriaAproximada() const;
    void compactar();

private:
    struct Nodo {
        int hijo;
        int hermano;
        char32_t simbolo;
        unsigned frecuencia;
    };

    std::vector<Nodo> nodos_;
    std::size_t palabras_;
    mutable std::size_t visitados_;

    int buscarHijo(int nodo, char32_t simbolo) const;
    int obtenerHijo(int nodo, char32_t simbolo);
    int descender(const std::u32string& prefijo) const;
    void recolectar(int nodo, std::u32string& actual, std::size_t maximo, std::vector<Sugerencia>& monton) const;
    void explorar(int nodo, std::u32string& actual, const std::u32string& objetivo,
                  const std::vector<int>& fila, int limite, std::vector<Sugerencia>& salida) const;
};

#endif
