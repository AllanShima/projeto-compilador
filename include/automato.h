// automato.h — o que AFN e AFD têm em comum: estado, símbolo, transição e as
// marcas reservadas.

#ifndef KOMBOSCRIPT_AUTOMATO_H
#define KOMBOSCRIPT_AUTOMATO_H

#include <cstddef>
#include <string>

namespace komboscript {

using Estado = std::size_t;

// Um símbolo é um identificador do alfabeto do jogo (Baixo, Frente, Jab, ...),
// não um caractere solto.
using Simbolo = std::string;

// Marcas reservadas.
//
// Transição vazia: não consome input. Como o símbolo vazio não pertence ao
// alfabeto, o texto vazio serve de marca sem colidir com nenhum movimento.
inline const Simbolo kEpsilon = "";

// Reservado para coringa. A linguagem não tem coringa; a construção de
// Thompson nunca o usa, mas a simulação e o movimento o tratam.
inline const Simbolo kQualquer = "*";

// Aresta de um AFN: lê `simbolo` e vai para `destino`.
struct Transicao {
    Simbolo simbolo = kEpsilon;
    Estado destino = 0;
};

}  // namespace komboscript

#endif  // KOMBOSCRIPT_AUTOMATO_H
