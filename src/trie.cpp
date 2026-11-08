#include "trie.h"
#include <algorithm>
#include <utility>
#include "utf8.h"

Trie::Trie() : palabras_(0), visitados_(0) {
    Nodo raiz;
    raiz.hijo = -1;
    raiz.hermano = -1;
    raiz.simbolo = 0;
    raiz.frecuencia = 0;
    nodos_.push_back(raiz);
}
int Trie::buscarHijo(int nodo, char32_t simbolo) const {
    int actual = nodos_[nodo].hijo;
    while (actual != -1 && nodos_[actual].simbolo < simbolo) actual = nodos_[actual].hermano;
    if (actual != -1 && nodos_[actual].simbolo == simbolo) return actual;
    return -1;
}
int Trie::obtenerHijo(int nodo, char32_t simbolo) {
    int previo = -1;
    int actual = nodos_[nodo].hijo;
    while (actual != -1 && nodos_[actual].simbolo < simbolo) {
        previo = actual;
        actual = nodos_[actual].hermano;
    }
    if (actual != -1 && nodos_[actual].simbolo == simbolo) return actual;

    Nodo nuevo;
    nuevo.hijo = -1;
    nuevo.hermano = actual;
    nuevo.simbolo = simbolo;
    nuevo.frecuencia = 0;
    nodos_.push_back(nuevo);
    int indice = static_cast<int>(nodos_.size()) - 1;
    if (previo == -1) nodos_[nodo].hijo = indice;
    else nodos_[previo].hermano = indice;
    return indice;
}

void Trie::insertar(const std::u32string& palabra, unsigned frecuencia) {
    if (palabra.empty()) return;
    if (frecuencia == 0) frecuencia = 1;
    int actual = 0;
    for (std::size_t i = 0; i < palabra.size(); ++i) actual = obtenerHijo(actual, palabra[i]);
    if (nodos_[actual].frecuencia == 0) ++palabras_;
    if (frecuencia > nodos_[actual].frecuencia) nodos_[actual].frecuencia = frecuencia;
}

int Trie::descender(const std::u32string& prefijo) const {
    int actual = 0;
    for (std::size_t i = 0; i < prefijo.size(); ++i) {
        ++visitados_;
        actual = buscarHijo(actual, prefijo[i]);
        if (actual == -1) return -1;
    }
    return actual;
}

bool Trie::contiene(const std::u32string& palabra) const {
    int nodo = descender(palabra);
    return nodo != -1 && nodos_[nodo].frecuencia > 0;
}

unsigned Trie::frecuencia(const std::u32string& palabra) const {
    int nodo = descender(palabra);
    return nodo == -1 ? 0u : nodos_[nodo].frecuencia;
}

bool Trie::eliminar(const std::u32string& palabra) {
    int nodo = descender(palabra);
    if (nodo == -1 || nodos_[nodo].frecuencia == 0) return false;
    nodos_[nodo].frecuencia = 0;
    --palabras_;
    return true;
}

void Trie::recolectar(int nodo, std::u32string& actual, std::size_t maximo,
                      std::vector<Sugerencia>& monton) const {
    ++visitados_;
    if (nodos_[nodo].frecuencia > 0) {
        Sugerencia candidata(texto::codificar(actual), nodos_[nodo].frecuencia, 0);
        if (monton.size() < maximo) {
            monton.push_back(candidata);
            std::push_heap(monton.begin(), monton.end(), masFrecuente);
        } else if (!monton.empty() && masFrecuente(candidata, monton.front())) {
            std::pop_heap(monton.begin(), monton.end(), masFrecuente);
            monton.back() = candidata;
            std::push_heap(monton.begin(), monton.end(), masFrecuente);
        }
    }
    int hijo = nodos_[nodo].hijo;
    while (hijo != -1) {
        actual.push_back(nodos_[hijo].simbolo);
        recolectar(hijo, actual, maximo, monton);
        actual.erase(actual.size() - 1);
        hijo = nodos_[hijo].hermano;
    }
}

