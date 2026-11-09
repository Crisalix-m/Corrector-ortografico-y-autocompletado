#include "Normalizador.h"

namespace {

struct Simbolo {
    char32_t cp;        // punto de codigo
    std::size_t ini;    // primer byte en el texto original
    std::size_t fin;    // byte siguiente al ultimo
};

const char32_t COMBINANTE_AGUDO = 0x0301;
const char32_t COMBINANTE_TILDE = 0x0303;
const char32_t COMBINANTE_DIERESIS = 0x0308;
const char32_t LETRA_ENIE = 0x00F1;  // Ã±

bool esContinuacion(const std::string& s, std::size_t i) {
    return i < s.size() && (static_cast<unsigned char>(s[i]) & 0xC0) == 0x80;
}

// Decodifica UTF-8. Un byte que no forme una secuencia valida se interpreta
// como Latin-1 (asi un archivo guardado en Latin-1 tambien se normaliza bien).
std::vector<Simbolo> decodificar(const std::string& s) {
    std::vector<Simbolo> salida;
    std::size_t i = 0;
    while (i < s.size()) {
        auto byte = [&](std::size_t k) {
            return static_cast<unsigned char>(s[k]);
        };
        unsigned char c = byte(i);
        std::size_t largo = 1;
        char32_t cp = c;

        if (c >= 0xC2 && c <= 0xDF && esContinuacion(s, i + 1)) {
            largo = 2;
            cp = (static_cast<char32_t>(c & 0x1F) << 6) |
                 (byte(i + 1) & 0x3F);
        } else if ((c & 0xF0) == 0xE0 && esContinuacion(s, i + 1) &&
                   esContinuacion(s, i + 2)) {
            largo = 3;
            cp = (static_cast<char32_t>(c & 0x0F) << 12) |
                 (static_cast<char32_t>(byte(i + 1) & 0x3F) << 6) |
                 (byte(i + 2) & 0x3F);
        } else if (c >= 0xF0 && c <= 0xF4 && esContinuacion(s, i + 1) &&
                   esContinuacion(s, i + 2) && esContinuacion(s, i + 3)) {
            largo = 4;
            cp = (static_cast<char32_t>(c & 0x07) << 18) |
                 (static_cast<char32_t>(byte(i + 1) & 0x3F) << 12) |
                 (static_cast<char32_t>(byte(i + 2) & 0x3F) << 6) |
                 (byte(i + 3) & 0x3F);
        }
        salida.push_back({cp, i, i + largo});
        i += largo;
    }
    return salida;
}

void agregarUTF8(std::string& salida, char32_t cp) {
    if (cp < 0x80) {
        salida += static_cast<char>(cp);
    } else if (cp < 0x800) {
        salida += static_cast<char>(0xC0 | (cp >> 6));
        salida += static_cast<char>(0x80 | (cp & 0x3F));
    } else if (cp < 0x10000) {
        salida += static_cast<char>(0xE0 | (cp >> 12));
        salida += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        salida += static_cast<char>(0x80 | (cp & 0x3F));
    } else {
        salida += static_cast<char>(0xF0 | (cp >> 18));
        salida += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
        salida += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        salida += static_cast<char>(0x80 | (cp & 0x3F));
    }
}

// Si cp es una letra del espanol devuelve su forma normalizada
// (a-z o Ã±); si no lo es devuelve 0.
char32_t mapearLetra(char32_t cp) {
    if (cp >= 'a' && cp <= 'z') return cp;
    if (cp >= 'A' && cp <= 'Z') return cp - 'A' + 'a';
    switch (cp) {
        case 0xE1: case 0xC1: return 'a';              // Ã¡ Ã
        case 0xE9: case 0xC9: return 'e';              // Ã© Ã‰
        case 0xED: case 0xCD: return 'i';              // Ã­ Ã
        case 0xF3: case 0xD3: return 'o';              // Ã³ Ã“
        case 0xFA: case 0xDA:                          // Ãº Ãš
        case 0xFC: case 0xDC: return 'u';              // Ã¼ Ãœ
        case 0xF1: case 0xD1: return LETRA_ENIE;       // Ã± Ã‘
        default: return 0;
    }
}

bool esMarcaCombinante(char32_t cp) {
    return cp == COMBINANTE_AGUDO || cp == COMBINANTE_TILDE ||
           cp == COMBINANTE_DIERESIS;
}

}  // namespace

