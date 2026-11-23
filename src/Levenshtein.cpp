#include "Levenshtein.h"

#include <algorithm>
#include <cstdlib>

#include "Normalizador.h"

namespace {

int minimo3(int a, int b, int c) {
    return std::min(a, std::min(b, c));
}

// Version con dos filas y corte anticipado. Se usa al comparar una palabra
// contra decenas de miles de candidatos.
// El minimo de cada fila de la matriz nunca disminuye de una fila a la
// siguiente; si ya supera el limite, la distancia final tambien lo supera.
int distanciaAcotadaPuntos(const std::vector<char32_t>& x,
                           const std::vector<char32_t>& y, int limite) {
    const int m = static_cast<int>(x.size());
    const int n = static_cast<int>(y.size());
    if (std::abs(m - n) > limite) return limite + 1;

    std::vector<int> anterior(n + 1), actual(n + 1);
    for (int j = 0; j <= n; ++j) anterior[j] = j;

    for (int i = 1; i <= m; ++i) {
        actual[0] = i;
        int minimoFila = actual[0];
        for (int j = 1; j <= n; ++j) {
            int costo = (x[i - 1] == y[j - 1]) ? 0 : 1;
            actual[j] = minimo3(anterior[j] + 1,        // eliminacion
                                actual[j - 1] + 1,      // insercion
                                anterior[j - 1] + costo);  // sustitucion
            minimoFila = std::min(minimoFila, actual[j]);
        }
        if (minimoFila > limite) return limite + 1;
        std::swap(anterior, actual);
    }
    return anterior[n] > limite ? limite + 1 : anterior[n];
}

}  // namespace

// ---------------------------------------------------------------------------

std::vector<std::vector<int>> Levenshtein::construirMatriz(const std::string& a,
                                                           const std::string& b) {
    const std::vector<char32_t> x = Normalizador::decodificarUTF8(a);
    const std::vector<char32_t> y = Normalizador::decodificarUTF8(b);
    const std::size_t m = x.size();
    const std::size_t n = y.size();

    std::vector<std::vector<int>> d(m + 1, std::vector<int>(n + 1, 0));

    // Primera columna y primera fila: convertir a/b en la cadena vacia.
    for (std::size_t i = 0; i <= m; ++i) d[i][0] = static_cast<int>(i);
    for (std::size_t j = 0; j <= n; ++j) d[0][j] = static_cast<int>(j);

    for (std::size_t i = 1; i <= m; ++i) {
        for (std::size_t j = 1; j <= n; ++j) {
            int costo = (x[i - 1] == y[j - 1]) ? 0 : 1;
            d[i][j] = minimo3(d[i - 1][j] + 1,           // eliminacion
                              d[i][j - 1] + 1,           // insercion
                              d[i - 1][j - 1] + costo);  // sustitucion
        }
    }
    return d;
}

int Levenshtein::distancia(const std::string& a, const std::string& b) {
    return construirMatriz(a, b).back().back();
}

int Levenshtein::distanciaAcotada(const std::string& a, const std::string& b,
                                  int limite) {
    if (limite < 0) limite = 0;
    return distanciaAcotadaPuntos(Normalizador::decodificarUTF8(a),
                                  Normalizador::decodificarUTF8(b), limite);
}

std::vector<Sugerencia> Levenshtein::sugerirConDistancia(
    const std::string& palabra, const std::vector<std::string>& candidatos,
    int maxSugerencias, int maxDistancia) {
    std::vector<Sugerencia> mejores;
    if (maxSugerencias <= 0 || maxDistancia < 1) return mejores;

    const std::vector<char32_t> objetivo = Normalizador::decodificarUTF8(palabra);
    const std::size_t capacidad = static_cast<std::size_t>(maxSugerencias);
    int limite = maxDistancia;

    for (const std::string& candidato : candidatos) {
        const std::vector<char32_t> puntos =
            Normalizador::decodificarUTF8(candidato);
        int d = distanciaAcotadaPuntos(objetivo, puntos, limite);
        if (d == 0 || d > limite) continue;

        // Insercion ordenada y estable: queda despues de los que tienen
        // la misma distancia.
        std::size_t pos = mejores.size();
        while (pos > 0 && mejores[pos - 1].distancia > d) --pos;
        if (pos >= capacidad) continue;

        mejores.insert(mejores.begin() + static_cast<std::ptrdiff_t>(pos),
                       Sugerencia{candidato, d});
        if (mejores.size() > capacidad) mejores.pop_back();

        // Con la lista llena solo entra quien mejore al ultimo.
        if (mejores.size() == capacidad) {
            limite = std::min(maxDistancia, mejores.back().distancia - 1);
            if (limite < 1) break;  // ya no puede entrar nadie mas
        }
    }
    return mejores;
}

std::vector<std::string> Levenshtein::sugerir(
    const std::string& palabra, const std::vector<std::string>& candidatos,
    int maxSugerencias, int maxDistancia) {
    std::vector<std::string> palabras;
    for (const Sugerencia& s :
         sugerirConDistancia(palabra, candidatos, maxSugerencias, maxDistancia)) {
        palabras.push_back(s.palabra);
    }
    return palabras;
}