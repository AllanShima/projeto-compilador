// AFN do KomboScript: estrutura, fecho vazio, movimento e simulação.
// A construção de Thompson está em thompson.h.

#ifndef KOMBOSCRIPT_KOMBO_AFN_H
#define KOMBOSCRIPT_KOMBO_AFN_H

#include <cstddef>
#include <string>
#include <vector>

#include "automato.h"

namespace komboscript {

// Estados ativos antes do primeiro input (simboloLido == kEpsilon) ou após um input.
struct PassoDaSimulacaoAfn {
    Simbolo simboloLido;
    std::vector<Estado> ativos;
};

struct SimulacaoAfn {
    bool aceitou = false;
    std::vector<PassoDaSimulacaoAfn> passos;
    std::size_t maiorConjuntoAtivo = 0;
};

class AfnCombo {
public:
    AfnCombo(std::vector<Simbolo> alfabeto);
    AfnCombo();

    Estado novoEstado();
    void adicionarTransicao(Estado origem, const Simbolo& simbolo, Estado destino);

    void definirInicial(Estado estado);
    void definirAceitacao(Estado estado);

    std::size_t quantidadeDeEstados() const;
    std::size_t quantidadeDeTransicoes() const;
    std::size_t quantidadeDeTransicoesVazias() const;

    // Estados alcançáveis sem consumir input.
    std::vector<Estado> fechoVazio(const std::vector<Estado>& conjunto) const;

    // Estados alcançados lendo `simbolo` a partir de qualquer estado do
    // conjunto, SEM fecho vazio (quem chama aplica o fecho). Ordenado e sem
    // repetição. Uma transição `kQualquer` casa com qualquer símbolo.
    std::vector<Estado> mover(const std::vector<Estado>& conjunto, const Simbolo& simbolo) const;

    Estado estadoInicial() const;
    Estado estadoDeAceitacao() const;

    // Símbolos que rotulam alguma transição (sem a transição vazia e sem
    // `kQualquer`), ordenados. É o alfabeto mínimo para determinizar.
    std::vector<Simbolo> simbolosUsados() const;

    bool aceita(const std::vector<Simbolo>& sequencia) const;
    SimulacaoAfn simular(const std::vector<Simbolo>& sequencia) const;

    std::string formatarSimulacao(const std::vector<Simbolo>& sequencia,
                                  const SimulacaoAfn& simulacao) const;

private:
    std::vector<Simbolo> alfabeto_;
    std::vector<std::vector<Transicao>> transicoes_;
    Estado inicial_ = 0;
    Estado aceitacao_ = 0;
};

}  // namespace komboscript

#endif  // KOMBOSCRIPT_KOMBO_AFN_H
