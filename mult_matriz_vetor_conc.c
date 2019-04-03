/* Disciplina: Computacao Concorrente */
/* Prof.: Silvana Rossetto */
/* Laboratório: 2 */
/* Codigo: Multiplica uma matriz por um vetor */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "timer.h"

//variaveis globais
//float *matA; //matriz de entrada
//float *vetX; //vetor de entrada
//float *vetB; //vetor de saida


typedef struct {
	float *matA;
	float *vetX;
	float *vetB;
	int linhas, colunas, nthreads;
	int tid;
} targs;

// função que as threads executam, multiplicando um bloco da matriz pelo vetor
	void *multiplicaMatrizVetor(void *args) {
	targs *arg = (targs *) args;
	float *a = arg->matA;
	float *x = arg->vetX;
	float *b = arg->vetB;
	int linhas = arg->linhas;
	int colunas = arg->colunas;
	int index = arg->tid;
	int nthreads = arg->nthreads;

	int inicio, fim; //intervalo de elementos processados por cada thread
	int tam_bloco = colunas/nthreads;
	inicio = index * tam_bloco;
	// o ultimo fluxo trata os elementos restantes
	if (index < nthreads-1) fim = inicio + tam_bloco;
	else fim = colunas;
	printf("thread: %d --- inicio: %d ---- fim: %d\n", index, inicio, fim);

	int i, j;
   for (i=0; i<linhas; i++) {
      b[i] = 0;
      for (j=inicio; j<fim; j++) {
         b[i] += a[i*colunas+j] * x[j];
      }
   }

	free(args);
	pthread_exit(NULL);
}

//funcao que aloca espaco para uma matriz e preenche seus valores
//entrada: matriz de entrada, dimensoes da matriz
//saida: retorna 1 se a matriz foi preenchida com sucesso e 0 caso contrario
int preencheMatriz(float **mat, int linhas, int colunas, FILE *arq) {
   int i, j;
   //aloca espaco de memoria para a matriz
   *mat = (float*) malloc(sizeof(float) * linhas * colunas);
   if (mat == NULL) return 0;
   //preenche o vetor
   for (i=0; i<linhas; i++) {
      for (j=0; j<colunas; j++) {
         //fscanf(arq, "%f", *( (*mat) + (i*colunas+j) ) );
         fscanf(arq, "%f", (*mat) + (i*colunas+j));
      }
   }
   return 1;
}

//funcao que imprime uma matriz
//entrada: matriz de entrada, dimensoes da matriz
//saida: matriz impressa na tela
void imprimeMatriz(float *mat, int linhas, int colunas, FILE *arq) {
   int i, j;
   for (i=0; i<linhas; i++) {
      for (j=0; j<colunas; j++) {
         fprintf(arq, "%.1f ", mat[i*colunas+j]);
      }
      fprintf(arq, "\n");
   }
}

//funcao que aloca espaco para um vetor e preenche seus valores
//entrada: vetor de entrada, dimensao do vetor
//saida: retorna 1 se o vetor foi preenchido com sucesso e 0 caso contrario
int preencheVetor(float **vet, int dim, FILE *arq) {
   int i;
   //aloca espaco de memoria para o vetor
   *vet = (float*) malloc(sizeof(float) * dim);
   if (vet == NULL) return 0;
   //preenche o vetor
   for (i=0; i<dim; i++) {
       //*( (*vet)+i ) = 1.0;
       fscanf(arq, "%f", (*vet) + i);
   }
   return 1;
}

//funcao que imprime um vetor
//entrada: vetor de entrada, dimensao do vetor
//saida: vetor impresso na tela
void imprimeVetor(float *vet, int dim, FILE *arq) {
   int i;
   for (i=0; i<dim; i++) {
      fprintf(arq, "%.1f ", vet[i]);
   }
   fprintf(arq, "\n");
}

