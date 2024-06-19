#include <stdio.h>
#include <stdlib.h>
#define TAMANHO_TABELA 34 // Tamanho da tabela de frequência

typedef struct no {
  unsigned char caractere;
  unsigned int frequencia;
  struct no *esquerda, *direita, *proximo; // Nós para árvore de Huffman
} No;

typedef struct listaEncadeada {
  No *inicio;
  unsigned int tamanho;
} ListaEncadeada;

struct compactadora {
  char simbolo;
  unsigned int codigo;
  int tamanho;
};

void inicializa_tabela_com_zero(unsigned int tabela[]);
void preenche_tabela_de_frequencia(FILE *arqTexto, unsigned int tabela[]);
void imprime_tabela_de_frequencia(unsigned int tabela[]);
void criar_lista_encadeada(ListaEncadeada *lista);
void inserir_elementos_ordenamente_na_lista(ListaEncadeada *lista, No *no);
void preencher_lista_ordenada(unsigned int tabela[], ListaEncadeada *lista);
void imprime_lista_encadeada(ListaEncadeada *lista);
void compacta(FILE *arqTexto, FILE *arqBin, struct compactadora *v);
void descompacta(FILE *arqBin, FILE *arqTexto);
No *extrair_menor(ListaEncadeada *lista);
No *construir_arvore_huffman(ListaEncadeada *lista);
void gerar_codigos_huffman(No *raiz, unsigned int codigo, int tamanho,
                           struct compactadora *v, int *indice);



int main(void) {
  FILE *arqTexto, *arqBin, *arqTextoDecodificado;
  unsigned int tabela_de_frequencia[TAMANHO_TABELA];
  ListaEncadeada lista;

  arqTexto = fopen("textoParaLer.txt", "r");
  if (arqTexto == NULL) {
    printf("Erro ao abrir o arquivo 'textoParaLer.txt'.\n");
    return 1;
  }

  inicializa_tabela_com_zero(tabela_de_frequencia);
  preenche_tabela_de_frequencia(arqTexto, tabela_de_frequencia);
  imprime_tabela_de_frequencia(tabela_de_frequencia);
  fclose(arqTexto);

  criar_lista_encadeada(&lista);
  preencher_lista_ordenada(tabela_de_frequencia, &lista);
  No *raiz = construir_arvore_huffman(&lista);
  struct compactadora v[TAMANHO_TABELA];
  int indice = 0;
  gerar_codigos_huffman(raiz, 0, 0, v, &indice);

  arqTexto = fopen("textoParaLer.txt", "r");
  if (!arqTexto) {
    perror("Erro ao abrir 'textoParaLer'");
    return 1;
  }

  arqBin = fopen("textoCodificado.bin", "wb");
  if (!arqBin) {
    perror("Erro ao abrir 'textoCodificado'");
    return 1;
  }

  arqTextoDecodificado = fopen("textoDecodificado.txt", "w");
  if (!arqTextoDecodificado) {
    perror("Erro ao abrir 'textoDecodificado'");
    return 1;
  }

  if (!arqTexto || !arqBin || !arqTextoDecodificado) {
    printf("Erro ao abrir um dos arquivos para compactação/descompactação.\n");
    return 1;
  }

  compacta(arqTexto, arqBin, v);
  fclose(arqTexto);
  fclose(arqBin);

  arqBin = fopen("textoCodificado.bin", "rb");
  if (!arqBin) {
    perror("Falha ao reabrir o arquivo binário para leitura");
    return 1;
  }
  descompacta(arqBin, arqTextoDecodificado);
  fclose(arqBin);
  fclose(arqTextoDecodificado);

  return 0;
}

void inicializa_tabela_com_zero(unsigned int tabela[]) {
  for (int i = 0; i < TAMANHO_TABELA; i++) {
    tabela[i] = 0;
  }
}

