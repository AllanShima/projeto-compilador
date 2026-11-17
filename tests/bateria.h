// bateria.h — a bateria de casos conferidos à mão (tests/casos.txt).
//
// Formato da linha: expressao <TAB> inputs separados por espaço <TAB> aceita|recusa
// Cada caso é o padrão, a cadeia e a resposta que o grupo calculou no papel.

#ifndef KOMBOSCRIPT_TESTES_BATERIA_H
#define KOMBOSCRIPT_TESTES_BATERIA_H

#include <cstddef>
#include <string>
#include <vector>

#include "automato.h"

namespace komboscript {

struct CasoDeTeste {
    std::string expressao;
    std::vector<Simbolo> sequencia;
    bool esperado = false;
    std::string origem;         // arquivo:linha
    std::string erroDeFormato;  // preenchido se a linha for inválida
};

struct ResultadoDaBateria {
    std::size_t total = 0;
    std::size_t passaram = 0;
    std::vector<std::string> falhas;
};

// Devolve lista vazia se o arquivo não abre.
std::vector<CasoDeTeste> lerCasos(const std::string& caminho);

// Roda todos os casos só pelo AFN, sem parar na primeira falha.
ResultadoDaBateria rodarBateria(const std::vector<CasoDeTeste>& casos);

}  // namespace komboscript

#endif  // KOMBOSCRIPT_TESTES_BATERIA_H
