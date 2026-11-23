#include "Corrector.h"

#include <iostream>
#include <string>
#include <vector>

namespace {

void ayuda() {
    std::cout
        << "Corrector ortografico y autocompletado\n"
        << "Uso: corrector <comando> [texto] [opciones]\n\n"
        << "Comandos:\n"
        << "  verificar <texto>   marca las palabras desconocidas\n"
        << "  sugerir <palabra>   hasta 5 correcciones por distancia y frecuencia\n"
        << "  completar <prefijo> palabras con ese prefijo, ordenadas por frecuencia\n"
        << "  agregar <palabra>   agrega una palabra al diccionario personal\n"
        << "  stats               muestra el tamano del diccionario cargado\n\n"
        << "Opciones:\n"
        << "  --diccionario=ruta  (por defecto data/Diccionario.txt)\n"
        << "  --personal=ruta     (por defecto data/diccionario_personal.txt)\n"
        << "  --frecuencias=ruta  (por defecto data/frecuencias.txt)\n";
}

bool leerOpcion(const std::string& argumento, const std::string& nombre,
                std::string& destino) {
    const std::string prefijo = "--" + nombre + "=";
    if (argumento.compare(0, prefijo.size(), prefijo) != 0) return false;
    destino = argumento.substr(prefijo.size());
    return true;
}

}  // namespace

int main(int argc, char* argv[]) {
    if (argc < 2) {
        ayuda();
        return 1;
    }

    const std::string comando = argv[1];
    if (comando == "ayuda" || comando == "--help" || comando == "-h") {
        ayuda();
        return 0;
    }

    Corrector::Opciones opciones;
    std::vector<std::string> palabras;
    for (int i = 2; i < argc; ++i) {
        const std::string argumento = argv[i];
        if (leerOpcion(argumento, "diccionario", opciones.diccionario)) continue;
        if (leerOpcion(argumento, "personal", opciones.personal)) continue;
        if (leerOpcion(argumento, "frecuencias", opciones.frecuencias)) continue;
        palabras.push_back(argumento);
    }

    std::string texto;
    for (std::size_t i = 0; i < palabras.size(); ++i) {
        if (i != 0) texto += ' ';
        texto += palabras[i];
    }

    if (comando != "verificar" && comando != "sugerir" &&
        comando != "completar" && comando != "agregar" && comando != "stats") {
        std::cerr << "Comando desconocido: " << comando << "\n";
        ayuda();
        return 1;
    }

    Corrector corrector(opciones);
    if (!corrector.cargar()) {
        std::cerr << "No se pudo cargar el diccionario: "
                  << opciones.diccionario << "\n";
        return 1;
    }

    if (comando == "stats") {
        const Corrector::Estadisticas datos = corrector.estadisticas();
        std::cout << "Palabras cargadas: " << datos.palabras << "\n"
                  << "Nodos del trie: " << datos.nodosTrie << "\n"
                  << "Nodos comprimidos: " << datos.nodosComprimidos << "\n"
                  << "Bytes ahorrados: " << datos.bytesAhorrados << "\n"
                  << "Memoria aproximada del trie: " << datos.memoriaTrie
                  << " bytes\n";
        return 0;
    }

    if (texto.empty()) {
        std::cerr << "Falta el texto para " << comando << "\n";
        return 1;
    }

    if (comando == "verificar") {
        const std::vector<PalabraVerificada> resultados =
            corrector.verificar(texto);
        for (const PalabraVerificada& item : resultados) {
            if (item.conocida) continue;
            std::cout << "Desconocida: " << item.token.original << " -> ";
            if (item.sugerencias.empty()) {
                std::cout << "(sin sugerencias)";
            } else {
                for (std::size_t i = 0; i < item.sugerencias.size(); ++i) {
                    if (i != 0) std::cout << ", ";
                    std::cout << item.sugerencias[i].palabra;
                }
            }
            std::cout << "\n";
        }
        std::cout << "Marcado: " << corrector.marcar(texto) << "\n";
        return 0;
    }

    if (comando == "sugerir" || comando == "completar") {
        const std::vector<Sugerencia> resultados =
            comando == "sugerir" ? corrector.sugerir(texto)
                                 : corrector.autocompletar(texto);
        for (const Sugerencia& item : resultados) {
            std::cout << item.palabra;
            if (comando == "sugerir") {
                std::cout << " (distancia=" << item.distancia << ", frecuencia="
                          << item.frecuencia << ")";
            } else {
                std::cout << " (frecuencia=" << item.frecuencia << ")";
            }
            std::cout << "\n";
        }
        if (resultados.empty()) std::cout << "(sin resultados)\n";
        return 0;
    }

    if (!corrector.agregarPersonal(texto)) {
        std::cerr << "No se pudo agregar la palabra: " << texto << "\n";
        return 1;
    }
    std::cout << "Agregada al diccionario personal: " << texto << "\n";
    return 0;
}
