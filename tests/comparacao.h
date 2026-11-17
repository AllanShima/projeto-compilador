// comparacao.h — as três máquinas (AFN, AFD, AFD mínimo) sobre as mesmas cadeias.
//
// Comparar só as duas pontas (AFN x mínimo) esconde onde está o defeito: se
// discordam, não se sabe se errou a determinização ou a minimização. Com as
// três, a discordância aponta o passo: AFN x AFD discordando acusa a
// determinização; AFD x mínimo acusa a minimização.

#ifndef KOMBOSCRIPT_COMPARACAO_H
#define KOMBOSCRIPT_COMPARACAO_H

#include <cstddef>
#include <string>
#include <vector>

#include "afd.h"
#include "afn.h"

namespace komboscript {

struct ComparacaoDasTresMaquinas {
    std::size_t cadeias = 0;
    std::size_t aceitasPeloAfn = 0;
    std::size_t aceitasPeloAfd = 0;
    std::size_t aceitasPeloMinimo = 0;
    std::vector<std::string> divergencias;  // as primeiras, com quem discordou

    bool concordam() const { return divergencias.empty(); }
};

// Toda cadeia sobre `alfabeto` de comprimento 0 até `comprimentoMaximo`.
ComparacaoDasTresMaquinas compararMaquinas(const AfnCombo& afn, const AfdCombo& afd,
                                           const AfdCombo& minimo,
                                           const std::vector<Simbolo>& alfabeto,
                                           std::size_t comprimentoMaximo);

}  // namespace komboscript

#endif  // KOMBOSCRIPT_COMPARACAO_H
