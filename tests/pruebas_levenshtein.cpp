// Pruebas de Levenshtein.
// Compilar (desde la raiz del proyecto):
//   g++ -std=c++17 -Wall -Wextra -Iinclude tests/pruebas_levenshtein.cpp
//       src/Levenshtein.cpp src/Normalizador.cpp -o pruebas_levenshtein
// (el comando va en una sola linea)
#include <iostream>
#include <string>
#include <vector>

#include "Levenshtein.h"

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

// Ã± en UTF-8 (se arma por partes para evitar problemas con escapes hex).
static const std::string N = "\xC3\xB1";

static unsigned int semilla = 12345;
static unsigned int siguiente() {
    semilla = semilla * 1103515245u + 12345u;
    return (semilla >> 16) & 0x7FFF;
}

static std::string palabraAleatoria() {
    const std::string alfabeto[] = {"a", "b", "c", "d", N};
    int largo = static_cast<int>(siguiente() % 8);
    std::string p;
    for (int i = 0; i < largo; ++i) p += alfabeto[siguiente() % 5];
    return p;
}

static void pruebasDistancia() {
    VERIFICAR(Levenshtein::distancia("casa", "cosa") == 1);   // sustitucion
    VERIFICAR(Levenshtein::distancia("casa", "casas") == 1);  // insercion
    VERIFICAR(Levenshtein::distancia("casas", "casa") == 1);  // eliminacion
    VERIFICAR(Levenshtein::distancia("casa", "casa") == 0);
    VERIFICAR(Levenshtein::distancia("", "") == 0);
    VERIFICAR(Levenshtein::distancia("", "abc") == 3);
    VERIFICAR(Levenshtein::distancia("abc", "") == 3);
    VERIFICAR(Levenshtein::distancia("kitten", "sitting") == 3);
    VERIFICAR(Levenshtein::distancia("computaodra", "computadora") == 2);
    VERIFICAR(Levenshtein::distancia("estudainte", "estudiante") == 2);

    // La Ã± cuenta como un solo caracter (no como 2 bytes).
    VERIFICAR(Levenshtein::distancia("a" + N + "o", "ano") == 1);
    VERIFICAR(Levenshtein::distancia("ni" + N + "o", "ni" + N + "a") == 1);
    VERIFICAR(Levenshtein::distancia(N, N) == 0);
}

static void pruebasMatriz() {
    auto d = Levenshtein::construirMatriz("casa", "cosa");
    VERIFICAR(d.size() == 5);
    VERIFICAR(d[0].size() == 5);
    for (int i = 0; i <= 4; ++i) {
        VERIFICAR(d[i][0] == i);
        VERIFICAR(d[0][i] == i);
    }
    VERIFICAR(d[4][4] == 1);
    VERIFICAR(d[1][1] == 0);  // "c" vs "c"
    VERIFICAR(d[2][2] == 1);  // "ca" vs "co"

    auto vacia = Levenshtein::construirMatriz("", "");
    VERIFICAR(vacia.size() == 1 && vacia[0].size() == 1 && vacia[0][0] == 0);
}

static void pruebasAcotada() {
    VERIFICAR(Levenshtein::distanciaAcotada("casa", "cosa", 2) == 1);
    VERIFICAR(Levenshtein::distanciaAcotada("casa", "mesa", 2) == 2);
    VERIFICAR(Levenshtein::distanciaAcotada("casa", "perro", 2) == 3);  // > limite
    VERIFICAR(Levenshtein::distanciaAcotada("a", "abcdefgh", 2) == 3);  // largo

    // Debe coincidir con la version completa (o valer limite+1 si la supera).
    for (int k = 0; k < 3000; ++k) {
        std::string a = palabraAleatoria();
        std::string b = palabraAleatoria();
        int real = Levenshtein::distancia(a, b);
        VERIFICAR(Levenshtein::distancia(b, a) == real);  // simetria
        for (int limite = 0; limite <= 3; ++limite) {
            int esperado = real <= limite ? real : limite + 1;
            VERIFICAR(Levenshtein::distanciaAcotada(a, b, limite) == esperado);
        }
    }
}

static void pruebasSugerencias() {
    const std::vector<std::string> dic = {
        "computadora", "computadoras", "computado", "computacion",
        "compilador",  "cosa",         "caso",      "mesa",
        "perro",       "casa",         "cuna"};

    auto s = Levenshtein::sugerirConDistancia("computaodra", dic);
    VERIFICAR(!s.empty());
    VERIFICAR(s[0].palabra == "computadora");
    VERIFICAR(s[0].distancia == 2);
    VERIFICAR(s.size() == 1);  // las demas estan a distancia 3 o mas

    auto s3 = Levenshtein::sugerir("computaodra", dic, 5, 3);
    VERIFICAR(s3.size() >= 3);
    VERIFICAR(s3[0] == "computadora");  // la mas cercana primero

    // Una palabra del diccionario no se sugiere a si misma; se respeta el
    // orden de entrada al empatar.
    auto c = Levenshtein::sugerir("casa", dic, 5, 1);
    VERIFICAR(c.size() == 2);
    VERIFICAR(c[0] == "cosa");
    VERIFICAR(c[1] == "caso");

    // Maximo cinco, ordenadas por distancia.
    std::vector<std::string> muchas;
    for (char ch = 'a'; ch <= 'z'; ++ch) muchas.push_back(std::string("casa") + ch);
    muchas.push_back("cas");
    muchas.push_back("cazas");
    auto cinco = Levenshtein::sugerirConDistancia("casa", muchas);
    VERIFICAR(cinco.size() == 5);
    for (std::size_t i = 1; i < cinco.size(); ++i)
        VERIFICAR(cinco[i - 1].distancia <= cinco[i].distancia);
    for (const auto& item : cinco) VERIFICAR(item.distancia >= 1 && item.distancia <= 2);

    // Si hay mas de cinco con distancia 1, ninguna de distancia 2 entra.
    std::vector<std::string> mezcla = {"cassa", "ccasa", "casaa", "caasa",
                                       "cassa2", "casaz", "caza", "cosa",
                                       "xxsa", "cusa"};
    auto r = Levenshtein::sugerirConDistancia("casa", mezcla);
    VERIFICAR(r.size() == 5);
    for (const auto& item : r) VERIFICAR(item.distancia == 1);

    // Casos limite.
    VERIFICAR(Levenshtein::sugerir("casa", {}).empty());
    VERIFICAR(Levenshtein::sugerir("casa", dic, 0).empty());
    VERIFICAR(Levenshtein::sugerir("casa", dic, 5, 0).empty());
    VERIFICAR(Levenshtein::sugerir("zzzzzzzz", dic).empty());

    // Ã± en sugerencias: "ano" -> "a" + Ã± + "o" (distancia 1).
    auto n = Levenshtein::sugerir("ano", {"a" + N + "o", "mesa"});
    VERIFICAR(n.size() == 1 && n[0] == "a" + N + "o");
}

int main() {
    pruebasDistancia();
    pruebasMatriz();
    pruebasAcotada();
    pruebasSugerencias();

    std::cout << "Pruebas de Levenshtein: " << (total - fallos) << "/" << total
              << " correctas\n";
    return fallos == 0 ? 0 : 1;
}