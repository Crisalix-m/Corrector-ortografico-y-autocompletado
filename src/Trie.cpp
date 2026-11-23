#include "trie.h"

#include <algorithm>
#include <utility>

#include "utf8.h"

Trie::Trie()
    : palabras_(0), visitados_(0), nodosAhorrados_(0), bytesAhorrados_(0) {
    nodos_.push_back({std::u32string(), 0, -1, -1});
}

int Trie::nuevoNodo(std::u32string etiqueta, unsigned frecuencia,
                    int hijo, int hermano) {
    nodos_.push_back({std::move(etiqueta), frecuencia, hijo, hermano});
    return static_cast<int>(nodos_.size()) - 1;
}

int Trie::buscarHijo(int nodo, char32_t simbolo) const {
    int actual = nodos_[nodo].hijo;
    while (actual != -1 &&
           nodos_[actual].etiqueta.front() < simbolo) {
        actual = nodos_[actual].hermano;
    }
    if (actual != -1 && nodos_[actual].etiqueta.front() == simbolo) {
        return actual;
    }
    return -1;
}

int Trie::obtenerHijo(int nodo, char32_t simbolo) {
    int previo = -1;
    int actual = nodos_[nodo].hijo;
    while (actual != -1 && nodos_[actual].etiqueta.front() < simbolo) {
        previo = actual;
        actual = nodos_[actual].hermano;
    }
    if (actual != -1 && nodos_[actual].etiqueta.front() == simbolo) {
        return actual;
    }
    int creado = nuevoNodo(std::u32string(1, simbolo), 0, -1, actual);
    if (previo == -1) {
        nodos_[nodo].hijo = creado;
    } else {
        nodos_[previo].hermano = creado;
    }
    return creado;
}

void Trie::insertar(const std::u32string& palabra, unsigned frecuencia) {
    if (palabra.empty()) return;
    if (frecuencia == 0) frecuencia = 1;

    int actual = 0;
    std::size_t pos = 0;
    while (pos < palabra.size()) {
        int hijo = buscarHijo(actual, palabra[pos]);
        if (hijo == -1) {
            actual = obtenerHijo(actual, palabra[pos]);
            ++pos;
            continue;
        }

        const std::u32string etiqueta = nodos_[hijo].etiqueta;
        std::size_t comunes = 0;
        while (comunes < etiqueta.size() && pos + comunes < palabra.size() &&
               etiqueta[comunes] == palabra[pos + comunes]) {
            ++comunes;
        }

        if (comunes == etiqueta.size()) {
            actual = hijo;
            pos += comunes;
            continue;
        }

        // La palabra diverge dentro de una etiqueta comprimida: se parte el
        // arco en dos nodos para conservar el orden alfabetico de los hermanos.
        int mitad = nuevoNodo(etiqueta.substr(0, comunes), 0, hijo,
                              nodos_[hijo].hermano);
        nodos_[hijo].etiqueta = etiqueta.substr(comunes);
        nodos_[hijo].hermano = -1;

        int cursor = nodos_[actual].hijo;
        if (cursor == hijo) {
            nodos_[actual].hijo = mitad;
        } else {
            while (cursor != -1 && nodos_[cursor].hermano != hijo) {
                cursor = nodos_[cursor].hermano;
            }
            if (cursor != -1) nodos_[cursor].hermano = mitad;
        }

        actual = mitad;
        pos += comunes;
    }

    if (nodos_[actual].frecuencia == 0) ++palabras_;
    if (frecuencia > nodos_[actual].frecuencia) {
        nodos_[actual].frecuencia = frecuencia;
    }
}

int Trie::descender(const std::u32string& texto,
                    std::size_t& consumidos, bool estricto) const {
    int actual = 0;
    consumidos = 0;
    while (consumidos < texto.size()) {
        int hijo = buscarHijo(actual, texto[consumidos]);
        if (hijo == -1) return -1;
        ++visitados_;

        const std::u32string& etiqueta = nodos_[hijo].etiqueta;
        std::size_t comunes = 0;
        while (comunes < etiqueta.size() &&
               consumidos + comunes < texto.size() &&
               etiqueta[comunes] == texto[consumidos + comunes]) {
            ++comunes;
        }
        if (comunes == 0) return -1;
        if (estricto && comunes < etiqueta.size() &&
            consumidos + comunes == texto.size()) {
            return -1;
        }

        consumidos += comunes;
        actual = hijo;
    }
    return actual;
}

void Trie::prefijoHasta(const std::u32string& prefijo,
                        std::u32string& salida) const {
    int actual = 0;
    std::size_t consumidos = 0;
    while (consumidos < prefijo.size()) {
        int hijo = buscarHijo(actual, prefijo[consumidos]);
        if (hijo == -1) return;
        const std::u32string& etiqueta = nodos_[hijo].etiqueta;
        salida += etiqueta;
        consumidos += std::min(etiqueta.size(), prefijo.size() - consumidos);
        actual = hijo;
    }
}

