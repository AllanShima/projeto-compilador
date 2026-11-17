// minimizacao.h — AFD -> AFD mínimo por refinamento de partições.
//
// Passos: (1) completar, (2) descartar inacessíveis, (3) partição inicial
// {aceitação, não aceitação}, (4) refinar até o ponto fixo, (5) reconstruir.
//
// (1) já vale por construção: o AfdCombo tem função de transição total, com
// erro absorvente (ver docs/representacao_transicao.md). Um AFD vindo de fora
// do projeto, incompleto, teria de ser completado aqui antes do passo 2.
//
// A classe que contém o erro original (o "poço") vira o estado de erro da
// máquina mínima. Estados não aceitos de que nunca se alcança uma aceitação
// são equivalentes ao erro e caem nessa mesma classe.

#ifndef KOMBOSCRIPT_MINIMIZACAO_H
#define KOMBOSCRIPT_MINIMIZACAO_H

#include <cstddef>
#include <vector>

#include "afd.h"

namespace komboscript {

struct ResultadoDaMinimizacao {
    AfdCombo afd;  // o mínimo; o estado de erro do AfdCombo é o poço

    // Contagens no sentido do guia: máquina completa, poço incluído quando
    // alcançável. `estadosAntes` conta tudo o que o AFD de entrada tem.
    std::size_t estadosAntes = 0;
    std::size_t estadosAlcancaveis = 0;
    std::size_t estadosDepois = 0;
    std::size_t rodadas = 0;  // rodadas de refinamento até o ponto fixo

    // classeDe[s] = índice, no mínimo, do estado s do AFD de entrada;
    // kInacessivel para os descartados.
    std::vector<std::size_t> classeDe;
    static constexpr std::size_t kInacessivel = static_cast<std::size_t>(-1);
};

ResultadoDaMinimizacao minimizar(const AfdCombo& afd);

}  // namespace komboscript

#endif  // KOMBOSCRIPT_MINIMIZACAO_H
