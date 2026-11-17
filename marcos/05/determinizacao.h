// determinizacao.h — construção de subconjuntos: AFN -> AFD.
//
// Cada estado do AFD representa um conjunto de estados do AFN. O conjunto é
// guardado (`conjuntos`), porque é ele que explica o que o estado "lembra" e
// que permite conferir a máquina contra o AFN.
//
// O conjunto vazio é o estado de erro: quando nenhum estado ativo tem
// transição para o símbolo lido, o destino é o vazio. No AfdCombo do projeto
// o erro já existe como estado absorvente (índice = quantidade de estados
// úteis), então o vazio vira esse estado e a função de transição sai total.
// Os `conjuntos` listam só os estados úteis; `vazioAlcancavel` diz se o erro
// recebe alguma transição.

#ifndef KOMBOSCRIPT_DETERMINIZACAO_H
#define KOMBOSCRIPT_DETERMINIZACAO_H

#include <cstddef>
#include <string>
#include <vector>

#include "afd.h"
#include "afn.h"

namespace komboscript {

struct ResultadoDaDeterminizacao {
    AfdCombo afd;
    // conjuntos[i] = estados do AFN (ordenados) que o estado i do AFD representa.
    std::vector<std::vector<Estado>> conjuntos;
    bool vazioAlcancavel = false;

    // Estados do AFD no sentido do guia: conjuntos alcançáveis, contando o
    // vazio (erro) apenas quando ele é de fato alcançado.
    std::size_t quantidadeDeEstados() const;
};

// `alfabeto` é o vocabulário sobre o qual o AFD é total. Símbolos que o AFN
// não usa levam ao erro.
ResultadoDaDeterminizacao determinizar(const AfnCombo& afn, const std::vector<Simbolo>& alfabeto);

// Atalho: usa os símbolos que aparecem no AFN como alfabeto.
ResultadoDaDeterminizacao determinizar(const AfnCombo& afn);

// "{0, 2, 5}" — para exibição.
std::string formatarConjunto(const std::vector<Estado>& conjunto);

}  // namespace komboscript

#endif  // KOMBOSCRIPT_DETERMINIZACAO_H
