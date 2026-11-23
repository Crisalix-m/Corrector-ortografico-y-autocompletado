// Pruebas del Trie y de la compresion radix.
// Compilar desde la raiz:
//   g++ -std=c++17 -Wall -Wextra -Iinclude src/Trie.cpp src/Normalizador.cpp
//       tests/pruebas_trie.cpp -o pruebas_trie
#include <iostream>
#include <string>
#include <vector>

#include "trie.h"
#include "utf8.h"

static int total = 0;
static int fallos = 0;

#define VERIFICAR(cond)                                                   \
    do {                                                                  \
        ++total;                                                          \
        if (!(cond)) {                                                    \
            ++fallos;                                                     \
            std::cout << "FALLO linea " << __LINE__ << ": " #cond "\n";   \
        }                                                                 \
    } while (0)

static std::vector<Sugerencia> completar(const Trie& t, const char* prefijo,
                                         std::size_t maximo = 5) {
    return t.autocompletar(texto::u32(prefijo), maximo);
}

static void pruebasBasicas() {
    Trie t;
    t.insertar(texto::u32("casa"), 1);
    t.insertar(texto::u32("casita"), 3);
    t.insertar(texto::u32("caballo"), 2);
    t.insertar(texto::u32("perro"), 1);
    t.insertar(texto::u32("ni\xC3\xB1o"), 1);

    VERIFICAR(t.palabras() == 5);
    VERIFICAR(t.contiene(texto::u32("casa")));
    VERIFICAR(t.contiene(texto::u32("casita")));
    VERIFICAR(t.contiene(texto::u32("ni\xC3\xB1o")));
    VERIFICAR(!t.contiene(texto::u32("cas")));
    VERIFICAR(!t.contiene(texto::u32("cab")));
    VERIFICAR(t.frecuencia(texto::u32("casita")) == 3);
    VERIFICAR(t.contarConPrefijo(texto::u32("ca")) == 3);
    VERIFICAR(t.contarConPrefijo(texto::u32("zz")) == 0);
    VERIFICAR(t.profundidadMaxima() == 7);

    VERIFICAR(t.eliminar(texto::u32("casita")));
    VERIFICAR(!t.contiene(texto::u32("casita")));
    VERIFICAR(t.contarConPrefijo(texto::u32("ca")) == 2);
    VERIFICAR(!t.eliminar(texto::u32("casita")));
}

static void pruebasAutocompletado() {
    Trie t;
    t.insertar(texto::u32("hablar"), 1);
    t.insertar(texto::u32("hablaron"), 9);
    t.insertar(texto::u32("hablaba"), 4);
    t.insertar(texto::u32("habia"), 2);
    t.insertar(texto::u32("otra"), 7);

    std::vector<Sugerencia> found = completar(t, "habl", 5);
    VERIFICAR(found.size() == 3);
    VERIFICAR(found[0].palabra == "hablaron");
    VERIFICAR(found[1].palabra == "hablaba");
    VERIFICAR(found[2].palabra == "hablar");

    VERIFICAR(completar(t, "habl", 2).size() == 2);
    VERIFICAR(completar(t, "zzz").empty());
    VERIFICAR(completar(t, "habl", 0).empty());

    // El prefijo puede terminar en medio de una etiqueta comprimida.
    t.compactar();
    found = completar(t, "habl", 5);
    VERIFICAR(found.size() == 3);
    VERIFICAR(found[0].palabra == "hablaron");
}

static void pruebasSugerencias() {
    Trie t;
    t.insertar(texto::u32("casa"), 10);
    t.insertar(texto::u32("cosa"), 4);
    t.insertar(texto::u32("casas"), 2);
    t.insertar(texto::u32("carro"), 1);
    t.insertar(texto::u32("perro"), 1);

    std::vector<Sugerencia> s = t.similares(texto::u32("caza"), 2, 5);
    VERIFICAR(!s.empty());
    VERIFICAR(s.front().palabra == "casa");
    VERIFICAR(s.front().distancia == 1);
    VERIFICAR(s.size() <= 5);
    for (std::size_t i = 1; i < s.size(); ++i) {
        VERIFICAR(s[i - 1].distancia <= s[i].distancia);
    }
    VERIFICAR(t.similares(texto::u32("caza"), 0, 5).empty());
    VERIFICAR(t.nodosVisitados() > 0);
}

static void pruebasRadix() {
    Trie t;
    t.insertar(texto::u32("casita"), 1);
    t.insertar(texto::u32("caserita"), 1);
    t.insertar(texto::u32("caballo"), 1);
    t.insertar(texto::u32("casa"), 1);
    t.insertar(texto::u32("perro"), 1);

    const std::size_t antes = t.nodosActivos();
    t.compactar();
    VERIFICAR(t.nodosAhorrados() > 0);
    VERIFICAR(t.bytesAhorrados() >= t.nodosAhorrados() * sizeof(std::u32string));
    VERIFICAR(t.nodosActivos() < antes);
    VERIFICAR(t.memoriaAproximada() > 0);
    VERIFICAR(t.aristas() == t.nodosActivos() - 1);

    VERIFICAR(t.contiene(texto::u32("casita")));
    VERIFICAR(t.contiene(texto::u32("caserita")));
    VERIFICAR(t.contiene(texto::u32("caballo")));
    VERIFICAR(t.contiene(texto::u32("casa")));
    VERIFICAR(t.contiene(texto::u32("perro")));
    VERIFICAR(!t.contiene(texto::u32("caso")));

    // Insertar despues de compactar parte una etiqueta comprimida.
    t.insertar(texto::u32("casamiento"), 7);
    t.insertar(texto::u32("casita"), 5);
    VERIFICAR(t.palabras() == 6);
    VERIFICAR(t.contiene(texto::u32("casamiento")));
    VERIFICAR(t.contiene(texto::u32("casita")));
    VERIFICAR(t.frecuencia(texto::u32("casita")) == 5);
    VERIFICAR(t.contarConPrefijo(texto::u32("cas")) == 4);

    std::vector<Sugerencia> found = completar(t, "cas", 10);
    VERIFICAR(found.size() == 4);
    VERIFICAR(found.front().palabra == "casamiento");

    t.compactar();
    VERIFICAR(t.contiene(texto::u32("casamiento")));
    VERIFICAR(!t.contiene(texto::u32("caso")));
    VERIFICAR(completar(t, "cas", 10).size() == 4);
}

int main() {
    std::cout << "=== PRUEBAS TRIE ===\n";
    pruebasBasicas();
    pruebasAutocompletado();
    pruebasSugerencias();
    pruebasRadix();
    std::cout << "Pruebas del Trie: " << (total - fallos) << "/" << total
              << " correctas\n";
    return fallos == 0 ? 0 : 1;
}