std::vector<Sugerencia> Trie::autocompletar(const std::u32string& prefijo, std::size_t maximo) const {
    std::vector<Sugerencia> salida;
    if (maximo == 0) return salida;
    visitados_ = 0;
    int nodo = descender(prefijo);
    if (nodo == -1) return salida;
    std::u32string actual = prefijo;
    salida.reserve(maximo);
    recolectar(nodo, actual, maximo, salida);
    std::sort(salida.begin(), salida.end(), masFrecuente);
    return salida;
}

std::size_t Trie::contarConPrefijo(const std::u32string& prefijo) const {
    int nodo = descender(prefijo);
    if (nodo == -1) return 0;
    std::size_t total = 0;
    std::vector<int> pila;
    pila.push_back(nodo);
    while (!pila.empty()) {
        int actual = pila.back();
        pila.pop_back();
        if (nodos_[actual].frecuencia > 0) ++total;
        int hijo = nodos_[actual].hijo;
        while (hijo != -1) {
            pila.push_back(hijo);
            hijo = nodos_[hijo].hermano;
        }
    }
    return total;
}

void Trie::explorar(int nodo, std::u32string& actual, const std::u32string& objetivo,
                    const std::vector<int>& fila, int limite, std::vector<Sugerencia>& salida) const {
    const std::size_t columnas = objetivo.size();
    int hijo = nodos_[nodo].hijo;
    std::vector<int> nueva(columnas + 1);
    while (hijo != -1) {
        ++visitados_;
        const char32_t simbolo = nodos_[hijo].simbolo;
        nueva[0] = fila[0] + 1;
        int mejor = nueva[0];
        for (std::size_t j = 1; j <= columnas; ++j) {
            int costo = (objetivo[j - 1] == simbolo) ? 0 : 1;
            int valor = nueva[j - 1] + 1;
            if (fila[j] + 1 < valor) valor = fila[j] + 1;
            if (fila[j - 1] + costo < valor) valor = fila[j - 1] + costo;
            nueva[j] = valor;
            if (valor < mejor) mejor = valor;
        }
        if (mejor <= limite) {
            actual.push_back(simbolo);
            if (nodos_[hijo].frecuencia > 0 && nueva[columnas] <= limite)
                salida.push_back(Sugerencia(texto::codificar(actual), nodos_[hijo].frecuencia, nueva[columnas]));
            explorar(hijo, actual, objetivo, nueva, limite, salida);
            actual.erase(actual.size() - 1);
        }
        hijo = nodos_[hijo].hermano;
    }
}

std::vector<Sugerencia> Trie::similares(const std::u32string& objetivo, int limite, std::size_t maximo) const {
    std::vector<Sugerencia> salida;
    if (objetivo.empty() || limite < 0) return salida;
    visitados_ = 0;
    std::vector<int> fila(objetivo.size() + 1);
    for (std::size_t j = 0; j < fila.size(); ++j) fila[j] = static_cast<int>(j);
    std::u32string actual;
    actual.reserve(objetivo.size() + limite + 1);
    explorar(0, actual, objetivo, fila, limite, salida);
    std::sort(salida.begin(), salida.end(), mejorCorreccion);
    if (salida.size() > maximo) salida.resize(maximo);
    return salida;
}

std::size_t Trie::profundidadMaxima() const {
    std::size_t maxima = 0;
    std::vector<std::pair<int, std::size_t> > pila;
    pila.push_back(std::make_pair(0, static_cast<std::size_t>(0)));
    while (!pila.empty()) {
        int nodo = pila.back().first;
        std::size_t nivel = pila.back().second;
        pila.pop_back();
        if (nivel > maxima) maxima = nivel;
        int hijo = nodos_[nodo].hijo;
        while (hijo != -1) {
            pila.push_back(std::make_pair(hijo, nivel + 1));
            hijo = nodos_[hijo].hermano;
        }
    }
    return maxima;
}

double Trie::ramificacionPromedio() const {
    std::size_t internos = 0;
    for (std::size_t i = 0; i < nodos_.size(); ++i)
        if (nodos_[i].hijo != -1) ++internos;
    if (internos == 0) return 0.0;
    return static_cast<double>(nodos_.size() - 1) / static_cast<double>(internos);
}

void Trie::compactar() {
    nodos_.shrink_to_fit();
}

std::size_t Trie::memoriaAproximada() const {
    return sizeof(Trie) + nodos_.capacity() * sizeof(Nodo);
}
