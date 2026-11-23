CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude

COMUNES = src/Normalizador.cpp src/Levenshtein.cpp src/DiccionarioHash.cpp src/Trie.cpp src/Corrector.cpp
PRUEBAS = pruebas_hash.exe pruebas_levenshtein.exe pruebas_normalizador.exe pruebas_trie.exe pruebas_integradas.exe

.PHONY: all test clean

all: corrector.exe

corrector.exe: $(COMUNES) src/main.cpp
	$(CXX) $(CXXFLAGS) $(COMUNES) src/main.cpp -o corrector.exe

pruebas_hash.exe: src/DiccionarioHash.cpp tests/pruebas_hash.cpp
	$(CXX) $(CXXFLAGS) src/DiccionarioHash.cpp tests/pruebas_hash.cpp -o pruebas_hash.exe

pruebas_levenshtein.exe: src/Levenshtein.cpp src/Normalizador.cpp tests/pruebas_levenshtein.cpp
	$(CXX) $(CXXFLAGS) src/Levenshtein.cpp src/Normalizador.cpp tests/pruebas_levenshtein.cpp -o pruebas_levenshtein.exe

pruebas_normalizador.exe: src/Normalizador.cpp tests/pruebas_normalizador.cpp
	$(CXX) $(CXXFLAGS) src/Normalizador.cpp tests/pruebas_normalizador.cpp -o pruebas_normalizador.exe

pruebas_trie.exe: src/Trie.cpp src/Normalizador.cpp tests/pruebas_trie.cpp
	$(CXX) $(CXXFLAGS) src/Trie.cpp src/Normalizador.cpp tests/pruebas_trie.cpp -o pruebas_trie.exe

pruebas_integradas.exe: $(COMUNES) tests/pruebas_integradas.cpp
	$(CXX) $(CXXFLAGS) $(COMUNES) tests/pruebas_integradas.cpp -o pruebas_integradas.exe

test: $(PRUEBAS)
	./pruebas_hash.exe
	./pruebas_levenshtein.exe
	./pruebas_normalizador.exe
	./pruebas_trie.exe
	./pruebas_integradas.exe

clean:
	rm -f $(PRUEBAS) corrector.exe
