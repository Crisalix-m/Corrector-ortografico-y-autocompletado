#ifndef LEVENSHTEIN_H
#define LEVENSHTEIN_H

#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Levenshtein: distancia de edicion entre palabras y generacion de
// sugerencias de correccion.
//
// La distancia es el minimo numero de operaciones (insertar, eliminar o
// sustituir un caracter) para convertir una palabra en otra. Se calcula sobre
// caracteres, no sobre bytes: la ñ (2 bytes en UTF-8) cuenta como 1 letra.
//
// Complejidad de comparar dos palabras de longitud m y n:
//   tiempo O(m * n),  espacio O(m * n) en distancia() / O(n) en la acotada.
// ---------------------------------------------------------------------------

struct Sugerencia {
    std::string palabra;
    int distancia;
};

class Levenshtein {
public:
    static constexpr int MAX_SUGERENCIAS = 5;        // requisito del proyecto
    static constexpr int DISTANCIA_MAXIMA = 2;       // por defecto

    // construirMatriz(a, b)
    //   Poscondicion:  matriz d de (|a|+1) x (|b|+1) donde d[i][j] es la
    //                  distancia entre los primeros i caracteres de a y los
    //                  primeros j de b. Util para mostrar el algoritmo.
    static std::vector<std::vector<int>> construirMatriz(const std::string& a,
                                                         const std::string& b);

    // distancia(a, b)
    //   Poscondicion:  devuelve d[|a|][|b|]. Es simetrica, vale 0 solo si
    //                  a == b, y distancia("", b) == |b|.
    static int distancia(const std::string& a, const std::string& b);

    // distanciaAcotada(a, b, limite)
    //   Como distancia(), pero si la distancia real supera `limite` devuelve
    //   limite + 1 sin terminar de calcularla (descarta candidatos rapido).
    //   Precondicion:  limite >= 0.
    static int distanciaAcotada(const std::string& a, const std::string& b,
                                int limite);

    // sugerirConDistancia(palabra, candidatos, maxSugerencias, maxDistancia)
    //   Precondicion:  palabra y candidatos estan normalizados
    //                  (ver Normalizador).
    //   Poscondicion:  hasta maxSugerencias candidatos con distancia entre 1 y
    //                  maxDistancia, ordenados por distancia creciente.
    //                  A igual distancia se respeta el orden de `candidatos`
    //                  (si se pasan ordenados por frecuencia, desempata la
    //                  frecuencia). Un candidato igual a la palabra no se
    //                  sugiere.
    static std::vector<Sugerencia> sugerirConDistancia(
        const std::string& palabra,
        const std::vector<std::string>& candidatos,
        int maxSugerencias = MAX_SUGERENCIAS,
        int maxDistancia = DISTANCIA_MAXIMA);

    // sugerir(...)  Igual que la anterior, pero devuelve solo las palabras.
    static std::vector<std::string> sugerir(
        const std::string& palabra,
        const std::vector<std::string>& candidatos,
        int maxSugerencias = MAX_SUGERENCIAS,
        int maxDistancia = DISTANCIA_MAXIMA);
};

#endif
