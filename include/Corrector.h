
#pragma once

#include <string>
#include <vector>

// Coordinador provisional: hasta integrar los otros modulos,
// gestiona exclusivamente el diccionario personal.
class Corrector {
public:
    enum class Resultado {
        Exito,
        YaExiste,
        NoEncontrada,
        EntradaInvalida,
        ErrorArchivo
    };

    explicit Corrector(
        const std::string& rutaPersonal =
            "data/diccionario_personal.txt"
    );

    bool cargarDiccionarioPersonal();

    Resultado agregarPalabraPersonal(
        const std::string& palabra
    );

    Resultado eliminarPalabraPersonal(
        const std::string& palabra
    );

    bool existePalabraPersonal(
        const std::string& palabra
    ) const;

    const std::vector<std::string>&
    listarPalabrasPersonales() const;

private:
    std::string rutaPersonal_;
    std::vector<std::string> palabrasPersonales_;

    static std::string normalizarPersonal(
        const std::string& palabra
    );

    static bool palabraValida(
        const std::string& palabra
    );

    bool guardar(
        const std::vector<std::string>& palabras
    ) const;
};
