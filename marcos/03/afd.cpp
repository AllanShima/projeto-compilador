#include "afd.h"

namespace komboscript {

AfdCombo::AfdCombo(std::vector<Simbolo> alfabeto, const std::size_t quantidadeDeEstados,
                    const Estado inicial)
    : alfabeto_(std::move(alfabeto)),
      aceitacao_(quantidadeDeEstados + 1, 0),
      nomes_(quantidadeDeEstados + 1),
      inicial_(inicial),
      erro_(quantidadeDeEstados) {
    for (std::size_t coluna = 0; coluna < alfabeto_.size(); ++coluna) {
        colunaDoSimbolo_[alfabeto_[coluna]] = coluna;
    }

    // Toda posição nasce apontando para o erro: quem monta a máquina declara
    // só as transições que existem, e a função já sai total.
    tabela_.assign((quantidadeDeEstados + 1) * alfabeto_.size(), erro_);

    for (std::size_t estado = 0; estado <= quantidadeDeEstados; ++estado) {
        nomes_[estado] = "q" + std::to_string(estado);
    }
    nomes_[erro_] = "erro";
}

AfdCombo::AfdCombo() : aceitacao_(1, 0), nomes_(1, "erro"), inicial_(0), erro_(0) {}

std::size_t AfdCombo::colunaDe(const Simbolo& simbolo) const {
    const auto it = colunaDoSimbolo_.find(simbolo);
    if (it == colunaDoSimbolo_.end()) {
        return kSemColuna;
    }
    return it->second;
}

void AfdCombo::definirTransicao(const Estado origem, const Simbolo& simbolo, const Estado destino) {
    const std::size_t coluna = colunaDe(simbolo);
    if (coluna == kSemColuna || origem >= aceitacao_.size()) {
        // Símbolo fora do vocabulário do jogo ou estado inexistente: retorna
        // sem escrever, e o par continua indo para o erro.
        return;
    }
    tabela_[origem * alfabeto_.size() + coluna] = destino;
}

void AfdCombo::marcarAceitacao(const Estado estado) {
    if (estado < aceitacao_.size()) {
        aceitacao_[estado] = 1;
    }
}

void AfdCombo::nomearEstado(const Estado estado, std::string nome) {
    if (estado < nomes_.size()) {
        nomes_[estado] = std::move(nome);
    }
}

Estado AfdCombo::estadoDeErro() const { return erro_; }

Estado AfdCombo::estadoInicial() const { return inicial_; }

const std::vector<Simbolo>& AfdCombo::alfabeto() const { return alfabeto_; }

Estado AfdCombo::transicao(const Estado origem, const Simbolo& simbolo) const {
    const std::size_t coluna = colunaDe(simbolo);
    if (coluna == kSemColuna) {
        return erro_;
    }
    return tabela_[origem * alfabeto_.size() + coluna];
}

bool AfdCombo::ehDeAceitacao(const Estado estado) const { return aceitacao_[estado] != 0; }

const std::string& AfdCombo::nomeDoEstado(const Estado estado) const { return nomes_[estado]; }

bool AfdCombo::aceita(const std::vector<Simbolo>& sequencia) const {
    Estado atual = inicial_;
    for (const Simbolo& simbolo : sequencia) {
        atual = transicao(atual, simbolo);
    }
    return ehDeAceitacao(atual);
}

Execucao AfdCombo::executar(const std::vector<Simbolo>& sequencia) const {
    Execucao execucao;
    Estado atual = inicial_;
    execucao.passos.push_back(Configuracao{atual, 0, ""});

    for (std::size_t i = 0; i < sequencia.size(); ++i) {
        const Simbolo& simbolo = sequencia[i];
        const bool foraDoAlfabeto = colunaDe(simbolo) == kSemColuna;
        const Estado proximo = transicao(atual, simbolo);
        execucao.passos.push_back(Configuracao{proximo, i + 1, simbolo});

        // Registra só a primeira queda e segue lendo. Parar daria a mesma
        // resposta, mas o traço terminaria antes da sequência e não
        // mostraria o erro absorvendo o resto dela.
        if (proximo == erro_ && execucao.posicaoDaQueda == static_cast<std::size_t>(-1)) {
            execucao.posicaoDaQueda = i;
            execucao.simboloForaDoAlfabeto = foraDoAlfabeto;
        }
        atual = proximo;
    }

    execucao.aceitou = ehDeAceitacao(atual);
    return execucao;
}

std::size_t AfdCombo::bytesDaTabela() const { return tabela_.size() * sizeof(Estado); }

std::size_t AfdCombo::transicoesDefinidas() const {
    std::size_t total = 0;
    for (const Estado destino : tabela_) {
        if (destino != erro_) {
            ++total;
        }
    }
    return total;
}

std::size_t AfdCombo::quantidadeDeEstados() const { return aceitacao_.size(); }

std::size_t AfdCombo::tamanhoDoAlfabeto() const { return alfabeto_.size(); }

std::string AfdCombo::formatarTabela() const {
    std::string texto = "ESTADO         ACEITA  TRANSICOES\n";
    for (std::size_t estado = 0; estado < aceitacao_.size(); ++estado) {
        std::string linha = nomes_[estado];
        linha.resize(std::max(linha.size(), std::size_t{18}), ' ');
        linha += (aceitacao_[estado] != 0) ? "sim   " : "nao   ";

        bool primeiro = true;
        for (std::size_t coluna = 0; coluna < alfabeto_.size(); ++coluna) {
            const Estado destino = tabela_[estado * alfabeto_.size() + coluna];
            if (destino == erro_) {
                continue;
            }
            if (!primeiro) {
                linha += ", ";
            }
            linha += alfabeto_[coluna] + " -> " + nomes_[destino];
            primeiro = false;
        }
        if (primeiro) {
            linha += "(todas para erro)";
        }
        texto += linha + "\n";
    }
    return texto;
}

std::string AfdCombo::formatarExecucao(const std::vector<Simbolo>& sequencia,
                                        const Execucao& execucao) const {
    std::string texto = "  sequencia: ";
    for (std::size_t i = 0; i < sequencia.size(); ++i) {
        if (i > 0) texto += ", ";
        texto += sequencia[i];
    }
    texto += "\n  configuracoes: ";
    for (std::size_t i = 0; i < execucao.passos.size(); ++i) {
        const Configuracao& passo = execucao.passos[i];
        if (i > 0) {
            texto += " -" + passo.simboloLido + "-> ";
        }
        texto += nomes_[passo.estado];
    }
    texto += "\n  resultado: ";
    texto += execucao.aceitou ? "ACEITA" : "RECUSA";
    if (!execucao.aceitou && execucao.posicaoDaQueda != static_cast<std::size_t>(-1)) {
        texto += " — caiu no erro na posicao " + std::to_string(execucao.posicaoDaQueda);
        texto += execucao.simboloForaDoAlfabeto
                     ? " (simbolo fora do vocabulario do jogo)"
                     : " (simbolo valido, transicao inexistente)";
    } else if (!execucao.aceitou) {
        texto += " — consumiu a sequencia inteira e parou em estado nao final";
    }
    texto += "\n";
    return texto;
}

// --- vocabulário e autômatos de exemplo ---------------------------------

std::vector<Simbolo> alfabetoDoJogo() {
    return {"Baixo",  "Cima",       "Frente",    "Tras",      "Neutro",
            "Jab",    "SocoForte",  "ChuteFraco", "ChuteForte"};
}

// Especificação: RolarFrente -> Jab, com RolarFrente = Baixo -> Frente.
// Estados: nada lido ainda; viu o Baixo; viu Baixo e Frente em sequência;
// combo completo. `viu_baixo` reabre em si mesma com um novo Baixo (permite
// "segurar" o rolamento), e um Tras ali não tem transição declarada — cai no
// erro, que é o ponto do Slide 6.
AfdCombo montarHadouken() {
    AfdCombo afd(alfabetoDoJogo(), 4, 0);
    afd.nomearEstado(0, "inicio");
    afd.nomearEstado(1, "viu_baixo");
    afd.nomearEstado(2, "viu_baixo_frente");
    afd.nomearEstado(3, "completo");

    afd.definirTransicao(0, "Baixo", 1);
    afd.definirTransicao(1, "Baixo", 1);
    afd.definirTransicao(1, "Frente", 2);
    afd.definirTransicao(2, "Baixo", 1);
    afd.definirTransicao(2, "Jab", 3);

    afd.marcarAceitacao(3);
    return afd;
}

}  // namespace komboscript
