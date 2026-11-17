// expressao.h — Leitura de uma expressão de combo e conversão em árvore.
//
// Recebe o texto de uma expressão (o lado direito de um `macro` ou `combo`) e
// devolve a árvore reduzida ao núcleo mínimo ou o primeiro erro encontrado, com 
// a posição exata no texto.
//
// Fecho positivo (`+`), opcional (`?`) e o agrupamento por colchetes são notação 
// de conveniência: são reduzidos ao núcleo durante a leitura, e o que sai daqui
// já não conhece nenhum dos três.
//
// Os nós vivem num vetor e se referenciam por índice, nunca por ponteiro. A
// árvore é copiável, serializável e não vaza; e a duplicação de subárvore que
// a redução de `+` exige vira cópia de faixa de vetor, não passeio recursivo
// de alocação.
//
// `ID` cobre tanto um símbolo primitivo (Baixo, Frente, Jab, ...) quanto a
// referência a uma macro ou combo já declarado -- as duas formas são
// léxicamente idênticas, e esta peça não as distingue: isso é trabalho da
// análise semântica, que ainda não existe.

#ifndef KOMBOSCRIPT_EXPRESSAO_H
#define KOMBOSCRIPT_EXPRESSAO_H

#include <cstddef>
#include <string>
#include <vector>

namespace komboscript {

// Índice ausente. Uma folha não tem filhos; o fecho tem só o esquerdo.
inline constexpr std::size_t kSemFilho = static_cast<std::size_t>(-1);

// recorte:inicio nucleo-minimo-como-tipo
enum class TipoDeNo {
    Identificador,  // símbolo primitivo OU referência a macro/combo
    Vazio,          // cadeia vazia, produzida pela redução do opcional
    Concatenacao,   // núcleo
    Alternancia,    // núcleo
    Fecho,          // núcleo
};

struct No {
    TipoDeNo tipo = TipoDeNo::Vazio;
    std::string nome;                    // significativo apenas em Identificador
    std::size_t esquerda = kSemFilho;
    std::size_t direita = kSemFilho;
};
// recorte:fim nucleo-minimo-como-tipo

struct Arvore {
    std::vector<No> nos;
    std::size_t raiz = kSemFilho;

    bool vazia() const;
};

// Erro de sintaxe com a posição em que foi detectado, contada em caracteres do
// texto da expressão a partir de zero. Carregar a posição desde a leitura é
// bem mais barato do que acrescentá-la depois, quando a análise já estiver
// espalhada por vários pontos (lexer de programa inteiro, análise semântica).
struct ErroDeSintaxe {
    std::size_t posicao = 0;
    std::string mensagem;
};

struct Resultado {
    bool ok = false;
    Arvore arvore;
    ErroDeSintaxe erro;
};

// Lê a expressão e devolve a árvore reduzida ao núcleo, ou o primeiro erro.
Resultado analisarExpressao(const std::string& expressao);

// Forma prefixa canônica da árvore, em uma linha. É o que permite verificar
// que duas notações diferentes da mesma expressão convergiram para a mesma
// estrutura -- comparação de texto, não inspeção visual de duas árvores.
std::string formatarArvore(const Arvore& arvore);

// A mensagem de erro pronta para exibição, com o cursor sob a posição.
std::string formatarErro(const std::string& expressao, const ErroDeSintaxe& erro);

// Número de nós da árvore -- a medida do custo de uma redução.
std::size_t tamanho(const Arvore& arvore);

}  // namespace komboscript

#endif  // KOMBOSCRIPT_EXPRESSAO_H
