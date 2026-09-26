#ifndef KOMBOSCRIPT_KOMBO_AFD_H
#define KOMBOSCRIPT_KOMBO_AFD_H

#include <algorithm>
#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

namespace komboscript {

using Estado = std::size_t;
using Simbolo = std::string;

// Uma configuração é o par (estado corrente, posição na sequência de inputs
// do jogador). A execução guarda a sequência delas.
struct Configuracao {
    Estado estado = 0;
    std::size_t posicao = 0;
    Simbolo simboloLido;  // vazio ("") na configuração inicial
};

struct Execucao {
    bool aceitou = false;
    std::vector<Configuracao> passos;
    // Posição do primeiro input que levou ao erro; kSemQueda quando a
    // execução nunca caiu nele.
    std::size_t posicaoDaQueda = static_cast<std::size_t>(-1);
    bool simboloForaDoAlfabeto = false;
};

class AfdCombo {
public:
    // `alfabeto` é o vocabulário de símbolos primitivos do jogo (movimentos e
    // ataques), o mesmo para todo combo/macro compilado — não um recorte
    // específico deste autômato. `quantidadeDeEstados` conta só os estados
    // úteis; o de erro recebe o índice seguinte. Toda posição da tabela nasce
    // apontando para ele, e definir uma transição é sobrescrever uma dessas
    // posições.
    AfdCombo(std::vector<Simbolo> alfabeto, std::size_t quantidadeDeEstados, Estado inicial);

    // Autômato degenerado: só o estado de erro, alfabeto vazio, recusa tudo.
    AfdCombo();

    void definirTransicao(Estado origem, const Simbolo& simbolo, Estado destino);
    void marcarAceitacao(Estado estado);
    void nomearEstado(Estado estado, std::string nome);

    Estado estadoDeErro() const;
    Estado estadoInicial() const;
    const std::vector<Simbolo>& alfabeto() const;
    Estado transicao(Estado origem, const Simbolo& simbolo) const;
    bool ehDeAceitacao(Estado estado) const;
    const std::string& nomeDoEstado(Estado estado) const;

    // Consome a sequência inteira e diz se parou em estado de aceitação.
    bool aceita(const std::vector<Simbolo>& sequencia) const;

    // O mesmo reconhecimento, guardando a configuração alcançada a cada
    // input — usado para o traço de execução (Slide 6).
    Execucao executar(const std::vector<Simbolo>& sequencia) const;

    // (estados + 1) x |alfabeto| x sizeof(Estado).
    std::size_t bytesDaTabela() const;
    // Posições cujo destino não é o erro.
    std::size_t transicoesDefinidas() const;
    std::size_t quantidadeDeEstados() const;
    std::size_t tamanhoDoAlfabeto() const;

    std::string formatarTabela() const;
    std::string formatarExecucao(const std::vector<Simbolo>& sequencia, const Execucao& execucao) const;

private:
    std::size_t colunaDe(const Simbolo& simbolo) const;
    static constexpr std::size_t kSemColuna = static_cast<std::size_t>(-1);

    std::vector<Simbolo> alfabeto_;
    std::unordered_map<Simbolo, std::size_t> colunaDoSimbolo_;  // identificador -> coluna
    std::vector<Estado> tabela_;                                // (estados+1) x |alfabeto|
    // `char`, e não `bool`: vector<bool> empacota bits e não devolve
    // referência de verdade (mesmo motivo do sistema Peneira).
    std::vector<char> aceitacao_;
    std::vector<std::string> nomes_;
    Estado inicial_ = 0;
    Estado erro_ = 0;
};

// O vocabulário do jogo usado nos exemplos deste módulo. Compartilhado por
// todos os autômatos, porque a VM roda um autômato por combo em paralelo
// sobre o mesmo fluxo de inputs.
std::vector<Simbolo> alfabetoDoJogo();

// O combo Hadouken, construído à mão a partir da especificação já reduzida:
// RolarFrente -> Jab, com RolarFrente = Baixo -> Frente.
AfdCombo montarHadouken();

}  // namespace komboscript

#endif  // KOMBOSCRIPT_KOMBO_AFD_H
