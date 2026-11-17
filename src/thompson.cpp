#include "thompson.h"

namespace komboscript {

namespace {

// Um ramo por tipo de nó. Cada um devolve um bloco com uma entrada e uma saída.
Bloco construir(const Arvore& arvore, const std::size_t indice, AfnCombo& afn) {
    const No& no = arvore.nos[indice];
    switch (no.tipo) {
        case TipoDeNo::Identificador:
        case TipoDeNo::Vazio: {
            // Dois estados e uma transição (rótulo = nome, ou vazia).
            const Estado entrada = afn.novoEstado();
            const Estado saida = afn.novoEstado();
            const Simbolo rotulo = no.tipo == TipoDeNo::Identificador ? no.nome : kEpsilon;
            afn.adicionarTransicao(entrada, rotulo, saida);
            return Bloco{entrada, saida};
        }
        case TipoDeNo::Concatenacao: {
            const Bloco esquerdo = construir(arvore, no.esquerda, afn);
            const Bloco direito = construir(arvore, no.direita, afn);
            afn.adicionarTransicao(esquerdo.saida, kEpsilon, direito.entrada);
            return Bloco{esquerdo.entrada, direito.saida};
        }
        case TipoDeNo::Alternancia: {
            const Bloco esquerdo = construir(arvore, no.esquerda, afn);
            const Bloco direito = construir(arvore, no.direita, afn);
            const Estado entrada = afn.novoEstado();
            const Estado saida = afn.novoEstado();
            afn.adicionarTransicao(entrada, kEpsilon, esquerdo.entrada);
            afn.adicionarTransicao(entrada, kEpsilon, direito.entrada);
            afn.adicionarTransicao(esquerdo.saida, kEpsilon, saida);
            afn.adicionarTransicao(direito.saida, kEpsilon, saida);
            return Bloco{entrada, saida};
        }
        case TipoDeNo::Fecho: {
            const Bloco interno = construir(arvore, no.esquerda, afn);
            const Estado entrada = afn.novoEstado();
            const Estado saida = afn.novoEstado();
            afn.adicionarTransicao(entrada, kEpsilon, interno.entrada);        // uma vez ou mais
            afn.adicionarTransicao(entrada, kEpsilon, saida);                  // zero vezes
            afn.adicionarTransicao(interno.saida, kEpsilon, interno.entrada);  // repetir
            afn.adicionarTransicao(interno.saida, kEpsilon, saida);            // terminar
            return Bloco{entrada, saida};
        }
    }
    return Bloco{0, 0};  // inalcançável; evita aviso do compilador
}

}  // namespace

AfnCombo construirThompson(const Arvore& arvore) {
    AfnCombo afn;
    if (arvore.vazia()) {
        // Um único estado: aceita só a sequência vazia.
        const Estado unico = afn.novoEstado();
        afn.definirInicial(unico);
        afn.definirAceitacao(unico);
        return afn;
    }
    const Bloco raiz = construir(arvore, arvore.raiz, afn);
    afn.definirInicial(raiz.entrada);
    afn.definirAceitacao(raiz.saida);
    return afn;
}

}  // namespace komboscript