bool Trie::contiene(const std::u32string& palabra) const {
    std::size_t consumidos = 0;
    int nodo = descender(palabra, consumidos, true);
    return nodo != -1 && consumidos == palabra.size() &&
           nodos_[nodo].frecuencia > 0;
}

unsigned Trie::frecuencia(const std::u32string& palabra) const {
    std::size_t consumidos = 0;
    int nodo = descender(palabra, consumidos, true);
    if (nodo == -1 || consumidos != palabra.size()) return 0;
    return nodos_[nodo].frecuencia;
}

bool Trie::eliminar(const std::u32string& palabra) {
    std::size_t consumidos = 0;
    int nodo = descender(palabra, consumidos, true);
    if (nodo == -1 || consumidos != palabra.size() ||
        nodos_[nodo].frecuencia == 0) {
        return false;
    }
    nodos_[nodo].frecuencia = 0;
    --palabras_;
    return true;
}

void Trie::fijarFrecuencia(const std::u32string& palabra,
                           unsigned frecuencia) {
    std::size_t consumidos = 0;
    int nodo = descender(palabra, consumidos, true);
    if (nodo == -1 || consumidos != palabra.size()) return;

    const unsigned anterior = nodos_[nodo].frecuencia;
    if (anterior == 0 && frecuencia > 0) ++palabras_;
    if (anterior > 0 && frecuencia == 0 && palabras_ > 0) --palabras_;
    nodos_[nodo].frecuencia = frecuencia;
}

void Trie::recolectar(int nodo, std::u32string& actual, std::size_t maximo,
                      std::vector<Sugerencia>& monton) const {
    ++visitados_;
    if (nodos_[nodo].frecuencia > 0) {
        Sugerencia candidata(texto::codificar(actual),
                             nodos_[nodo].frecuencia, 0);
        if (monton.size() < maximo) {
            monton.push_back(candidata);
            std::push_heap(monton.begin(), monton.end(), masFrecuente);
        } else if (masFrecuente(candidata, monton.front())) {
            std::pop_heap(monton.begin(), monton.end(), masFrecuente);
            monton.back() = candidata;
            std::push_heap(monton.begin(), monton.end(), masFrecuente);
        }
    }

    int hijo = nodos_[nodo].hijo;
    while (hijo != -1) {
        const std::u32string& etiqueta = nodos_[hijo].etiqueta;
        actual += etiqueta;
        recolectar(hijo, actual, maximo, monton);
        actual.resize(actual.size() - etiqueta.size());
        hijo = nodos_[hijo].hermano;
    }
}

std::vector<Sugerencia> Trie::autocompletar(const std::u32string& prefijo,
                                            std::size_t maximo) const {
    std::vector<Sugerencia> salida;
    if (maximo == 0) return salida;
    visitados_ = 0;

    std::size_t consumidos = 0;
    int nodo = descender(prefijo, consumidos);
    if (nodo == -1) return salida;

    std::u32string actual;
    prefijoHasta(prefijo, actual);
    salida.reserve(maximo);
    recolectar(nodo, actual, maximo, salida);
    std::sort(salida.begin(), salida.end(), masFrecuente);
    return salida;
}

