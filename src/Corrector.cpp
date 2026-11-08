
#include "Corrector.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <utility>

Corrector::Corrector(const std::string& rutaPersonal)
    : rutaPersonal_(rutaPersonal) {}

std::string Corrector::normalizarPersonal(
    const std::string& palabra
) {
    std::size_t inicio = 0;
    std::size_t fin = palabra.size();

    while (inicio < fin &&
           std::isspace(
               static_cast<unsigned char>(palabra[inicio])
           )) {
        ++inicio;
    }

    while (fin > inicio &&
           std::isspace(
               static_cast<unsigned char>(palabra[fin - 1])
           )) {
        --fin;
    }

    const std::string recortada =
        palabra.substr(inicio, fin - inicio);

    const std::pair<std::string, std::string>
        equivalencias[] = {
            {"Á", "á"}, {"É", "é"},
            {"Í", "í"}, {"Ó", "ó"},
            {"Ú", "ú"}, {"Ü", "ü"},
            {"Ñ", "ñ"}
        };

    std::string resultado;

    for (std::size_t i = 0; i < recortada.size();) {
        bool convertido = false;

        for (const auto& equivalencia : equivalencias) {
            if (recortada.compare(
                    i,
                    equivalencia.first.size(),
                    equivalencia.first
                ) == 0) {

                resultado += equivalencia.second;
                i += equivalencia.first.size();
                convertido = true;
                break;
            }
        }

        if (convertido) continue;

        const unsigned char c =
            static_cast<unsigned char>(recortada[i]);

        if (c >= 'A' && c <= 'Z') {
            resultado +=
                static_cast<char>(c - 'A' + 'a');
        } else {
            resultado += static_cast<char>(c);
        }

        ++i;
    }

    return resultado;
}

bool Corrector::palabraValida(
    const std::string& palabra
) {
    if (palabra.empty() || palabra.size() > 120) {
        return false;
    }

    const std::string caracteresEspanoles[] = {
        "á", "é", "í", "ó", "ú", "ü", "ñ"
    };

    for (std::size_t i = 0; i < palabra.size();) {
        const unsigned char c =
            static_cast<unsigned char>(palabra[i]);

        if (c >= 'a' && c <= 'z') {
            ++i;
            continue;
        }

        bool permitido = false;

        for (const auto& caracter : caracteresEspanoles) {
            if (palabra.compare(
                    i, caracter.size(), caracter
                ) == 0) {

                i += caracter.size();
                permitido = true;
                break;
            }
        }

        if (!permitido) return false;
    }

    return true;
}

bool Corrector::cargarDiccionarioPersonal() {
    std::ifstream archivo(
        rutaPersonal_, std::ios::binary
    );

    if (!archivo) return false;

    std::vector<std::string> nuevas;
    std::string linea;

    while (std::getline(archivo, linea)) {
        const std::string palabra =
            normalizarPersonal(linea);

        if (palabraValida(palabra) &&
            std::find(
                nuevas.begin(),
                nuevas.end(),
                palabra
            ) == nuevas.end()) {

            nuevas.push_back(palabra);
        }
    }

    if (archivo.bad()) return false;

    palabrasPersonales_ = std::move(nuevas);

    return true;
}

bool Corrector::guardar(
    const std::vector<std::string>& palabras
) const {
    std::ofstream archivo(
        rutaPersonal_,
        std::ios::binary | std::ios::trunc
    );

    if (!archivo) return false;

    for (const auto& palabra : palabras) {
        archivo << palabra << '\n';
    }

    archivo.flush();

    return archivo.good();
}

Corrector::Resultado
Corrector::agregarPalabraPersonal(
    const std::string& entrada
) {
    const std::string palabra =
        normalizarPersonal(entrada);

    if (!palabraValida(palabra)) {
        return Resultado::EntradaInvalida;
    }

    if (existePalabraPersonal(palabra)) {
        return Resultado::YaExiste;
    }

    std::vector<std::string> nuevas =
        palabrasPersonales_;

    nuevas.push_back(palabra);

    if (!guardar(nuevas)) {
        return Resultado::ErrorArchivo;
    }

    palabrasPersonales_ = std::move(nuevas);

    return Resultado::Exito;
}

Corrector::Resultado
Corrector::eliminarPalabraPersonal(
    const std::string& entrada
) {
    const std::string palabra =
        normalizarPersonal(entrada);

    if (!palabraValida(palabra)) {
        return Resultado::EntradaInvalida;
    }

    if (!existePalabraPersonal(palabra)) {
        return Resultado::NoEncontrada;
    }

    std::vector<std::string> nuevas =
        palabrasPersonales_;

    nuevas.erase(
        std::remove(
            nuevas.begin(),
            nuevas.end(),
            palabra
        ),
        nuevas.end()
    );

    if (!guardar(nuevas)) {
        return Resultado::ErrorArchivo;
    }

    palabrasPersonales_ = std::move(nuevas);

    return Resultado::Exito;
}

bool Corrector::existePalabraPersonal(
    const std::string& entrada
) const {
    const std::string palabra =
        normalizarPersonal(entrada);

    return std::find(
        palabrasPersonales_.begin(),
        palabrasPersonales_.end(),
        palabra
    ) != palabrasPersonales_.end();
}

const std::vector<std::string>&
Corrector::listarPalabrasPersonales() const {
    return palabrasPersonales_;
}