// ---------------------------------------------------------------------------

std::vector<char32_t> Normalizador::decodificarUTF8(const std::string& texto) {
    std::vector<char32_t> puntos;
    for (const Simbolo& s : decodificar(texto)) puntos.push_back(s.cp);
    return puntos;
}

std::string Normalizador::codificarUTF8(const std::vector<char32_t>& puntos) {
    std::string salida;
    for (char32_t cp : puntos) agregarUTF8(salida, cp);
    return salida;
}

std::string Normalizador::normalizar(const std::string& texto) {
    std::string salida;
    for (const Simbolo& s : decodificar(texto)) {
        char32_t letra = mapearLetra(s.cp);
        if (letra != 0) {
            agregarUTF8(salida, letra);
        } else if (s.cp == COMBINANTE_TILDE && !salida.empty() &&
                   salida.back() == 'n') {
            // n + tilde combinante (texto en forma NFD) -> Ã±
            salida.pop_back();
            agregarUTF8(salida, LETRA_ENIE);
        } else if (esMarcaCombinante(s.cp)) {
            // acento o dieresis combinantes sueltos: se descartan
        } else {
            salida.append(texto, s.ini, s.fin - s.ini);
        }
    }
    return salida;
}

std::vector<Token> Normalizador::tokenizar(const std::string& texto) {
    std::vector<Token> tokens;
    std::string actual;
    std::size_t ini = 0, fin = 0;
    bool enPalabra = false;

    auto cerrar = [&]() {
        if (enPalabra && !actual.empty()) {
            tokens.push_back({texto.substr(ini, fin - ini), actual, ini});
        }
        actual.clear();
        enPalabra = false;
    };

    for (const Simbolo& s : decodificar(texto)) {
        char32_t letra = mapearLetra(s.cp);
        if (letra != 0) {
            if (!enPalabra) {
                enPalabra = true;
                ini = s.ini;
                actual.clear();
            }
            agregarUTF8(actual, letra);
            fin = s.fin;
        } else if (enPalabra && s.cp == COMBINANTE_TILDE &&
                   !actual.empty() && actual.back() == 'n') {
            actual.pop_back();
            agregarUTF8(actual, LETRA_ENIE);
            fin = s.fin;
        } else if (enPalabra && esMarcaCombinante(s.cp)) {
            fin = s.fin;  // la marca pertenece a la palabra, no la parte
        } else {
            cerrar();
        }
    }
    cerrar();
    return tokens;
}

std::vector<std::string> Normalizador::separarPalabras(const std::string& texto) {
    std::vector<std::string> palabras;
    for (const Token& t : tokenizar(texto)) palabras.push_back(t.normalizada);
    return palabras;
}

int Normalizador::indiceLetra(char32_t c) {
    if (c >= 'a' && c <= 'z') return static_cast<int>(c - 'a');
    if (c == LETRA_ENIE) return 26;
    return -1;
}

bool Normalizador::esPalabraValida(const std::string& palabra) {
    std::vector<char32_t> puntos = decodificarUTF8(palabra);
    if (puntos.empty()) return false;
    for (char32_t c : puntos) {
        if (indiceLetra(c) < 0) return false;
    }
    return true;
}

std::vector<int> Normalizador::aIndices(const std::string& palabra) {
    std::vector<int> indices;
    for (char32_t c : decodificarUTF8(palabra)) indices.push_back(indiceLetra(c));
    return indices;
}