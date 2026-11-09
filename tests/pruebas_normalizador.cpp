// Pruebas del Normalizador.
// Compilar (desde la raiz del proyecto):
//   g++ -std=c++17 -Wall -Wextra -Iinclude tests/pruebas_normalizador.cpp
//       src/Normalizador.cpp -o pruebas_normalizador
// (el comando va en una sola linea)
#include <iostream>
#include <string>
#include <vector>

#include "Normalizador.h"

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

// Caracteres UTF-8 (se concatenan por partes para evitar que un escape hex
// absorba la letra que le sigue).
static const std::string N = "\xC3\xB1";              // Ã±
static const std::string N_MAY = "\xC3\x91";          // Ã‘
static const std::string A_T = "\xC3\xA1";            // Ã¡
static const std::string E_T = "\xC3\xA9";            // Ã©
static const std::string I_T = "\xC3\xAD";            // Ã­
static const std::string O_T = "\xC3\xB3";            // Ã³
static const std::string U_T = "\xC3\xBA";            // Ãº
static const std::string U_D = "\xC3\xBC";            // Ã¼
static const std::string ABRE_INT = "\xC2\xBF";       // Â¿
static const std::string ABRE_EXC = "\xC2\xA1";       // Â¡

static void pruebasNormalizar() {
    VERIFICAR(Normalizador::normalizar("Casa") == "casa");
    VERIFICAR(Normalizador::normalizar("canci" + O_T + "n") == "cancion");
    VERIFICAR(Normalizador::normalizar(A_T + E_T + I_T + O_T + U_T) == "aeiou");
    VERIFICAR(Normalizador::normalizar("\xC3\x81\xC3\x89\xC3\x8D\xC3\x93\xC3\x9A") ==
              "aeiou");  // mayusculas con tilde
    VERIFICAR(Normalizador::normalizar("ping" + U_D + "ino") == "pinguino");
    VERIFICAR(Normalizador::normalizar("NI" + N_MAY + "O") == "ni" + N + "o");
    VERIFICAR(Normalizador::normalizar("a" + N + "o") == "a" + N + "o");  // Ã± se conserva
    VERIFICAR(Normalizador::normalizar("Hola, mundo 123!") == "hola, mundo 123!");
    VERIFICAR(Normalizador::normalizar("") == "");

    // Texto en forma NFD (letra + marca combinante).
    VERIFICAR(Normalizador::normalizar("esta\xCC\x81") == "esta");
    VERIFICAR(Normalizador::normalizar("n\xCC\x83") == N);

    // Archivo Latin-1: un byte suelto 0xE9 es 'Ã©'.
    VERIFICAR(Normalizador::normalizar("caf\xE9") == "cafe");
}

static void pruebasSeparar() {
    auto p = Normalizador::separarPalabras("El estudainte esta aprendiendo programacion.");
    VERIFICAR(p.size() == 5);
    VERIFICAR(p[0] == "el" && p[1] == "estudainte" && p[4] == "programacion");

    auto q = Normalizador::separarPalabras(ABRE_INT + "C" + O_T + "mo est" + A_T +
                                           "s? " + ABRE_EXC + "Bien!");
    VERIFICAR(q.size() == 3);
    VERIFICAR(q[0] == "como" && q[1] == "estas" && q[2] == "bien");

    auto r = Normalizador::separarPalabras("abc123def  x-y don't");
    VERIFICAR(r.size() == 6);
    VERIFICAR(r[0] == "abc" && r[1] == "def" && r[2] == "x" && r[3] == "y");

    VERIFICAR(Normalizador::separarPalabras("").empty());
    VERIFICAR(Normalizador::separarPalabras("  ... 123 !!").empty());
    VERIFICAR(Normalizador::separarPalabras("UNMSM")[0] == "unmsm");
}

static void pruebasTokenizar() {
    auto t = Normalizador::tokenizar("Hola  Mundo");
    VERIFICAR(t.size() == 2);
    VERIFICAR(t[0].original == "Hola" && t[0].normalizada == "hola" && t[0].posicion == 0);
    VERIFICAR(t[1].original == "Mundo" && t[1].normalizada == "mundo" && t[1].posicion == 6);

    auto u = Normalizador::tokenizar("est" + A_T + " aqu" + I_T);
    VERIFICAR(u.size() == 2);
    VERIFICAR(u[0].original == "est" + A_T && u[0].normalizada == "esta");
    VERIFICAR(u[1].normalizada == "aqui");

    auto v = Normalizador::tokenizar("esta\xCC\x81 bien");  // marca combinante
    VERIFICAR(v.size() == 2);
    VERIFICAR(v[0].original == "esta\xCC\x81" && v[0].normalizada == "esta");
}

static void pruebasIndices() {
    VERIFICAR(Normalizador::esPalabraValida("casa"));
    VERIFICAR(Normalizador::esPalabraValida("a" + N + "o"));
    VERIFICAR(!Normalizador::esPalabraValida(""));
    VERIFICAR(!Normalizador::esPalabraValida("Casa"));       // no normalizada
    VERIFICAR(!Normalizador::esPalabraValida("ca-sa"));
    VERIFICAR(!Normalizador::esPalabraValida("canci" + O_T + "n"));

    VERIFICAR(Normalizador::indiceLetra('a') == 0);
    VERIFICAR(Normalizador::indiceLetra('z') == 25);
    VERIFICAR(Normalizador::indiceLetra(0x00F1) == 26);
    VERIFICAR(Normalizador::indiceLetra('A') == -1);
    VERIFICAR(Normalizador::indiceLetra('1') == -1);

    auto idx = Normalizador::aIndices("a" + N + "o");
    VERIFICAR(idx.size() == 3 && idx[0] == 0 && idx[1] == 26 && idx[2] == 14);

    auto cps = Normalizador::decodificarUTF8("a" + N + "o");
    VERIFICAR(cps.size() == 3 && cps[1] == 0x00F1);
    VERIFICAR(Normalizador::codificarUTF8(cps) == "a" + N + "o");
}

int main() {
    pruebasNormalizar();
    pruebasSeparar();
    pruebasTokenizar();
    pruebasIndices();

    std::cout << "Pruebas del Normalizador: " << (total - fallos) << "/" << total
              << " correctas\n";
    return fallos == 0 ? 0 : 1;
}