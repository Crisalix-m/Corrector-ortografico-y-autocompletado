
#include "Corrector.h"

#include <iostream>
#include <string>

void mostrarResultado(Corrector::Resultado resultado) {
    switch (resultado) {
        case Corrector::Resultado::Exito:
            std::cout
                << "Operacion realizada correctamente.\n";
            break;

        case Corrector::Resultado::YaExiste:
            std::cout
                << "La palabra ya existe en el diccionario personal.\n";
            break;

        case Corrector::Resultado::NoEncontrada:
            std::cout
                << "La palabra no esta en el diccionario personal.\n";
            break;

        case Corrector::Resultado::EntradaInvalida:
            std::cout
                << "Ingrese solo una palabra con letras del espanol.\n";
            break;

        case Corrector::Resultado::ErrorArchivo:
            std::cout
                << "Error al guardar el diccionario personal.\n";
            break;
    }
}

void administrarDiccionarioPersonal(
    Corrector& corrector
) {
    std::string opcion;
    std::string palabra;

    while (true) {
        std::cout
            << "\n--- DICCIONARIO PERSONAL ---\n"
            << "1. Mostrar palabras\n"
            << "2. Agregar palabra\n"
            << "3. Eliminar palabra\n"
            << "4. Volver al menu principal\n"
            << "Opcion: ";

        if (!std::getline(std::cin, opcion)) {
            return;
        }

        if (opcion == "1") {
            const auto& palabras =
                corrector.listarPalabrasPersonales();

            if (palabras.empty()) {
                std::cout
                    << "El diccionario personal esta vacio.\n";
            } else {
                for (const auto& p : palabras) {
                    std::cout << "- " << p << '\n';
                }
            }

        } else if (opcion == "2" || opcion == "3") {
            std::cout << "Palabra: ";

            if (!std::getline(std::cin, palabra)) {
                return;
            }

            if (opcion == "2") {
                mostrarResultado(
                    corrector.agregarPalabraPersonal(palabra)
                );
            } else {
                mostrarResultado(
                    corrector.eliminarPalabraPersonal(palabra)
                );
            }

        } else if (opcion == "4") {
            return;

        } else {
            std::cout << "Opcion no valida.\n";
        }
    }
}

int main() {
    Corrector corrector(
        "data/diccionario_personal.txt"
    );

    if (!corrector.cargarDiccionarioPersonal()) {
        std::cerr
            << "Aviso: no se pudo cargar "
            << "data/diccionario_personal.txt.\n"
            << "Comprueba que ejecutas el programa "
            << "desde la raiz del proyecto.\n";
    }

    std::string opcion;

    while (true) {
        std::cout
            << "\n====== CORRECTOR ORTOGRAFICO ======\n"
            << "1. Cargar diccionario principal\n"
            << "2. Verificar un texto\n"
            << "3. Sugerir correcciones\n"
            << "4. Autocompletar palabra\n"
            << "5. Diccionario personal\n"
            << "6. Salir\n"
            << "Opcion: ";

        if (!std::getline(std::cin, opcion)) {
            break;
        }

        if (opcion == "1" || opcion == "2" ||
            opcion == "3" || opcion == "4") {

            std::cout
                << "Pendiente: falta integrar los "
                << "modulos de tus companeros.\n";

        } else if (opcion == "5") {
            administrarDiccionarioPersonal(corrector);

        } else if (opcion == "6") {
            std::cout
                << "Saliendo del programa.\n";
            break;

        } else {
            std::cout << "Opcion no valida.\n";
        }
    }

    return 0;
}
