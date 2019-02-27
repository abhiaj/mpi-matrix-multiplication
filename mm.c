#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

/*
 * MATSIZE = Tamanho da matriz
 * MAXNUMBER = Valor máximo de um número dentro da matriz
 * NRA = Número de linhas na matriz A
 * NRB = Número de linhas na matriz B
 * NCA = Número de colunas ma matriz A
 * NCB = Número de colunas ma matriz B
 * -- Importante: Número de colunas da matriz A deve ser igual
 * -- ao número de linhas da matriz B
 * MASTER = ID do master (0), apenas para facilitar leitura
*/

#define MATSIZE 5
#define MAXNUMBER 5
#define NRA MATSIZE
#define NCA MATSIZE
#define NRB NCA /* Número de linhas de B = número de colunas de A */
#define NCB MATSIZE
#define MASTER 0
#define FROM_MASTER 1
#define FROM_WORKER 2

void printMatrix(double matrix[][MATSIZE],int nrows,int ncolumns, char* title) {
    /* Print results */
    printf("******************************************************\n");
    printf("%s:\n",title);
    for (int i=0; i<nrows; i++) {
        printf("\n");
        for (int j=0; j<ncolumns; j++) {
			printf("%6.2f   ", matrix[i][j]);
        }
    }
    printf("\n******************************************************\n");
}

int main (int argc, char *argv[]) {
    /*
     * nproc = número de processos
     * id = identificador do processo atual
     * numslaves = número de slaves disponíveis
     * source, dest, mtype -> apenas para controle do envio de mensagens
     * rows = linhas da matriz A enviada para cada slave
     * averow, extra, offset -> utilizados para determinar as linhas que são enviadas
     *
     * a = matriz A
     * b = matriz B
     * c = matriz Resultado
     */
    int	nproc, id, numslaves, source, dest, mtype, rows, averow, extra, offset, i, j, k, rc;
    double	a[NRA][NCA], b[NRB][NCB], c[NRA][NCB];
    MPI_Status status;

    //Semente para função rand
    srand(time(NULL));

    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&id);
    MPI_Comm_size(MPI_COMM_WORLD,&nproc);

    /* Número de escravos é sempre número total de processos menos um (o master) */
    numslaves = nproc-1;

    /* Processo Principal - MASTER */
    if (id == MASTER) {
        printf("Utilizando %d processos, sendo %d slaves.\n",nproc,numslaves);
        printf("Iniciando matrizes...\n");
        for (i=0; i<NRA; i++) {
            for (j=0; j<NCA; j++) {
                a[i][j]= rand()%MAXNUMBER;
            }
        }
        for (i=0; i<NRA; i++) {
            for (j=0; j<NCB; j++) {
                b[i][j]= rand()%MAXNUMBER;
            }
        }

        printMatrix(a,NRA,NCA,"Matriz A");
        printMatrix(b,NRB,NCB,"Matriz B");

        /* Computa o tempo de ínicio */
        double start = MPI_Wtime();

        /* Envia matriz para os slaves */
        averow = NRA/numslaves;
        extra = NRA%numslaves;/* Marcador para identificar quais processos receberão linhas extras */
        offset = 0; /* Offset - serve para marcar o que já foi enviado */
        mtype = FROM_MASTER;
        for (dest=1; dest<=numslaves; dest++) {
            rows = (dest <= extra) ? averow+1 : averow;
            printf("Enviando %d linhas para slave %d offset=%d\n",rows,dest,offset);
            /* Envia o valor do offset */
            MPI_Send(&offset, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
            /* Envia a quantidade de linhas que serão enviadas */
            MPI_Send(&rows, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
            /* Envias as linhas da Matriz A */
            MPI_Send(&a[offset][0], rows*NCA, MPI_DOUBLE, dest, mtype,
                     MPI_COMM_WORLD);
            /* Envia a Matriz B */
            MPI_Send(&b, NRA*NCB, MPI_DOUBLE, dest, mtype, MPI_COMM_WORLD);
            offset = offset + rows;
        }

        /* Recebe resultados dos slaves */
        mtype = FROM_WORKER;
        for (i=1; i<=numslaves; i++) {
            source = i;
            /* Recebe o valor do Offset  e quantidade de linhas
             * para poder posicionar os valores na matriz C */
            MPI_Recv(&offset, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
            MPI_Recv(&rows, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
            MPI_Recv(&c[offset][0], rows*NCB, MPI_DOUBLE, source, mtype,
                     MPI_COMM_WORLD, &status);
            printf("Recebendo linhas do slave %d\n",source);
        }

        printMatrix(c,NRA,NCB,"Resultado Final");

        /* Computa o tempo de término e exibe o tempo total do processo */
        double finish = MPI_Wtime();
        printf("Concluído em %f segundos.\n", finish - start);
    }


    /* Processo SALVES - Todos possuem ID > 0 */
    if (id > MASTER) {
        mtype = FROM_MASTER;
        MPI_Recv(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
        MPI_Recv(&rows, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
        MPI_Recv(&a, rows*NCA, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD, &status);
        MPI_Recv(&b, NRB*NCB, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD, &status);

        for (k=0; k<NCB; k++) {
            for (i=0; i<rows; i++) {
                c[i][k] = 0.0;
                for (j=0; j<NCA; j++) {
                    c[i][k] = c[i][k] + a[i][j] * b[j][k];
                }
            }
        }
        mtype = FROM_WORKER;
        MPI_Send(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
        MPI_Send(&rows, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
        MPI_Send(&c, rows*NCB, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD);
    }
    MPI_Finalize();
}

/*
 * TRABALHOS FUTUROS
 *
 * Ao invés de cortar a matriz A e passar a matriz B inteira,
 * enviar apenas a linha e coluna que deve ser calculada, dessa
 * forma vai diminuir o gasto de memória em cada um dos procesos
 * e as multiplicões se tornam 1xM por Mx1
 *
 * Tornar o código genérico, para que seja uma função única
 * Com isso poderá ser usado dentro de outros códigos de rede
 * neural para realizar o treinamento das mesmas.
 */
