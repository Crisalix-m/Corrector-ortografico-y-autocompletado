#ifndef NORMALIZADOR_H
#define NORMALIZADOR_H

#include <cstddef>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Normalizador: prepara texto en espanol para las demas estructuras.
//
// Forma normalizada de una palabra:
//   - solo letras a-z y la letra n con tilde (ñ, UTF-8: 0xC3 0xB1)
//   - todo en minusculas
//   - sin tildes ni dieresis (a, e, i, o, u; u con dieresis -> u)
//
// Entrada admitida: UTF-8 (y, por tolerancia, bytes sueltos Latin-1).
// ---------------------------------------------------------------------------

// Una palabra encontrada dentro de un texto.
struct Token {
    std::string original;     // tal como aparece en el texto
    std::string normalizada;  // forma normalizada (lista para hash / trie)
    std::size_t posicion;     // byte del texto donde empieza la palabra
};

class Normalizador {
public:
    // Cantidad de letras del alfabeto de trabajo: a-z + ñ (hijos del trie).
    static constexpr int TOTAL_LETRAS = 27;

    // normalizar(texto)
    //   Dominio:       cadena UTF-8 cualquiera.
    //   Precondicion:  ninguna.
    //   Poscondicion:  devuelve el texto con las letras en minuscula y sin
    //                  tildes; la ñ se conserva; los demas simbolos
    //                  (espacios, puntuacion, numeros) quedan igual.
    static std::string normalizar(const std::string& texto);

    // separarPalabras(texto)
    //   Poscondicion:  devuelve las palabras del texto, en orden, ya
    //                  normalizadas. Todo lo que no sea letra separa palabras.
    static std::vector<std::string> separarPalabras(const std::string& texto);

    // tokenizar(texto)
    //   Igual que separarPalabras, pero conserva la palabra original y su
    //   posicion para poder marcar las palabras desconocidas en el texto.
    static std::vector<Token> tokenizar(const std::string& texto);

    // esPalabraValida(palabra)
    //   Poscondicion:  true si la cadena no es vacia y esta formada solo por
    //                  a-z y ñ (es decir, ya esta normalizada).
    //                  Precondicion de buscar/insertar en hash y trie.
    static bool esPalabraValida(const std::string& palabra);

    // indiceLetra(c)
    //   'a'..'z' -> 0..25,  ñ -> 26,  cualquier otro -> -1.
    static int indiceLetra(char32_t c);

    // aIndices(palabra)
    //   Precondicion:  esPalabraValida(palabra).
    //   Poscondicion:  una posicion por letra con su indice (0..26).
    //                  Util para recorrer el trie sin tratar bytes UTF-8.
    static std::vector<int> aIndices(const std::string& palabra);

    // Conversion UTF-8 <-> puntos de codigo (una posicion por caracter).
    static std::vector<char32_t> decodificarUTF8(const std::string& texto);
    static std::string codificarUTF8(const std::vector<char32_t>& puntos);
};

#endif
