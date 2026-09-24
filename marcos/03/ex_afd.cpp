#include "03_afd.h"

namespace peneira {

namespace {

std::string preencher(const std::string& texto, const std::size_t largura) {
    std::string resultado = texto;
    while (resultado.size() < largura) {
        resultado += ' ';
    }
    return resultado;
}

// Os símbolos de `inicio` a `fim`, pelo código do caractere; monta os alfabetos
// das três máquinas.
std::string faixa(const char inicio, const char fim) {
    std::string simbolos;
    for (int codigo = static_cast<unsigned char>(inicio); codigo <= static_cast<unsigned char>(fim);
         ++codigo) {
        simbolos += static_cast<char>(codigo);
    }
    return simbolos;
}

}  // namespace

Afd::Afd(std::string alfabeto, const std::size_t quantidadeDeEstados, const Estado inicial)
    : alfabeto_(std::move(alfabeto)),
      colunaDoByte_(256, kSemColuna),
      aceitacao_(quantidadeDeEstados + 1, 0),
      nomes_(quantidadeDeEstados + 1),
      inicial_(inicial),
      erro_(quantidadeDeEstados) {
// recorte:inicio coluna-nao-e-o-byte
    for (std::size_t coluna = 0; coluna < alfabeto_.size(); ++coluna) {
        const std::size_t byte = static_cast<unsigned char>(alfabeto_[coluna]);
        colunaDoByte_[byte] = coluna;
    }
    // recorte:fim coluna-nao-e-o-byte

// recorte:inicio transicao-total-por-construcao
    // Toda posição nasce apontando para o erro: quem monta a máquina declara só
    // as transições que existem, e a função já sai total.
    tabela_.assign((quantidadeDeEstados + 1) * alfabeto_.size(), erro_);
    // recorte:fim transicao-total-por-construcao

    for (std::size_t estado = 0; estado <= quantidadeDeEstados; ++estado) {
        nomes_[estado] = "q" + std::to_string(estado);
    }
    nomes_[erro_] = "erro";
}

Afd::Afd()
    : colunaDoByte_(256, kSemColuna), aceitacao_(1, 0), nomes_(1, "erro"), inicial_(0), erro_(0) {}

std::size_t Afd::colunaDe(const char simbolo) const {
    return colunaDoByte_[static_cast<unsigned char>(simbolo)];
}

void Afd::definirTransicao(const Estado origem, const char simbolo, const Estado destino) {
    const std::size_t coluna = colunaDe(simbolo);
    if (coluna == kSemColuna || origem >= aceitacao_.size()) {
        // Símbolo fora do alfabeto ou estado inexistente: retorna sem escrever, e
        // o par continua indo para o erro. Lançar pegaria o erro de digitação na
        // hora, mas obrigaria cada chamada de construção a tratar a falha.
        return;
    }
    tabela_[origem * alfabeto_.size() + coluna] = destino;
}

void Afd::definirTransicoes(const Estado origem, const std::string& simbolos,
                            const Estado destino) {
    for (const char simbolo : simbolos) {
        definirTransicao(origem, simbolo, destino);
    }
}

void Afd::marcarAceitacao(const Estado estado) {
    if (estado < aceitacao_.size()) {
        aceitacao_[estado] = 1;
    }
}

void Afd::nomearEstado(const Estado estado, std::string nome) {
    if (estado < nomes_.size()) {
        nomes_[estado] = std::move(nome);
    }
}

Estado Afd::estadoDeErro() const { return erro_; }

Estado Afd::estadoInicial() const { return inicial_; }

const std::string& Afd::alfabeto() const { return alfabeto_; }

// recorte:inicio tabela-densa-indexada
Estado Afd::transicao(const Estado origem, const char simbolo) const {
    const std::size_t coluna = colunaDe(simbolo);
    if (coluna == kSemColuna) {
        return erro_;
    }
    return tabela_[origem * alfabeto_.size() + coluna];
}
// recorte:fim tabela-densa-indexada

bool Afd::ehDeAceitacao(const Estado estado) const { return aceitacao_[estado] != 0; }

const std::string& Afd::nomeDoEstado(const Estado estado) const { return nomes_[estado]; }

// diagrama:adiado quem só precisa do sim ou não é a bateria do AFN (04), a equivalência da determinização (05) e o marco 06; o marco 03 percorre pela executar(), a mesma travessia com o traço registrado
// recorte:inicio aceita-em-quatro-linhas
bool Afd::aceita(const std::string& cadeia) const {
    Estado atual = inicial_;
    for (const char simbolo : cadeia) {
        atual = transicao(atual, simbolo);
    }
    return ehDeAceitacao(atual);
}
// recorte:fim aceita-em-quatro-linhas

// recorte:inicio queda-registrada-sem-parar
Execucao Afd::executar(const std::string& cadeia) const {
    Execucao execucao;
    Estado atual = inicial_;
    execucao.passos.push_back(Configuracao{atual, 0, '\0'});

    for (std::size_t i = 0; i < cadeia.size(); ++i) {
        const char simbolo = cadeia[i];
        const bool foraDoAlfabeto = colunaDe(simbolo) == kSemColuna;
        const Estado proximo = transicao(atual, simbolo);
        execucao.passos.push_back(Configuracao{proximo, i + 1, simbolo});

        // Registra só a primeira queda e segue lendo. Parar daria a mesma
        // resposta, mas o traço terminaria antes da cadeia e não mostraria o erro
        // absorvendo o resto dela.
        if (proximo == erro_ && execucao.posicaoDaQueda == std::string::npos) {
            execucao.posicaoDaQueda = i;
            execucao.simboloForaDoAlfabeto = foraDoAlfabeto;
        }
        atual = proximo;
    }

    execucao.aceitou = ehDeAceitacao(atual);
    return execucao;
}
// recorte:fim queda-registrada-sem-parar

// recorte:inicio bytes-da-tabela
std::size_t Afd::bytesDaTabela() const { return tabela_.size() * sizeof(Estado); }

std::size_t Afd::transicoesDefinidas() const {
    std::size_t total = 0;
    for (const Estado destino : tabela_) {
        if (destino != erro_) {
            ++total;
        }
    }
    return total;
}
// recorte:fim bytes-da-tabela

std::size_t Afd::quantidadeDeEstados() const { return aceitacao_.size(); }

std::size_t Afd::tamanhoDoAlfabeto() const { return alfabeto_.size(); }

std::string Afd::formatarTabela() const {
    // Por faixa de colunas com o mesmo destino, e não coluna a coluna: o
    // alfabeto do identificador daria trinta e sete colunas por linha. Cada
    // estado lista os destinos que não são o erro e os símbolos que levam a eles.
    std::string texto;
    texto += preencher("ESTADO", 14) + preencher("ACEITA", 8) + "TRANSICOES\n";
    for (std::size_t estado = 0; estado < aceitacao_.size(); ++estado) {
        texto += preencher(nomes_[estado], 14);
        texto += preencher(aceitacao_[estado] != 0 ? "sim" : "nao", 8);

        bool primeiro = true;
        for (std::size_t coluna = 0; coluna < alfabeto_.size(); ++coluna) {
            const Estado destino = tabela_[estado * alfabeto_.size() + coluna];
            if (destino == erro_) {
                continue;
            }
            // Estende a faixa enquanto a coluna seguinte levar ao mesmo destino:
            // os dez dígitos saem numa entrada só.
            std::size_t fimDaFaixa = coluna;
            while (fimDaFaixa + 1 < alfabeto_.size() &&
                   tabela_[estado * alfabeto_.size() + fimDaFaixa + 1] == destino) {
                ++fimDaFaixa;
            }
            if (!primeiro) {
                texto += ", ";
            }
            const std::size_t quantidade = fimDaFaixa - coluna + 1;
            if (quantidade <= 3) {
                // Até três símbolos, um a um: `+ -` escrito como faixa sugeriria
                // um intervalo de códigos que não existe.
                for (std::size_t i = coluna; i <= fimDaFaixa; ++i) {
                    if (i > coluna) {
                        texto += ' ';
                    }
                    texto += alfabeto_[i];
                }
            } else {
                // A faixa é por posição no alfabeto declarado, não por código do
                // caractere. A contagem entre colchetes impede que `a.._ [37]`
                // seja lido como intervalo ASCII.
                texto += alfabeto_[coluna];
                texto += "..";
                texto += alfabeto_[fimDaFaixa];
                texto += " [" + std::to_string(quantidade) + "]";
            }
            texto += " -> " + nomes_[destino];
            primeiro = false;
            coluna = fimDaFaixa;
        }
        if (primeiro) {
            texto += "(todas para erro)";
        }
        texto += '\n';
    }
    return texto;
}

std::string Afd::formatarExecucao(const std::string& cadeia, const Execucao& execucao) const {
    std::string texto = "  cadeia: \"" + cadeia + "\"\n";
    texto += "  configuracoes: ";
    for (std::size_t i = 0; i < execucao.passos.size(); ++i) {
        const Configuracao& passo = execucao.passos[i];
        if (i > 0) {
            texto += " -";
            texto += passo.simboloLido;
            texto += "-> ";
        }
        texto += nomes_[passo.estado];
    }
    texto += '\n';
    texto += std::string("  resultado: ") + (execucao.aceitou ? "ACEITA" : "RECUSA");
    if (!execucao.aceitou && execucao.posicaoDaQueda != std::string::npos) {
        texto += " — caiu no erro na posicao " + std::to_string(execucao.posicaoDaQueda);
        texto += execucao.simboloForaDoAlfabeto ? " (simbolo fora do alfabeto declarado)"
                                                : " (simbolo valido, transicao inexistente)";
    } else if (!execucao.aceitou) {
        texto += " — consumiu a cadeia inteira e parou em estado nao final";
    }
    texto += '\n';
    return texto;
}

// --- os três autômatos projetados à mão --------------------------------------

// Especificação: uma letra minúscula seguida de qualquer número de letras
// minúsculas, dígitos ou sublinhados. Dois estados: nada lido ainda, e a
// primeira letra já lida.
Afd afdIdentificador() {
    const std::string letras = faixa('a', 'z');
    const std::string digitos = faixa('0', '9');
    Afd afd(letras + digitos + "_", 2, 0);
    afd.nomearEstado(0, "inicio");
    afd.nomearEstado(1, "corpo");
    afd.definirTransicoes(0, letras, 1);
    afd.definirTransicoes(1, letras, 1);
    afd.definirTransicoes(1, digitos, 1);
    afd.definirTransicao(1, '_', 1);
    afd.marcarAceitacao(1);
    return afd;
}

// Especificação: sinal opcional, ao menos um dígito e, opcionalmente, um ponto
// seguido de ao menos um dígito. Cada estado é uma resposta diferente a "o que
// falta para a cadeia ser válida?". `apos_ponto` não aceita: um número não
// termina em ponto.
Afd afdNumeroComSinal() {
    const std::string digitos = faixa('0', '9');
    Afd afd(digitos + "+-.", 4, 0);
    afd.nomearEstado(0, "inicio");
    afd.nomearEstado(1, "inteiro");
    afd.nomearEstado(2, "apos_ponto");
    afd.nomearEstado(3, "fracao");
    afd.definirTransicao(0, '+', 0);
    afd.definirTransicao(0, '-', 0);
    afd.definirTransicoes(0, digitos, 1);
    afd.definirTransicoes(1, digitos, 1);
    afd.definirTransicao(1, '.', 2);
    afd.definirTransicoes(2, digitos, 3);
    afd.definirTransicoes(3, digitos, 3);
    afd.marcarAceitacao(1);
    afd.marcarAceitacao(3);
    return afd;
}

// Especificação: duas barras e qualquer coisa até o fim da linha. O alfabeto é
// estreito (barra, espaço e minúsculas) para que a demonstração possa recusar
// por símbolo fora dele, e não só por transição inexistente.
Afd afdComentarioDeLinha() {
    const std::string letras = faixa('a', 'z');
    Afd afd("/ " + letras, 3, 0);
    afd.nomearEstado(0, "inicio");
    afd.nomearEstado(1, "uma_barra");
    afd.nomearEstado(2, "no_comentario");
    afd.definirTransicao(0, '/', 1);
    afd.definirTransicao(1, '/', 2);
    afd.definirTransicao(2, '/', 2);
    afd.definirTransicao(2, ' ', 2);
    afd.definirTransicoes(2, letras, 2);
    afd.marcarAceitacao(2);
    return afd;
}

}  // namespace peneira