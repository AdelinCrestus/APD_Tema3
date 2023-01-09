#include <stdio.h>
#include "mpi.h"
#include "string.h"
#include "stdlib.h"

int previous(int x)
{
    if(x == 0)
    {
        return 3;
    }
    else return x-1;
}

int succesor(int x)
{
    if(x == 3)
    {
        return 0;
    }
    else return x+1;
}

int *ring(int initiator, int rank, int *msg, int size)
{
    int *nr = NULL;
    if(rank == initiator)
    {
        MPI_Send(msg, size, MPI_INT, succesor(rank), 0, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, succesor(rank));
    }
    else if( rank == previous(initiator))
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

void printare_topo(int rank, int *size, int **clusters_workers)
{

        printf("%d ->", rank);
        for(int i = 0; i < 4; i++)
        {
            printf(" %d:", i);
            for(int j = 0; j < size[i]; j++)
            {
                if( j == 0)
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
    if (rank < 4)
    {
        char *string = calloc(15, sizeof(char));
        strcat(string, "cluster");
        string[strlen(string)] = (char)('0' - 0 + rank);
        strcat(string, ".txt");
        FILE *f = fopen(string, "r");
        int number_workers;
        int *workers;
        fscanf(f, "%d", &number_workers);
        workers = calloc(number_workers, sizeof(int));
        for (int i = 0; i < number_workers; i++)
        {
            int aux;
            fscanf(f, "%d", &aux);
            workers[i] = aux;
        }

        for(int i = 0; i < 4; i++)
        {
            int *aux = ring(i, rank, &number_workers, 1);
            if(aux)
            {
                size[i] = *aux;
            }
            else 
            {
                size[i] = number_workers;
            }
        }
        // printf("rank: %d, size = ", rank);
        // for(int i = 0; i < 4; i++)
        // {
        //     printf(" %d ", size[i]);
        // }
        // printf("\n");

        for(int i = 0; i < 4; i++)
        {
            int *aux = ring(i, rank, workers, size[i]);
            if(aux)
            {
                clusters_workers[i] = aux;
            }
            else
            {
                clusters_workers[i] = workers;
            }
        }

        // for(int i = 0; i < 4; i++)
        // {
        //     for(int j = 0; j < size[i]; j++)
        //     {
        //         printf(" %d ", clusters_workers[i][j]);
        //     }
        //     printf("\n");
        // }
        // printf("\n");

        printare_topo(rank, size, clusters_workers);

        for(int i = 0; i < number_workers; i++)
        {
            for(int j = 0; j < 4; j++)
            {
                MPI_Send(&size[j], 1, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
                printf("M(%d,%d)\n", rank, workers[i]);
            }
            for(int j = 0; j < 4; j++)
            {
                MPI_Send(clusters_workers[j], size[j], MPI_INT, workers[i], 0, MPI_COMM_WORLD);
            
                printf("M(%d,%d)\n", rank, workers[i]);
            }
        }


    }
    else
    {
        for(int j = 0; j < 4; j++)
        {
            MPI_Recv(&size[j], 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        for(int i = 0; i < 4; i++)
        {
            clusters_workers[i] = calloc(size[i], sizeof(int));
        }
        for(int j = 0; j < 4; j++)
        {
            MPI_Recv(clusters_workers[j], size[j], MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        printare_topo(rank, size, clusters_workers);

    }

    MPI_Finalize();
}