#include "expressao.h"

#include <cctype>

namespace komboscript {

bool Arvore::vazia() const { return raiz == kSemFilho; }

std::size_t tamanho(const Arvore& arvore) { return arvore.nos.size(); }

namespace {

enum class TipoToken {
    Identificador,
    Seta,            // ->
    Barra,           // |
    Asterisco,       // *
    Mais,            // +
    Interrogacao,    // ?
    ColcheteAbre,    // [
    ColcheteFecha,   // ]
    Fim,
};

struct Token {
    TipoToken tipo = TipoToken::Fim;
    std::string texto;
    std::size_t posicao = 0;
};

struct ErroLexico {
    bool houve = false;
    std::size_t posicao = 0;
    std::string mensagem;
};

std::vector<Token> tokenizar(const std::string& texto, ErroLexico& erro) {
    std::vector<Token> tokens;
    std::size_t i = 0;
    const std::size_t n = texto.size();
    while (i < n) {
        const char c = texto[i];
        if (std::isspace(static_cast<unsigned char>(c))) {
            ++i;
            continue;
        }
        if (std::isalpha(static_cast<unsigned char>(c))) {
            const std::size_t inicio = i;
            std::string nome;
            while (i < n && std::isalnum(static_cast<unsigned char>(texto[i]))) {
                nome += texto[i];
                ++i;
            }
            tokens.push_back({TipoToken::Identificador, nome, inicio});
            continue;
        }
        if (c == '-' && i + 1 < n && texto[i + 1] == '>') {
            tokens.push_back({TipoToken::Seta, "->", i});
            i += 2;
            continue;
        }
        if (c == '|') {
            tokens.push_back({TipoToken::Barra, "|", i});
            ++i;
            continue;
        }
        if (c == '*') {
            tokens.push_back({TipoToken::Asterisco, "*", i});
            ++i;
            continue;
        }
        if (c == '+') {
            tokens.push_back({TipoToken::Mais, "+", i});
            ++i;
            continue;
        }
        if (c == '?') {
            tokens.push_back({TipoToken::Interrogacao, "?", i});
            ++i;
            continue;
        }
        if (c == '[') {
            tokens.push_back({TipoToken::ColcheteAbre, "[", i});
            ++i;
            continue;
        }
        if (c == ']') {
            tokens.push_back({TipoToken::ColcheteFecha, "]", i});
            ++i;
            continue;
        }
        erro.houve = true;
        erro.posicao = i;
        erro.mensagem = std::string("caractere inesperado '") + c + "' na expressao";
        return tokens;
    }
    tokens.push_back({TipoToken::Fim, "", n});
    return tokens;
}

class Analisador {
public:
    explicit Analisador(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

    Resultado analisar() {
        Resultado resultado;
        const std::size_t raiz = alternancia();
        if (falhou_) {
            resultado.ok = false;
            resultado.erro = erro_;
            return resultado;
        }
        if (atual().tipo != TipoToken::Fim) {
            return falhar(erroEm("simbolo inesperado apos o fim da expressao"), resultado);
        }
        resultado.ok = true;
        resultado.arvore.nos = nos_;
        resultado.arvore.raiz = raiz;
        return resultado;
    }

private:
    // --- construção de nós --------------------------------------------

    std::size_t novoIdentificador(const std::string& nome) {
        No no;
        no.tipo = TipoDeNo::Identificador;
        no.nome = nome;
        nos_.push_back(no);
        return nos_.size() - 1;
    }

    std::size_t novoVazio() {
        No no;
        no.tipo = TipoDeNo::Vazio;
        nos_.push_back(no);
        return nos_.size() - 1;
    }

    std::size_t novoBinario(const TipoDeNo tipo, const std::size_t esquerda,
                             const std::size_t direita) {
        No no;
        no.tipo = tipo;
        no.esquerda = esquerda;
        no.direita = direita;
        nos_.push_back(no);
        return nos_.size() - 1;
    }

    std::size_t novoFecho(const std::size_t filho) {
        No no;
        no.tipo = TipoDeNo::Fecho;
        no.esquerda = filho;
        nos_.push_back(no);
        return nos_.size() - 1;
    }

    // recorte:inicio clonar-em-vez-de-compartilhar
    // Duplica a subárvore enraizada em `origem`. A redução de `+` precisa da
    // subárvore duas vezes -- uma direta e outra sob o fecho --, e
    // compartilhar o mesmo índice nas duas posições produziria um grafo, não
    // uma árvore: uma peça futura que percorra a árvore duas vezes pelo mesmo
    // caminho encontraria os mesmos nós contados em dobro.
    std::size_t clonar(const std::size_t origem) {
        const No& modelo = nos_[origem];
        No copia;
        copia.tipo = modelo.tipo;
        copia.nome = modelo.nome;
        const std::size_t esquerdaOriginal = modelo.esquerda;
        const std::size_t direitaOriginal = modelo.direita;
        // Os filhos são clonados ANTES de o pai entrar no vetor: o
        // push_back de um filho pode realocar o vetor e invalidar `modelo`.
        const std::size_t esquerdaCopia =
            esquerdaOriginal == kSemFilho ? kSemFilho : clonar(esquerdaOriginal);
        const std::size_t direitaCopia =
            direitaOriginal == kSemFilho ? kSemFilho : clonar(direitaOriginal);
        copia.esquerda = esquerdaCopia;
        copia.direita = direitaCopia;
        nos_.push_back(copia);
        return nos_.size() - 1;
    }
    // recorte:fim clonar-em-vez-de-compartilhar

    // --- leitura de tokens ----------------------------------------------

    const Token& atual() const { return tokens_[posicao_]; }

    Resultado falhar(std::size_t /*marcador*/, Resultado resultado) {
        resultado.ok = false;
        resultado.erro = erro_;
        return resultado;
    }

    // Registra o primeiro erro e ignora os seguintes -- um analisador que
    // segue depois da falha produziria erros em cascata, todos derivados do
    // primeiro.
    std::size_t erroEm(const std::string& mensagem) {
        if (!falhou_) {
            falhou_ = true;
            erro_.posicao = atual().posicao;
            erro_.mensagem = mensagem;
        }
        return kSemFilho;
    }

    // --- produções --------------------------------------------------------

    // recorte:inicio precedencia-por-descida
    // alternancia := concatenacao ( '|' concatenacao )*
    std::size_t alternancia() {
        std::size_t esquerda = concatenacao();
        if (falhou_) {
            return kSemFilho;
        }
        while (atual().tipo == TipoToken::Barra) {
            ++posicao_;
            const std::size_t direita = concatenacao();
            if (falhou_) {
                return kSemFilho;
            }
            esquerda = novoBinario(TipoDeNo::Alternancia, esquerda, direita);
        }
        return esquerda;
    }
    // recorte:fim precedencia-por-descida

    // recorte:inicio associatividade-na-arvore
    // concatenacao := repeticao ( '->' repeticao )*
    // A associatividade à esquerda está na FORMA da árvore, não em nota à
    // parte: `a -> b -> c` vira Concat(Concat(a,b),c).
    std::size_t concatenacao() {
        std::size_t esquerda = repeticao();
        if (falhou_) {
            return kSemFilho;
        }
        while (atual().tipo == TipoToken::Seta) {
            ++posicao_;
            const std::size_t direita = repeticao();
            if (falhou_) {
                return kSemFilho;
            }
            esquerda = novoBinario(TipoDeNo::Concatenacao, esquerda, direita);
        }
        return esquerda;
    }
    // recorte:fim associatividade-na-arvore

    // recorte:inicio reducao-ao-nucleo
    // repeticao := atomo ( '*' | '+' | '?' )*
    // As duas reduções ao núcleo moram aqui. Aceitar sufixos repetidos custa
    // um laço e evita recusar `a+*`, que é redundante, não malformado.
    std::size_t repeticao() {
        std::size_t no = atomo();
        if (falhou_) {
            return kSemFilho;
        }
        while (atual().tipo == TipoToken::Asterisco || atual().tipo == TipoToken::Mais ||
               atual().tipo == TipoToken::Interrogacao) {
            const TipoToken sufixo = atual().tipo;
            ++posicao_;
            if (sufixo == TipoToken::Asterisco) {
                no = novoFecho(no);
            } else if (sufixo == TipoToken::Mais) {
                // x+ reduz a concat(x, fecho(x)).
                const std::size_t copia = clonar(no);
                no = novoBinario(TipoDeNo::Concatenacao, no, novoFecho(copia));
            } else {
                // x? reduz a alt(x, vazio).
                no = novoBinario(TipoDeNo::Alternancia, no, novoVazio());
            }
        }
        return no;
    }
    // recorte:fim reducao-ao-nucleo

    // atomo := ID | '[' alternancia ']'
    std::size_t atomo() {
        if (atual().tipo == TipoToken::Identificador) {
            const std::string nome = atual().texto;
            ++posicao_;
            return novoIdentificador(nome);
        }
        if (atual().tipo == TipoToken::ColcheteAbre) {
            ++posicao_;
            const std::size_t interno = alternancia();
            if (falhou_) {
                return kSemFilho;
            }
            if (atual().tipo != TipoToken::ColcheteFecha) {
                return erroEm("falta o fecha-colchete do grupo");
            }
            ++posicao_;
            return interno;  // grupo elidido -- nao sobrevive a leitura
        }
        if (atual().tipo == TipoToken::Asterisco || atual().tipo == TipoToken::Mais ||
            atual().tipo == TipoToken::Interrogacao) {
            return erroEm("operador de repeticao sem expressao a que se aplicar");
        }
        if (atual().tipo == TipoToken::ColcheteFecha) {
            return erroEm("fecha-colchete sem abertura correspondente");
        }
        if (atual().tipo == TipoToken::Fim) {
            return erroEm("expressao terminou antes do esperado");
        }
        return erroEm("esperava um identificador ou um grupo entre colchetes aqui");
    }

    std::vector<Token> tokens_;
    std::size_t posicao_ = 0;
    std::vector<No> nos_;
    bool falhou_ = false;
    ErroDeSintaxe erro_;
};

// recorte:inicio forma-prefixa-comparavel
void escreverPrefixa(const Arvore& arvore, const std::size_t indice, std::string& saida) {
    if (indice == kSemFilho) {
        return;
    }
    const No& no = arvore.nos[indice];
    switch (no.tipo) {
        case TipoDeNo::Identificador:
            saida += no.nome;
            return;
        case TipoDeNo::Vazio:
            saida += "vazio";
            return;
        case TipoDeNo::Concatenacao:
            saida += "concat(";
            break;
        case TipoDeNo::Alternancia:
            saida += "alt(";
            break;
        case TipoDeNo::Fecho:
            saida += "fecho(";
            break;
    }
    escreverPrefixa(arvore, no.esquerda, saida);
    if (no.direita != kSemFilho) {
        saida += ", ";
        escreverPrefixa(arvore, no.direita, saida);
    }
    saida += ')';
}
// recorte:fim forma-prefixa-comparavel

}  // namespace

Resultado analisarExpressao(const std::string& expressao) {
    Resultado resultado;
    if (expressao.empty()) {
        resultado.ok = false;
        resultado.erro.posicao = 0;
        resultado.erro.mensagem = "expressao vazia";
        return resultado;
    }
    ErroLexico erroLexico;
    std::vector<Token> tokens = tokenizar(expressao, erroLexico);
    if (erroLexico.houve) {
        resultado.ok = false;
        resultado.erro.posicao = erroLexico.posicao;
        resultado.erro.mensagem = erroLexico.mensagem;
        return resultado;
    }
    Analisador analisador(std::move(tokens));
    return analisador.analisar();
}

std::string formatarArvore(const Arvore& arvore) {
    if (arvore.vazia()) {
        return "(arvore vazia)";
    }
    std::string saida;
    escreverPrefixa(arvore, arvore.raiz, saida);
    return saida;
}

std::string formatarErro(const std::string& expressao, const ErroDeSintaxe& erro) {
    std::string saida = "  " + expressao + '\n';
    saida += "  ";
    // A posição é contada em caracteres desde zero; o cursor vai exatamente
    // sob o ponto recusado -- contar coluna manualmente é o trabalho que a
    // mensagem existe para poupar.
    for (std::size_t i = 0; i < erro.posicao && i < expressao.size(); ++i) {
        saida += ' ';
    }
    saida += "^ ";
    saida += erro.mensagem;
    saida += " (posicao " + std::to_string(erro.posicao) + ")";
    return saida;
}

}  // namespace komboscript
