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

int succesor1(int x)
{
    if (x == 3)
    {
        return 0;
    }

    if (x == 0)
    {
        return -1;
    }

    return x + 1;
}

int previous1(int x)
{
    if (x == 0)
    {
        return 3;
    }
    if (x == 1)
    {
        return -1;
    }
    return x - 1;
}

int afisare = 1;

int *ring(int initiator, int rank, int *msg, int size)
{
    int *nr = NULL;
    if (rank == initiator)
    {
        MPI_Send(msg, size, MPI_INT, succesor(rank), 0, MPI_COMM_WORLD);
        if (afisare == 1)
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
        if (afisare == 1)
            printf("M(%d,%d)\n", rank, succesor(rank));
    }
    return nr;
}

int exist_in_v(int *v, int x, int nr)
{
    for (int i = 0; i < nr; i++)
    {
        if (v[i] == x)
        {
            return 1;
        }
    }
    return 0;
}

int *ring_def(int initiator, int rank, int *msg, int size, int (*f)(int), int (*q)(int), int last, int nr_adaugat, int adaugare, int *v, int nr_elements_v) // fiecare isi trece numarul de workeri cand mergem de la 0 ->3 -> 2, iar invers doar transmitem inf
{
    int *nr = NULL;

    if (initiator == last)
    {
        return NULL;
    }

    if (rank == initiator)
    {
        MPI_Send(msg, size, MPI_INT, (*f)(rank), 0, MPI_COMM_WORLD);
        if (afisare == 1)
        {
            printf("M(%d,%d)\n", rank, (*f)(rank));
        }
    }
    else if (rank == last)
    {
        nr = calloc(size, sizeof(int));
        MPI_Recv(nr, size, MPI_INT, (*q)(rank), 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    else if (exist_in_v(v, rank, nr_elements_v))
    {
        nr = calloc(size, sizeof(int));
        MPI_Recv(nr, size, MPI_INT, (*q)(rank), 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if (adaugare == 1)
        {
            nr[rank] = nr_adaugat;
        }
        MPI_Send(nr, size, MPI_INT, (*f)(rank), 0, MPI_COMM_WORLD);
        if (afisare == 1)
            printf("M(%d,%d)\n", rank, (*f)(rank));
    }
    return nr;
}
int *send_btw_coordinators(int sender, int rank, int receiver, int *msg, int size, int (*f)(int), int (*q)(int), int to_all)
{
    int *nr = NULL;
    if (rank == sender)
    {
        MPI_Send(msg, size, MPI_INT, (*f)(rank), 0, MPI_COMM_WORLD);
        if (afisare)
        {
            printf("M(%d,%d)\n", rank, (*f)(rank));
        }
    }
    else if (rank == receiver)
    {
        nr = calloc(size, sizeof(int));
        MPI_Recv(nr, size, MPI_INT, (*q)(rank), 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if ((*f)(rank) != sender && to_all)
        {
            MPI_Send(nr, size, MPI_INT, (*f)(rank), 0, MPI_COMM_WORLD);
            if (afisare)
                printf("M(%d,%d)\n", rank, (*f)(rank));
        }
    }
    else
    {
        nr = calloc(size, sizeof(int));
        MPI_Recv(nr, size, MPI_INT, (*q)(rank), 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if ((*f)(rank) != sender)
        {
            MPI_Send(nr, size, MPI_INT, (*f)(rank), 0, MPI_COMM_WORLD);
            if (afisare)
                printf("M(%d,%d)\n", rank, (*f)(rank));
        }
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
    int *size = calloc(4, sizeof(int)); // cati workeri are fiecare cluster
    int **clusters_workers = calloc(4, sizeof(int *));
    int number_workers;
    int *workers;
    // int (*succesorf)(int) = succesor;
    // int (*previousf)(int) = previous;
    int er_com;
    int *v = calloc(4, sizeof(int));
    int nr_elements_v;
    if (rank < 4)
    {
        char *string = calloc(15, sizeof(char));
        strcat(string, "cluster");
        string[strlen(string)] = (char)('0' - 0 + rank);
        strcat(string, ".txt");
        FILE *f = fopen(string, "r");

        fscanf(f, "%d", &number_workers);
        workers = calloc(number_workers, sizeof(int)); // workerii lui rank
        for (int i = 0; i < number_workers; i++)
        {
            int aux;
            fscanf(f, "%d", &aux);
            workers[i] = aux;
        }
        er_com = atoi(argv[2]);
        if (er_com == 0)
        {
            for (int i = 0; i < 4; i++)
            {                                                 // trimitem dim
                int *aux = ring(i, rank, &number_workers, 1); // pt cand se rupe legatura o facem ring de la 0 la 1 si de la 1 la 0
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
            { // trimitem workers
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
        }
        else if (er_com == 1)
        {
            for (int i = 0; i < 3; i++)
            {
                v[i] = i + 1; // punem de la 1 spre 2, 3, 0 pentru a elimina din lista celor care asteapta de la stg la drp pe topologie
            }
            nr_elements_v = 4;
            if (rank == 0)
            {
                size[rank] = number_workers;
            }
            int *aux = ring_def(0, rank, size, 4, previous, succesor, 1, number_workers, 1, v, nr_elements_v); // parcurgem 0 -> 3 ->2 ->1
            if (rank == 1)
            {
                aux[rank] = number_workers;
                size = aux;
            }

            aux = ring_def(1, rank, aux, 4, succesor, previous, 0, 0, 0, v, nr_elements_v);
            if (aux)
            {
                size = aux;
            }

            clusters_workers[rank] = workers;
            int i = 0;
            while (i != -1) // trimitem pe inel din 0, apoi din 3, si apoi din 2
            {
                int *aux = ring_def(i, rank, workers, size[i], previous, succesor, 1, 0, 0, v, nr_elements_v);
                nr_elements_v--;

                if (aux && !clusters_workers[i])
                {
                    clusters_workers[i] = aux;
                }
                i = previous1(i);
            }

            i = 1;
            v[0] = 0;
            for (int i = 1; i < 4; i++)
            {
                v[i] = 4 - i;
            }
            nr_elements_v = 4;
            while (i != -1) // trimitem pe inel din 1, apoi din 2, si apoi din 3
            {
                int *aux = ring_def(i, rank, workers, size[i], succesor, previous, 0, 0, 0, v, nr_elements_v);
                nr_elements_v--;

                if (aux && !clusters_workers[i])
                {
                    clusters_workers[i] = aux;
                }

                i = succesor1(i);
            }
        }

        printare_topo(rank, size, clusters_workers);

        //  trimitem spre workers
        for (int i = 0; i < number_workers; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                MPI_Send(&size[j], 1, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
                if (afisare == 1)
                    printf("M(%d,%d)\n", rank, workers[i]);
            }
            for (int j = 0; j < 4; j++)
            {
                MPI_Send(clusters_workers[j], size[j], MPI_INT, workers[i], 0, MPI_COMM_WORLD);
                if (afisare == 1)
                    printf("M(%d,%d)\n", rank, workers[i]);
            }
        }
    }
    else
    { // receptionam topologia si in workers
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
    int rest_quant = -1, quant = -1;
    int N = atoi(argv[1]);
    if (rank == 0)
    {
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
        for (int i = 0; i < 3; i++)
        {
            v[i] = i + 1; // punem de la 1 spre 2, 3, 0 pentru a elimina din lista celor care asteapta de la stg la drp pe topologie
        }
        v[3] = 0;
        nr_elements_v = 4;
        int *aux = NULL;
        if (er_com == 0)
        {
            aux = ring(0, rank, quantity, 4);
        }
        else if (er_com == 1)
        {
            aux = ring_def(0, rank, quantity, 4, previous1, succesor1, 1, 0, 0, v, nr_elements_v);
        }
        if (aux)
        {
            quantity = aux;
        }
        int offset = 0;
        for (int i = 1; i < 4; i++)
        {
            offset += quantity[i - 1];
            int *aux = NULL;
            if (er_com == 0)
            {
                aux = send_btw_coordinators(0, rank, i, &(vector[offset]), quantity[i], succesor, previous, 1);
            }
            else if (er_com == 1)
            {
                aux = send_btw_coordinators(0, rank, i, &(vector[offset]), quantity[i], previous, succesor, 1);
            }
            if (aux)
            {
                vector = aux;
            }
        }

        quant = quantity[rank] / number_workers;
        int quant_sum = 0;

        for (int i = 0; i < number_workers - 1; i++)
        {
            quant_sum += quant;
        }
        rest_quant = quantity[rank] - quant_sum;
        for (int i = 0; i < number_workers - 1; i++)
        {

            MPI_Send(&quant, 1, MPI_INT, workers[i], workers[i], MPI_COMM_WORLD);
            if (afisare == 1)
                printf("M(%d,%d)\n", rank, workers[i]);
        }
        MPI_Send(&rest_quant, 1, MPI_INT, workers[number_workers - 1], workers[number_workers - 1], MPI_COMM_WORLD);
        offset = 0;
        if (afisare == 1)
            printf("M(%d,%d)\n", rank, workers[number_workers - 1]);

        for (int i = 0; i < number_workers - 1; i++)
        {
            MPI_Send(&(vector[offset]), quant, MPI_INT, workers[i], workers[i], MPI_COMM_WORLD);
            offset += quant;
            if (afisare == 1)
                printf("M(%d,%d)\n", rank, workers[i]);
        }
        MPI_Send(&(vector[offset]), rest_quant, MPI_INT, workers[number_workers - 1], workers[number_workers - 1], MPI_COMM_WORLD);
        if (afisare == 1)
            printf("M(%d,%d)\n", rank, workers[number_workers - 1]);
    }
    else
    {
        int quant;
        MPI_Status status;
        MPI_Recv(&quant, 1, MPI_INT, MPI_ANY_SOURCE, rank, MPI_COMM_WORLD, &status);
        vector = calloc(quant, sizeof(int));
        MPI_Recv(vector, quant, MPI_INT, MPI_ANY_SOURCE, rank, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        for (int i = 0; i < quant; i++)
        {
            vector[i] *= 5;
        }

        MPI_Send(vector, quant, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
        if (afisare == 1)
            printf("M(%d,%d)\n", rank, status.MPI_SOURCE);
    }

    if (rank < 4)
    {
        int offset = 0;
        for (int i = 0; i < number_workers - 1; i++)
        {
            MPI_Status status;
            MPI_Recv(&(vector[offset]), quant, MPI_INT, workers[i], 0, MPI_COMM_WORLD, &status);
            offset += quant;
        }
        MPI_Recv(&vector[offset], rest_quant, MPI_INT, workers[number_workers - 1], 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // primim vectorul de la workers


        int offset_recv = 0;
        if (er_com == 0)
        {
            for (int i = 1; i < 4; i++)
            {
                int *aux = NULL;

                aux = send_btw_coordinators(i, rank, 0, vector, quantity[i], previous, succesor, 1);

                offset_recv += quantity[i - 1];
                if (aux)
                {
                    memcpy(&(vector[offset_recv]), aux, quantity[i] * sizeof(int));
                }
            }
        }
        else if (er_com == 1)
        {
            v[0] = 0;
            for (int i = 1; i < 4; i++)
            {
                v[i] = 4 - i; // punem de la 1 spre 2, 3, 0 pentru a elimina din lista celor care asteapta de la stg la drp pe topologie
            }

            nr_elements_v = 4;
            int i = 1;
            //.........................................................................
            while (i != -1) // trimitem pe inel din 1, apoi din 2, si apoi din 3
            {

                int *aux = ring_def(i, rank, vector, quantity[i], succesor, previous, 0, 0, 0, v, nr_elements_v);
                nr_elements_v--;
                int sum = 0;
                for (int k = 0; k < i; k++)
                {
                    sum += quantity[k];
                }
                offset_recv = sum;
                
                if (aux && rank == 0)
                {

                    memcpy(&(vector[offset_recv]), aux, quantity[i] * sizeof(int));
                }

                i = succesor1(i);
            }
        }
    }

    if (rank == 0)
    {
        printf("Rezultat:");
        for (int i = 0; i < N; i++)
        {
            printf(" %d", vector[i]);
        }
        printf("\n");
    }

    MPI_Finalize();
}