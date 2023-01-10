#include <stdio.h>
#include "mpi.h"
#include "string.h"
#include "stdlib.h"
#include "math.h"

int previous(int x)
{
    if (x == 0)
    {
        return 3;
    }
    else
        return x - 1;
}

int succesor(int x)
{
    if (x == 3)
    {
        return 0;
    }
    else
        return x + 1;
}

int my_floor(float x)
{
    if(x > (int) x)
    {
        return x+1;
    }

    return x;
}

int *ring(int initiator, int rank, int *msg, int size)
{
    int *nr = NULL;
    if (rank == initiator)
    {
        MPI_Send(msg, size, MPI_INT, succesor(rank), 0, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, succesor(rank));
    }
    else if (rank == previous(initiator))
    {
        nr = calloc(size, sizeof(int));
        MPI_Recv(nr, size, MPI_INT, previous(rank), 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    else
    {
        nr = calloc(size, sizeof(int));
        MPI_Recv(nr, size, MPI_INT, previous(rank), 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Send(nr, size, MPI_INT, succesor(rank), 0, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, succesor(rank));
    }
    return nr;
}

int *send_btw_coordinators(int sender, int rank, int receiver, int *msg, int size, int (*f)(int), int (*q)(int))
{
    int *nr = NULL;
    if (rank == sender)
    {
        MPI_Send(msg, size, MPI_INT, (*f)(rank), 0, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, (*f)(rank));
    }
    else if (rank == receiver)
    {
        nr = calloc(size, sizeof(int));
        MPI_Recv(nr, size, MPI_INT, (*q)(rank), 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    else if (rank < receiver)
    {
        nr = calloc(size, sizeof(int));
        MPI_Recv(nr, size, MPI_INT, (*q)(rank), 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Send(nr, size, MPI_INT, (*f)(rank), 0, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, (*f)(rank));
    }
    if (rank == receiver)
    {
        return nr;
    }
    else
        return NULL;
}

void printare_topo(int rank, int *size, int **clusters_workers)
{

    printf("%d ->", rank);
    for (int i = 0; i < 4; i++)
    {
        printf(" %d:", i);
        for (int j = 0; j < size[i]; j++)
        {
            if (j == 0)
                printf("%d", clusters_workers[i][j]);
            else
                printf(",%d", clusters_workers[i][j]);
        }
    }

    printf("\n");
}

int main(int argc, char *argv[])
{
    int numtasks, rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int size[4]; // cati workeri are fiecare cluster
    int **clusters_workers = calloc(4, sizeof(int *));
    int number_workers;
    int *workers;
    if (rank < 4)
    {
        char *string = calloc(15, sizeof(char));
        strcat(string, "cluster");
        string[strlen(string)] = (char)('0' - 0 + rank);
        strcat(string, ".txt");
        FILE *f = fopen(string, "r");

        
        fscanf(f, "%d", &number_workers);
        workers = calloc(number_workers, sizeof(int));
        for (int i = 0; i < number_workers; i++)
        {
            int aux;
            fscanf(f, "%d", &aux);
            workers[i] = aux;
        }

        for (int i = 0; i < 4; i++)
        {
            int *aux = ring(i, rank, &number_workers, 1);
            if (aux)
            {
                size[i] = *aux;
            }
            else
            {
                size[i] = number_workers;
            }
        }

        for (int i = 0; i < 4; i++)
        {
            int *aux = ring(i, rank, workers, size[i]);
            if (aux)
            {
                clusters_workers[i] = aux;
            }
            else
            {
                clusters_workers[i] = workers;
            }
        }

        printare_topo(rank, size, clusters_workers);

        for (int i = 0; i < number_workers; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                MPI_Send(&size[j], 1, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
                printf("M(%d,%d)\n", rank, workers[i]);
            }
            for (int j = 0; j < 4; j++)
            {
                MPI_Send(clusters_workers[j], size[j], MPI_INT, workers[i], 0, MPI_COMM_WORLD);

                printf("M(%d,%d)\n", rank, workers[i]);
            }
        }
    }
    else
    {
        for (int j = 0; j < 4; j++)
        {
            MPI_Recv(&size[j], 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        for (int i = 0; i < 4; i++)
        {
            clusters_workers[i] = calloc(size[i], sizeof(int));
        }
        for (int j = 0; j < 4; j++)
        {
            MPI_Recv(clusters_workers[j], size[j], MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        printare_topo(rank, size, clusters_workers);
    }
    int *quantity = NULL;
    int *vector = NULL;
    if (rank == 0)
    {
        int N = atoi(argv[1]);
        vector = calloc(N, sizeof(int));
        quantity = calloc(4, sizeof(int));
        for (int k = 0; k < N; k++)
        {
            vector[k] = N - k - 1;
        }

        int number_total_workers = 0;
        for (int i = 0; i < 4; i++)
        {
            number_total_workers += size[i];
        }
        float ratio = (float)N / number_total_workers;
        for (int i = 0; i < 3; i++)
        {
            quantity[i] = (int)(ratio * size[i]);
        }
        quantity[3] = N - quantity[0] - quantity[1] - quantity[2];
    }

    if (rank < 4)
    {
        int *aux = ring(0, rank, quantity, 4);
        if (aux)
        {
            quantity = aux;
        }
        int offset = 0;
        for (int i = 1; i < 4; i++)
        {
            offset += quantity[i - 1];
            int *aux = send_btw_coordinators(0, rank, i, &(vector[offset]), quantity[i], succesor, previous);
            if (aux)
            {
                vector = aux;
            }
        }

        
        int quant = quantity[rank] / number_workers;
        int quant_sum = 0;

        for (int i = 0; i < number_workers - 1; i++)
        {
            quant_sum += quant;

        }
        int rest_quant = quantity[rank] - quant_sum;
        for (int i = 0; i < number_workers - 1; i++)
        {
            
            MPI_Send(&quant, 1, MPI_INT, workers[i], workers[i] , MPI_COMM_WORLD);
        }
        MPI_Send(&rest_quant, 1, MPI_INT, workers[number_workers -1], workers[number_workers -1], MPI_COMM_WORLD);
        offset = 0;
        //.........................................................
        for (int i = 0; i < number_workers - 1; i++)
        {
            MPI_Send(&(vector[offset]), quant, MPI_INT,workers[i], workers[i], MPI_COMM_WORLD);
            offset += quant;
        }
        MPI_Send(&(vector[offset]), rest_quant, MPI_INT, workers[number_workers -1], workers[number_workers -1], MPI_COMM_WORLD);
        
    }
    else
    {
        int quant;
        MPI_Status status;
        MPI_Recv(&quant, 1, MPI_INT, MPI_ANY_SOURCE, rank, MPI_COMM_WORLD, &status);
        vector = calloc(100, sizeof(int));
        MPI_Recv(vector, quant, MPI_INT, MPI_ANY_SOURCE, rank, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("rank = %d \n", rank);
        for(int i = 0; i < quant; i++)
        {
           vector[i] *= 5;
        }
        printf("\n");
    }

    MPI_Finalize();
}