void preenche_tabela_de_frequencia(FILE *arqTexto, unsigned int tabela[]) {
  int c = fgetc(arqTexto); 

  while (c != EOF) { 
    if (c >= 'A' && c <= 'Z') {
      tabela[c - 'A']++;
    } else if (c == ' ') {
      tabela[26]++;
    } else if (c == ',') {
      tabela[27]++;
    } else if (c == '.') {
      tabela[28]++;
    } else if (c == '\n') {
      tabela[29]++;
    } else if (c == ';') {
      tabela[30]++;
    } else if (c == '?') {
      tabela[31]++;
    } else if (c == '!') {
      tabela[32]++;
    }
    c = fgetc(arqTexto); 
  }
  tabela[33] = 1; //adiciona um EOF no final 
}

void imprime_tabela_de_frequencia(unsigned int tabela[]) {
  printf("\tTABELA DE FREQUENCIA\n");
  for (int i = 0; i < 26; i++) {
    printf("\t%u = %c = %u\n", i, 'A' + i, tabela[i]);
  }
  printf("\t26 = space = %u\n", tabela[26]); 
  printf("\t27 = ',' = %u\n", tabela[27]);  
  printf("\t28 = '.' = %u\n", tabela[28]);  
  printf("\t29 = '\\n' = %u\n", tabela[29]);
  printf("\t30 = ';' = %u\n", tabela[30]);  
  printf("\t31 = '?' = %u\n", tabela[31]);   
  printf("\t32 = '!' = %u\n", tabela[32]); 
  printf("\t33 = EOF = %u\n", tabela[33]);  
}

void criar_lista_encadeada(ListaEncadeada *lista) {
  lista->inicio = NULL;
  lista->tamanho = 0;
}

void inserir_elementos_ordenamente_na_lista(ListaEncadeada *lista, No *no) {
  No *auxiliar;
  if (lista->inicio == NULL) {
    lista->inicio = no;

  } else if (no->frequencia < lista->inicio->frequencia) {
    // novo nó aponta para o inicio da lista
    no->proximo = lista->inicio;
    lista->inicio = no;

  } else {
    auxiliar =
        lista->inicio; 
    while (auxiliar->proximo &&
           auxiliar->proximo->frequencia <=
               no->frequencia) { 
      auxiliar = auxiliar->proximo; 
    }
    no->proximo = auxiliar->proximo; // insere o no no meio da lista
    auxiliar->proximo = no; 
  }
  lista->tamanho++;
}

void preencher_lista_ordenada(unsigned int tabela[], ListaEncadeada *lista) {
    int i;
    No *novo;

    for (i = 0; i < TAMANHO_TABELA; i++) {
        if (tabela[i] > 0) {
            novo = malloc(sizeof(No));
            if (novo == NULL) {
                printf("\n\tERRO ao alocar memoria em preencher_lista_ordenada!\n");
                break;
            }
            if (i < 26) { 
                novo->caractere = 'A' + i;
            } else if (i == 26) {
                novo->caractere = ' '; 
            } else if (i == 27) {
                novo->caractere = ','; 
            } else if (i == 28) {
                novo->caractere = '.';
            } else if (i == 29) {
                novo->caractere = '\n'; 
            } else if (i == 30) {
                novo->caractere = ';'; 
            } else if (i == 31) {
                novo->caractere = '?'; 
            } else if (i == 32) {
                novo->caractere = '!'; 
            }

            novo->frequencia = tabela[i];
            novo->esquerda = NULL;
            novo->direita = NULL;
            novo->proximo = NULL;

            inserir_elementos_ordenamente_na_lista(lista, novo);
        }
    }
}

void imprime_lista_encadeada(ListaEncadeada *lista) {
  No *auxiliar =
      lista->inicio; // comecar do primeiro no da lista para percorrê-la

  printf("\t\nLISTA ENCADEADA - TAMANHO: %u\n", lista->tamanho);
  while (auxiliar != NULL) {
    printf("\tCaractere: %c, Frequencia: %u\n", auxiliar->caractere,
           auxiliar->frequencia);
    auxiliar = auxiliar->proximo;
  }
}

No *extrair_menor(ListaEncadeada *lista) { //pega o de menor frequencia 
  if (lista->inicio == NULL)
    return NULL;
  No *menor = lista->inicio;      
  lista->inicio = menor->proximo; 
  lista->tamanho--;               
  menor->proximo = NULL;         
  return menor;                  
}

