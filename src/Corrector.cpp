#include "Corrector.h"

#include <algorithm>
#include <fstream>
#include <limits>
#include <sstream>
#include <utility>

#include "Levenshtein.h"
#include "utf8.h"

namespace {

std::string normalizarLinea(const std::string& linea) {
    std::string limpia = linea;
    if (!limpia.empty() && limpia.back() == '\r') limpia.pop_back();
    const std::string normalizada = Normalizador::normalizar(limpia);
    if (!Normalizador::esPalabraValida(normalizada)) return std::string();
    return normalizada;
}

std::vector<std::string> leerLineas(const std::string& ruta) {
    std::vector<std::string> lineas;
    std::ifstream archivo(ruta);
    if (!archivo.is_open()) return lineas;
    std::string linea;
    while (std::getline(archivo, linea)) lineas.push_back(linea);
    return lineas;
}

}  // namespace

Corrector::Corrector() : Corrector(Opciones()) {}

Corrector::Corrector(Opciones opciones)
    : opciones_(std::move(opciones)), cargadas_(0) {}

void Corrector::insertarPalabra(const std::string& palabra,
                                std::size_t frecuencia) {
    std::size_t anterior = 0;
    if (hash_.obtenerFrecuencia(palabra, anterior)) {
        const std::size_t nueva = anterior + frecuencia;
        hash_.fijarFrecuencia(palabra, nueva);
        trie_.fijarFrecuencia(texto::u32(palabra),
                              static_cast<unsigned>(
                                  std::min<std::size_t>(nueva,
                                      std::numeric_limits<unsigned>::max())));
        return;
    }

    hash_.insertar(palabra, frecuencia);
    trie_.insertar(texto::u32(palabra),
                   static_cast<unsigned>(
                       std::min<std::size_t>(frecuencia,
                           std::numeric_limits<unsigned>::max())));
}

bool Corrector::insertarDesdeArchivo(const std::string& ruta,
                                     std::size_t frecuencia) {
    std::ifstream archivo(ruta);
    if (!archivo.is_open()) return false;

    std::string linea;
    while (std::getline(archivo, linea)) {
        const std::string palabra = normalizarLinea(linea);
        if (!palabra.empty()) insertarPalabra(palabra, frecuencia);
    }
    return !archivo.bad();
}

void Corrector::aplicarFrecuencias() {
    const std::vector<std::string> lineas = leerLineas(opciones_.frecuencias);
    for (const std::string& linea : lineas) {
        std::istringstream entrada(linea);
        std::string cruda;
        unsigned long long valor = 0;
        if (!(entrada >> cruda >> valor)) continue;
        const std::string palabra = normalizarLinea(cruda);
        if (palabra.empty() || !hash_.buscar(palabra)) continue;

        const std::size_t frecuencia = static_cast<std::size_t>(
            std::min<unsigned long long>(valor,
                std::numeric_limits<std::size_t>::max()));
        hash_.fijarFrecuencia(palabra, frecuencia);
        trie_.fijarFrecuencia(texto::u32(palabra),
                              static_cast<unsigned>(
                                  std::min<std::size_t>(frecuencia,
                                      std::numeric_limits<unsigned>::max())));
    }
}

bool Corrector::cargar() {
    if (!insertarDesdeArchivo(opciones_.diccionario, 1)) return false;
    insertarDesdeArchivo(opciones_.personal, 2);
    aplicarFrecuencias();
    trie_.compactar();
    cargadas_ = hash_.tamano();
    return true;
}

std::size_t Corrector::palabrasCargadas() const {
    return cargadas_;
}

Corrector::Estadisticas Corrector::estadisticas() const {
    Estadisticas resultado;
    resultado.palabras = cargadas_;
    resultado.nodosTrie = trie_.nodosActivos();
    resultado.nodosComprimidos = trie_.nodosAhorrados();
    resultado.bytesAhorrados = trie_.bytesAhorrados();
    resultado.memoriaTrie = trie_.memoriaAproximada();
    return resultado;
}

