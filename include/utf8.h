#ifndef UTF8_H
#define UTF8_H

#include <string>
#include <vector>

#include "Normalizador.h"

namespace texto {

inline std::vector<char32_t> decodificar(const std::string& s) {
    return Normalizador::decodificarUTF8(s);
}

inline std::u32string u32(const std::string& s) {
    const std::vector<char32_t> puntos = decodificar(s);
    return std::u32string(puntos.begin(), puntos.end());
}

inline std::string codificar(const std::vector<char32_t>& puntos) {
    return Normalizador::codificarUTF8(puntos);
}

inline std::string codificar(const std::u32string& puntos) {
    return Normalizador::codificarUTF8(
        std::vector<char32_t>(puntos.begin(), puntos.end()));
}

}  // namespace texto

#endif
