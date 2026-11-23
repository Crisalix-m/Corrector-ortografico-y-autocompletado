#ifndef TRIE_H
#define TRIE_H

#include <cstddef>
#include <string>
#include <vector>

#include "tipos.h"

// Trie multivia sobre puntos de codigo UTF-8. Cada nodo guarda la etiqueta
// completa del arco que lo une con su padre; eso permite comprimir caminos
// de un solo hijo (radix tree) sin cambiar la interfaz.
class Trie {
public:
    Trie();

    void insertar(const std::u32string& palabra, unsigned frecuencia);
    bool contiene(const std::u32string& palabra) const;
    unsigned frecuencia(const std::u32string& palabra) const;
    bool eliminar(const std::u32string& palabra);
    void fijarFrecuencia(const std::u32string& palabra, unsigned frecuencia);

    std::vector<Sugerencia> autocompletar(const std::u32string& prefijo,
                                          std::size_t maximo) const;
    std::vector<Sugerencia> similares(const std::u32string& objetivo,
                                      int limite, std::size_t maximo) const;
    std::size_t contarConPrefijo(const std::u32string& prefijo) const;

    std::size_t palabras() const { return palabras_; }
    std::size_t nodos() const { return nodos_.size(); }
    std::size_t nodosActivos() const;
    std::size_t aristas() const;
    std::size_t profundidadMaxima() const;
    double ramificacionPromedio() const;
    std::size_t nodosVisitados() const { return visitados_; }
    std::size_t memoriaAproximada() const;
    std::size_t nodosAhorrados() const { return nodosAhorrados_; }
    std::size_t bytesAhorrados() const { return bytesAhorrados_; }

    void compactar();

private:
    struct Nodo {
        std::u32string etiqueta;
        unsigned frecuencia;
        int hijo;
        int hermano;
    };

    std::vector<Nodo> nodos_;
    std::size_t palabras_;
    mutable std::size_t visitados_;
    std::size_t nodosAhorrados_;
    std::size_t bytesAhorrados_;

    int nuevoNodo(std::u32string etiqueta, unsigned frecuencia,
                  int hijo, int hermano);
    int buscarHijo(int nodo, char32_t simbolo) const;
    int obtenerHijo(int nodo, char32_t simbolo);
    int descender(const std::u32string& texto, std::size_t& consumidos,
                  bool estricto = false) const;
    void prefijoHasta(const std::u32string& prefijo, std::u32string& salida) const;
    void recolectar(int nodo, std::u32string& actual, std::size_t maximo,
                    std::vector<Sugerencia>& monton) const;
    void explorar(int nodo, std::u32string& actual,
                  const std::u32string& objetivo, const std::vector<int>& fila,
                  int limite, std::vector<Sugerencia>& salida) const;
};

#endif