No *construir_arvore_huffman(ListaEncadeada *lista) {
  while (lista->tamanho > 1) {     
    No *esq = extrair_menor(lista); 
    No *dir = extrair_menor(lista); 

    No *novo = malloc(sizeof(No));
    if (!novo) {
      printf("Falha na alocação de memória para novo nó.\n");
      return NULL;
    }
    novo->caractere = 0; 
    novo->frequencia = esq->frequencia + dir->frequencia;
    novo->esquerda = esq;
    novo->direita = dir;
    novo->proximo = NULL;

    inserir_elementos_ordenamente_na_lista(lista, novo);
  }
  return lista->inicio; 
}

void gerar_codigos_huffman(No *raiz, unsigned int codigo, int tamanho, struct compactadora *v, int *indice) {
  if (raiz == NULL)
    return;
  if (raiz->esquerda == NULL && raiz->direita == NULL) { // É uma folha
    v[*indice].simbolo = raiz->caractere;
    v[*indice].codigo = codigo;
    v[*indice].tamanho = tamanho;
    (*indice)++;
  } else {
    gerar_codigos_huffman(raiz->esquerda, (codigo << 1) + 0, tamanho + 1, v, indice);
    gerar_codigos_huffman(raiz->direita, (codigo << 1) + 1, tamanho + 1, v, indice);
  }
}

void imprime_arvore_huffman(No *raiz, int tam) {
  if (raiz->esquerda == NULL && raiz->direita == NULL) {
    printf("\t\nFolha: %c\tAltura: %d\n", raiz->caractere, tam);
  } else {
    imprime_arvore_huffman(raiz->esquerda, tam + 1);
    imprime_arvore_huffman(raiz->direita, tam + 1);
  }
}

void compacta(FILE *arqTexto, FILE *arqBin, struct compactadora *v) {
  unsigned char bit_buffer = 0; 
  int bit_count = 0; 
  char ch; 

  fwrite(v, sizeof(struct compactadora), TAMANHO_TABELA, arqBin); //add tabela no arquivo 

  while (fscanf(arqTexto, "%c", &ch) != EOF) {
    for (int i = 0; i < TAMANHO_TABELA; i++) {
      if (ch == v[i].simbolo) { 
        for (int j = v[i].tamanho - 1; j >= 0; j--) {
          bit_buffer = (bit_buffer << 1) | ((v[i].codigo >> j) & 1);
          bit_count++;
          if (bit_count == 8) {
            fwrite(&bit_buffer, 1, 1, arqBin); 
            bit_buffer = 0; 
            bit_count = 0; 
          }
        }
        break; 
      }
    }
  }
  for (int i = 0; i < TAMANHO_TABELA; i++) {
    if (v[i].simbolo == '\0') { 
      for (int j = v[i].tamanho - 1; j >= 0; j--) {
        bit_buffer = (bit_buffer << 1) | ((v[i].codigo >> j) & 1);
        bit_count++;
        if (bit_count == 8) {
          fwrite(&bit_buffer, 1, 1, arqBin); 
          bit_buffer = 0;
          bit_count = 0;
        }
      }
      break; 
    }
  }
  if (bit_count > 0) {
    bit_buffer <<= (8 - bit_count);
    fwrite(&bit_buffer, 1, 1, arqBin); 
  }
}

void descompacta(FILE *arqBin, FILE *arqTexto) {
  unsigned char bit_buffer; //armazena aq
  int bit_count = 0;
  unsigned int codigo_atual = 0;
  int bits_no_codigo = 0;
  struct compactadora v[TAMANHO_TABELA];

  fread(v, sizeof(struct compactadora), TAMANHO_TABELA, arqBin);

  while (fread(&bit_buffer, 1, 1, arqBin)) {
    for (int i = 7; i >= 0; i--) {
      codigo_atual = (codigo_atual << 1) | ((bit_buffer >> i) & 1);
      bits_no_codigo++;

      for (int j = 0; j < TAMANHO_TABELA; j++) {
        if (bits_no_codigo == v[j].tamanho && codigo_atual == v[j].codigo) {
          if (v[j].simbolo == '\0') {  
            return;  
          }
          fprintf(arqTexto, "%c", v[j].simbolo);
          codigo_atual = 0;
          bits_no_codigo = 0;
          break;
        }
      }
    }
  }
}