std::size_t Trie::contarConPrefijo(const std::u32string& prefijo) const {
    std::size_t consumidos = 0;
    int nodo = descender(prefijo, consumidos);
    if (nodo == -1) return 0;

    std::size_t total = 0;
    std::vector<int> pila(1, nodo);
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

void Trie::explorar(int nodo, std::u32string& actual,
                    const std::u32string& objetivo,
                    const std::vector<int>& fila, int limite,
                    std::vector<Sugerencia>& salida) const {
    const std::size_t columnas = objetivo.size();
    int hijo = nodos_[nodo].hijo;
    while (hijo != -1) {
        ++visitados_;
        const std::u32string& etiqueta = nodos_[hijo].etiqueta;
        std::vector<int> anterior = fila;
        bool podado = false;

        for (char32_t simbolo : etiqueta) {
            std::vector<int> nueva(columnas + 1);
            nueva[0] = anterior[0] + 1;
            int mejor = nueva[0];
            for (std::size_t j = 1; j <= columnas; ++j) {
                const int costo = (objetivo[j - 1] == simbolo) ? 0 : 1;
                int valor = nueva[j - 1] + 1;
                valor = std::min(valor, anterior[j] + 1);
                valor = std::min(valor, anterior[j - 1] + costo);
                nueva[j] = valor;
                mejor = std::min(mejor, valor);
            }
            if (mejor > limite) {
                podado = true;
                break;
            }
            anterior.swap(nueva);
        }

        if (!podado) {
            actual += etiqueta;
            if (nodos_[hijo].frecuencia > 0 &&
                anterior[columnas] <= limite) {
                salida.push_back(Sugerencia(texto::codificar(actual),
                                            nodos_[hijo].frecuencia,
                                            anterior[columnas]));
            }
            explorar(hijo, actual, objetivo, anterior, limite, salida);
            actual.resize(actual.size() - etiqueta.size());
        }
        hijo = nodos_[hijo].hermano;
    }
}

std::vector<Sugerencia> Trie::similares(const std::u32string& objetivo,
                                        int limite, std::size_t maximo) const {
    std::vector<Sugerencia> salida;
    if (objetivo.empty() || limite < 0 || maximo == 0) return salida;
    visitados_ = 0;

    std::vector<int> fila(objetivo.size() + 1);
    for (std::size_t j = 0; j < fila.size(); ++j) {
        fila[j] = static_cast<int>(j);
    }
    std::u32string actual;
    actual.reserve(objetivo.size() + static_cast<std::size_t>(limite) + 1);
    explorar(0, actual, objetivo, fila, limite, salida);

    std::sort(salida.begin(), salida.end(), mejorCorreccion);
    if (salida.size() > maximo) salida.resize(maximo);
    return salida;
}

std::size_t Trie::nodosActivos() const {
    std::size_t total = 0;
    std::vector<int> pila(1, 0);
    while (!pila.empty()) {
        int actual = pila.back();
        pila.pop_back();
        ++total;
        int hijo = nodos_[actual].hijo;
        while (hijo != -1) {
            pila.push_back(hijo);
            hijo = nodos_[hijo].hermano;
        }
    }
    return total;
}

std::size_t Trie::aristas() const {
    std::size_t activos = nodosActivos();
    return activos == 0 ? 0 : activos - 1;
}

std::size_t Trie::profundidadMaxima() const {
    std::size_t maxima = 0;
    std::vector<std::pair<int, std::size_t> > pila;
    pila.push_back(std::make_pair(0, static_cast<std::size_t>(0)));
    while (!pila.empty()) {
        int nodo = pila.back().first;
        std::size_t nivel = pila.back().second + nodos_[nodo].etiqueta.size();
        pila.pop_back();
        maxima = std::max(maxima, nivel);
        int hijo = nodos_[nodo].hijo;
        while (hijo != -1) {
            pila.push_back(std::make_pair(hijo, nivel));
            hijo = nodos_[hijo].hermano;
        }
    }
    return maxima;
}

double Trie::ramificacionPromedio() const {
    std::size_t internos = 0;
    std::vector<int> pila(1, 0);
    while (!pila.empty()) {
        int actual = pila.back();
        pila.pop_back();
        if (nodos_[actual].hijo != -1) ++internos;
        int hijo = nodos_[actual].hijo;
        while (hijo != -1) {
            pila.push_back(hijo);
            hijo = nodos_[hijo].hermano;
        }
    }
    if (internos == 0) return 0.0;
    return static_cast<double>(aristas()) / static_cast<double>(internos);
}

std::size_t Trie::memoriaAproximada() const {
    std::size_t total = 0;
    std::vector<int> pila(1, 0);
    while (!pila.empty()) {
        int actual = pila.back();
        pila.pop_back();
        total += sizeof(Nodo) +
                 nodos_[actual].etiqueta.capacity() * sizeof(char32_t);
        int hijo = nodos_[actual].hijo;
        while (hijo != -1) {
            pila.push_back(hijo);
            hijo = nodos_[hijo].hermano;
        }
    }
    return total;
}

void Trie::compactar() {
    for (std::size_t p = 0; p < nodos_.size(); ++p) {
        int hijo = nodos_[p].hijo;
        while (hijo != -1) {
            int nieto = nodos_[hijo].hijo;
            const bool esUnario = nieto != -1 &&
                                  nodos_[nieto].hermano == -1;
            if (nodos_[hijo].frecuencia != 0 || !esUnario) {
                hijo = nodos_[hijo].hermano;
                continue;
            }

            const std::u32string fusionada =
                nodos_[hijo].etiqueta + nodos_[nieto].etiqueta;
            const int siguiente = nodos_[hijo].hermano;
            nodos_[nieto].etiqueta = fusionada;
            nodos_[nieto].hermano = siguiente;
            nodos_[hijo].hijo = -1;
            nodos_[hijo].hermano = -1;

            int cursor = nodos_[p].hijo;
            int anterior = -1;
            while (cursor != -1 && cursor != hijo) {
                anterior = cursor;
                cursor = nodos_[cursor].hermano;
            }
            if (anterior == -1) {
                nodos_[p].hijo = nieto;
            } else {
                nodos_[anterior].hermano = nieto;
            }

            ++nodosAhorrados_;
            bytesAhorrados_ += sizeof(Nodo);
            hijo = nieto;
        }
    }
    nodos_.shrink_to_fit();
}
