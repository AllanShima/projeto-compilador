#include "comparacao.h"

namespace komboscript {

namespace {

constexpr std::size_t kMaximoDeDivergenciasGuardadas = 10;

std::string juntar(const std::vector<Simbolo>& cadeia) {
    if (cadeia.empty()) {
        return "(vazia)";
    }
    std::string texto;
    for (std::size_t i = 0; i < cadeia.size(); ++i) {
        if (i > 0) {
            texto += ' ';
        }
        texto += cadeia[i];
    }
    return texto;
}

void conferir(const std::vector<Simbolo>& cadeia, const AfnCombo& afn, const AfdCombo& afd,
              const AfdCombo& minimo, ComparacaoDasTresMaquinas& comparacao) {
    const bool peloAfn = afn.aceita(cadeia);
    const bool peloAfd = afd.aceita(cadeia);
    const bool peloMinimo = minimo.aceita(cadeia);
    ++comparacao.cadeias;
    comparacao.aceitasPeloAfn += peloAfn;
    comparacao.aceitasPeloAfd += peloAfd;
    comparacao.aceitasPeloMinimo += peloMinimo;
    if ((peloAfn != peloAfd || peloAfd != peloMinimo) &&
        comparacao.divergencias.size() < kMaximoDeDivergenciasGuardadas) {
        comparacao.divergencias.push_back("\"" + juntar(cadeia) + "\": AFN=" + (peloAfn ? "aceita" : "recusa") +
                                          " AFD=" + (peloAfd ? "aceita" : "recusa") +
                                          " minimo=" + (peloMinimo ? "aceita" : "recusa"));
    }
}

}  // namespace

ComparacaoDasTresMaquinas compararMaquinas(const AfnCombo& afn, const AfdCombo& afd,
                                           const AfdCombo& minimo,
                                           const std::vector<Simbolo>& alfabeto,
                                           const std::size_t comprimentoMaximo) {
    ComparacaoDasTresMaquinas comparacao;

    // Por comprimento: cada nível é o anterior estendido por um símbolo.
    std::vector<std::vector<Simbolo>> nivel = {{}};
    for (std::size_t comprimento = 0;; ++comprimento) {
        for (const std::vector<Simbolo>& cadeia : nivel) {
            conferir(cadeia, afn, afd, minimo, comparacao);
        }
        if (comprimento == comprimentoMaximo) {
            break;
        }
        std::vector<std::vector<Simbolo>> proximo;
        proximo.reserve(nivel.size() * alfabeto.size());
        for (const std::vector<Simbolo>& cadeia : nivel) {
            for (const Simbolo& simbolo : alfabeto) {
                std::vector<Simbolo> estendida = cadeia;
                estendida.push_back(simbolo);
                proximo.push_back(std::move(estendida));
            }
        }
        nivel = std::move(proximo);
    }
    return comparacao;
}

}  // namespace komboscript
