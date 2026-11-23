// Pruebas de integracion: carga, verificacion, sugerencias, autocompletado
// y persistencia del diccionario personal.
// Compilar desde la raiz:
//   g++ -std=c++17 -Wall -Wextra -Iinclude src/*.cpp tests/pruebas_integradas.cpp
//       -o pruebas_integradas
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "Corrector.h"

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

namespace fs = std::filesystem;

static void escribir(const fs::path& ruta, const std::string& contenido) {
    std::ofstream archivo(ruta);
    archivo << contenido;
}

static bool contiene(const std::vector<Sugerencia>& sugerencias,
                     const std::string& palabra) {
    for (const Sugerencia& sugerencia : sugerencias) {
        if (sugerencia.palabra == palabra) return true;
    }
    return false;
}

static bool contieneTexto(const std::string& texto, const std::string& parte) {
    return texto.find(parte) != std::string::npos;
}

int main() {
    std::cout << "=== PRUEBAS INTEGRADAS ===\n";

    const fs::path dir = fs::temp_directory_path() / "corrector_pruebas";
    fs::create_directories(dir);
    const fs::path principal = dir / "diccionario.txt";
    const fs::path personal = dir / "personal.txt";
    const fs::path frecuencias = dir / "frecuencias.txt";

    escribir(principal,
             "hablar\nhablaron\nhabia\ncasa\ncasita\ncaballo\nperro\n"
             "algoritmo\nestructuras\nni\xC3\xB1o\n"
             "-acho, cha\ncanci\xC3\xB3n\n");
    escribir(personal, "chatgpt\n");
    escribir(frecuencias, "hablaron 12\n");

    Corrector::Opciones opciones;
    opciones.diccionario = principal.string();
    opciones.personal = personal.string();
    opciones.frecuencias = frecuencias.string();

    Corrector corrector(opciones);
    VERIFICAR(corrector.cargar());
    VERIFICAR(corrector.palabrasCargadas() == 12);

    // La carga normaliza y descarta entradas con puntuacion del diccionario
    // original; "cancion" y "nino" quedan disponibles.
    VERIFICAR(corrector.verificar("ni\xC3\xB1o").front().conocida);
    VERIFICAR(corrector.verificar("canci\xC3\xB3n").front().conocida);
    VERIFICAR(corrector.verificar("chatgpt").front().conocida);

    const std::string texto = "casa perrro caza hablaron";
    const std::vector<PalabraVerificada> resultados = corrector.verificar(texto);
    VERIFICAR(resultados.size() == 4);
    VERIFICAR(resultados[0].conocida);
    VERIFICAR(!resultados[1].conocida);
    VERIFICAR(!resultados[2].conocida);
    VERIFICAR(resultados[3].conocida);
    VERIFICAR(contiene(resultados[2].sugerencias, "casa"));
    VERIFICAR(contiene(resultados[1].sugerencias, "perro"));
    VERIFICAR(resultados[1].sugerencias.size() <= 5);
    VERIFICAR(contieneTexto(corrector.marcar(texto), "[caza]"));
    VERIFICAR(contieneTexto(corrector.marcar(texto), "[perrro]"));

    const std::vector<Sugerencia> completados = corrector.autocompletar("habl");
    VERIFICAR(completados.size() == 2);
    VERIFICAR(completados.front().palabra == "hablaron");
    VERIFICAR(completados.front().frecuencia == 12);
    VERIFICAR(corrector.autocompletar("zz").empty());

    VERIFICAR(corrector.agregarPersonal("frutilla"));
    VERIFICAR(corrector.verificar("frutilla").front().conocida);
    std::ifstream guardado(personal);
    std::string lineas;
    std::string linea;
    while (std::getline(guardado, linea)) lineas += linea + "\n";
    VERIFICAR(contieneTexto(lineas, "frutilla"));
    guardado.close();

    VERIFICAR(corrector.registrarUso("casa"));
    const std::vector<Sugerencia> trasUso = corrector.autocompletar("cas");
    VERIFICAR(trasUso.front().palabra == "casa");
    VERIFICAR(trasUso.front().frecuencia == 2);

    fs::remove_all(dir);
    std::cout << "Pruebas integradas: " << (total - fallos) << "/" << total
              << " correctas\n";
    return fallos == 0 ? 0 : 1;
}