std::vector<PalabraVerificada> Corrector::verificar(
    const std::string& texto) const {
    const std::vector<Token> tokens = Normalizador::tokenizar(texto);
    std::vector<PalabraVerificada> resultado;
    resultado.reserve(tokens.size());

    for (const Token& token : tokens) {
        PalabraVerificada item;
        item.token = token;
        item.conocida = hash_.buscar(token.normalizada);
        if (!item.conocida) item.sugerencias = sugerir(token.normalizada);
        resultado.push_back(std::move(item));
    }
    return resultado;
}

std::string Corrector::marcar(const std::string& texto) const {
    const std::vector<PalabraVerificada> palabras = verificar(texto);
    std::string salida;
    std::size_t cursor = 0;
    for (const PalabraVerificada& item : palabras) {
        const std::size_t inicio = item.token.posicion;
        if (inicio >= cursor && inicio <= texto.size()) {
            salida.append(texto, cursor, inicio - cursor);
        }
        if (item.conocida) {
            salida += item.token.original;
        } else {
            salida += '[';
            salida += item.token.original;
            salida += ']';
        }
        cursor = inicio + item.token.original.size();
    }
    if (cursor < texto.size()) salida.append(texto, cursor, texto.size() - cursor);
    return salida;
}

std::vector<Sugerencia> Corrector::sugerir(const std::string& palabra) const {
    const std::string normalizada = Normalizador::normalizar(palabra);
    if (!Normalizador::esPalabraValida(normalizada)) return std::vector<Sugerencia>();
    if (hash_.buscar(normalizada)) return std::vector<Sugerencia>();

    std::vector<Sugerencia> resultado = trie_.similares(
        texto::u32(normalizada), opciones_.distanciaMaxima,
        opciones_.maxSugerencias);

    if (resultado.empty()) {
        resultado = Levenshtein::sugerirConDistancia(
            normalizada, hash_.palabras(),
            static_cast<int>(opciones_.maxSugerencias),
            opciones_.distanciaMaxima);
    }
    return resultado;
}

std::vector<Sugerencia> Corrector::autocompletar(
    const std::string& prefijo) const {
    const std::string normalizado = Normalizador::normalizar(prefijo);
    if (!Normalizador::esPalabraValida(normalizado)) {
        return std::vector<Sugerencia>();
    }
    return trie_.autocompletar(texto::u32(normalizado),
                               opciones_.maxAutocompletados);
}

bool Corrector::agregarPersonal(const std::string& palabra) {
    const std::string normalizada = Normalizador::normalizar(palabra);
    if (!Normalizador::esPalabraValida(normalizada)) return false;

    const std::vector<std::string> lineas = leerLineas(opciones_.personal);
    for (const std::string& linea : lineas) {
        if (normalizarLinea(linea) == normalizada) return registrarUso(normalizada);
    }

    insertarPalabra(normalizada, 2);
    std::ofstream archivo(opciones_.personal, std::ios::app);
    if (!archivo.is_open()) return false;
    archivo << normalizada << '\n';
    return archivo.good() && guardarFrecuencias();
}

bool Corrector::registrarUso(const std::string& palabra) {
    const std::string normalizada = Normalizador::normalizar(palabra);
    if (!Normalizador::esPalabraValida(normalizada)) return false;

    std::size_t frecuencia = 0;
    if (!hash_.obtenerFrecuencia(normalizada, frecuencia)) return false;
    ++frecuencia;
    hash_.fijarFrecuencia(normalizada, frecuencia);
    trie_.fijarFrecuencia(texto::u32(normalizada),
                          static_cast<unsigned>(
                              std::min<std::size_t>(frecuencia,
                                  std::numeric_limits<unsigned>::max())));
    return guardarFrecuencias();
}

bool Corrector::guardarFrecuencias() const {
    std::ofstream archivo(opciones_.frecuencias);
    if (!archivo.is_open()) return false;

    const std::vector<std::string> palabras = hash_.palabras();
    for (const std::string& palabra : palabras) {
        std::size_t frecuencia = 0;
        if (!hash_.obtenerFrecuencia(palabra, frecuencia)) continue;
        if (frecuencia == 1) continue;
        archivo << palabra << ' ' << frecuencia << '\n';
        if (!archivo.good()) return false;
    }
    return true;
}