//funcao principal
int main(int argc, char *argv[]) {
   float *matA; //matriz de entrada
   float *vetX; //vetor de entrada
   float *vetB; //vetor de saida
   FILE *arqA, *arqX, *arqB; //arquivos dos dados de entrada e saida
   int linhas, colunas; //dimensoes da matriz de entrada
   int dim; //dimensao do vetor de entrada

	int nthreads;
	int t; // para os loops de criar e esperar threads terminarem

	double inicio, fim, delta1, delta2, delta3; //variaveis para medir o tempo de execucao
	pthread_t *tid_sistema; //vetor de identificadores das threads no sistema
	int *tid; //identificadores das threads no programa

	// --------------------------------------------------------------------------------------------------------
	GET_TIME(inicio);

   //le e valida os parametros de entrada
   //o arquivo da matriz de entrada deve conter na primeira linha as dimensoes da matriz (linha coluna) seguido dos elementos da matriz separados por espaco
   //o arquivo do vetor de entrada deve conter na primeira linha a dimensao do vetor seguido dos elementos separados por espaco
   if(argc < 5) {
      fprintf(stderr, "Digite: %s <arquivo matriz A> <arquivo vetor X> <arquivo vetor B> <numero de threads>.\n", argv[0]);
      exit(EXIT_FAILURE);
   }

	// guarda o número de threads a se usar
	nthreads = atoi(argv[4]);

	//aloca espaco para o vetor de identificadores das threads no sistema
  tid_sistema = (pthread_t *) malloc(sizeof(pthread_t) * nthreads);
  if(tid_sistema==NULL) {
      printf("--ERRO: malloc()\n"); exit(-1);
  }

   //abre o arquivo da matriz de entrada
   arqA = fopen(argv[1], "r");
   if(arqA == NULL) {
      fprintf(stderr, "Erro ao abrir o arquivo da matriz de entrada.\n");
      exit(EXIT_FAILURE);
   }
   //le as dimensoes da matriz de entrada
   fscanf(arqA, "%d", &linhas);
   fscanf(arqA, "%d", &colunas);

   //abre o arquivo do vetor de entrada
   arqX = fopen(argv[2], "r");
   if(arqX == NULL) {
      fprintf(stderr, "Erro ao abrir o arquivo do vetor de entrada.\n");
      exit(EXIT_FAILURE);
   }
   //le a dimensao do vetor de entrada
   fscanf(arqX, "%d", &dim);

   //valida as dimensoes da matriz e vetor de entrada
   if(colunas != dim) {
      fprintf(stderr, "Erro: as dimensoes da matriz e do vetor de entrada nao sao compativeis.\n");
      exit(EXIT_FAILURE);
   }

   //abre o arquivo do vetor de saida
   arqB = fopen(argv[3], "w");
   if(arqB == NULL) {
      fprintf(stderr, "Erro ao abrir o arquivo do vetor de saida.\n");
      exit(EXIT_FAILURE);
   }

   //aloca e preenche a matriz de entrada
   if(preencheMatriz(&matA, linhas, colunas, arqA) == 0) {
      fprintf(stderr, "Erro de preenchimento da matriz de entrada\n");
      exit(EXIT_FAILURE);
   }
   //aloca e preenche o vetor de entrada
   if(preencheVetor(&vetX, dim, arqX) == 0) {
      fprintf(stderr, "Erro de preenchimento do vetor de entrada\n");
      exit(EXIT_FAILURE);
   }
   //aloca o vetor de saida
   vetB = (float*) malloc(sizeof(float) * linhas);
   if(vetB==NULL) {
      fprintf(stderr, "Erro de alocacao do vetor de saida\n");
      exit(EXIT_FAILURE);
   }
	GET_TIME(fim);

	//calcula o tempo gasto com as inicializacoes
	delta1 = fim - inicio;
	// --------------------------------------------------------------------------------------------------------


   /*
   //imprime a matriz de entrada
   printf("Matriz de entrada:\n");
   imprimeMatriz(matA, linhas, colunas, stdout);
   //imprime o vetor de entrada
   printf("Vetor de entrada:\n");
   imprimeVetor(vetX, colunas, stdout);
   */

	// --------------------------------------------------------------------------------------------------------
	GET_TIME(inicio);

//cria as threads
  for(t=0; t<nthreads; t++) {
    tid = malloc(sizeof(int)); if(tid==NULL) { printf("--ERRO: malloc()\n"); exit(-1); }
    *tid = t;
	
	
	targs *args = (targs *) malloc(sizeof(targs));
	args->matA = matA;
	args->vetX = vetX;
	args->vetB = vetB;
	args->linhas = linhas;
	args->colunas = colunas;
	args->tid = t;
	args->nthreads = nthreads;
	
    if (pthread_create(&tid_sistema[t], NULL, multiplicaMatrizVetor, (void*) args)) {
       printf("--ERRO: pthread_create()\n"); exit(-1);
    }
  }

  //espera todas as threads terminarem e imprime o vetor de saída
  for(t=0; t<nthreads; t++) {
     if (pthread_join(tid_sistema[t], NULL)) {
        printf("--ERRO: pthread_join()\n"); exit(-1);
     }
  }	

   //multiplica a matriz de entrada pelo vetor de entrada
   //multiplicaMatrizVetor(matA, vetX, vetB, linhas, colunas);
   
	GET_TIME(fim);

	//calcula o tempo gasto com a multiplicação
	delta2 = fim - inicio;
	// --------------------------------------------------------------------------------------------------------

	// --------------------------------------------------------------------------------------------------------
	GET_TIME(inicio);

   //imprime o vetor de saida no arquivo de saida
   imprimeVetor(vetB, linhas, arqB);

   //libera os espacos de memoria alocados
   free(matA);   
   free(vetX);   
   free(vetB);   
   
	GET_TIME(fim);

	//calcula o tempo gasto com a finalização do programa
	delta3 = fim - inicio;
	// --------------------------------------------------------------------------------------------------------

	//exibe os tempos gastos em cada parte do programa 
	printf("Tempo inicializacoes: %.8lf\n", delta1);
	printf("Tempo de multiplicacao: %.8lf\n", delta2);
	printf("Tempo finalizacoes: %.8lf\n", delta3);

   //return 0;
	
	//printf("--Thread principal terminou\n");
	pthread_exit(NULL);
}

