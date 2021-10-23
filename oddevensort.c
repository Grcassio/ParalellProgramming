#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>   // for clock_t, clock(), CLOCKS_PER_SEC
#include <unistd.h> // for sleep()

#include <errno.h>
#include <unistd.h>
#include <signal.h>

#define TAG 0

int IncOrder(const void *e1, const void *e2)
{
    return (*((int *)e1) - *((int *)e2));
}

void CompareSplit(int nlocal, int *elmnts, int *relmnts, int nrelmnts, int *wspace, int keepsmall)
{
    int i, j, k;

    for (size_t i = 0; i < nlocal; i++)
        wspace[i] = elmnts[i];

    if (keepsmall)
    {

        for (i = j = k = 0; k < nlocal; k++)
        {

            if (j == nlocal || (i < nlocal && wspace[i] < relmnts[j]))
            {
                elmnts[k] = wspace[i];

                i++;
            }
            else if (j >= nrelmnts)
            {
                elmnts[k] = wspace[i];
            }
            else
            {
                elmnts[k] = relmnts[j];

                j++;
            }
        }
    }
    else
    {
        int x = nlocal - 1;
        if (nrelmnts > nlocal)
        {
            x = nrelmnts - 1;
        }

        for (i = k = j = nlocal - 1; k >= 0; k--)
        {
            if (j == -1 || (i >= 0 && wspace[i] >= relmnts[x]))
            {
                elmnts[k] = wspace[i];
                i--;
                x--;
            }
            else
            {
                elmnts[k] = relmnts[x];
                x--;
                j--;
            }
        }
    }
}

int main(int argc, char *argv[])
{
    int npes, idProc;
    int n;
    int *vec;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &npes);
    MPI_Comm_rank(MPI_COMM_WORLD, &idProc);

    // FILE *arq;
    // int n, *vec, num, i_vector = 0;
    // arq = fopen(argv[1], "rt");

    // if (idProc == 0)
    // {
    //     if (arq == NULL)
    //     {
    //         printf("Problemas na CRIACAO do arquivo\n");
    //     }
    //     else
    //     {

    //         fscanf(arq, "%d", &n);
    //         vec = (int *)malloc(n * sizeof(int));

    //         while (fscanf(arq, "%d", &num) != EOF && n != i_vector)
    //         {
    //             vec[i_vector] = num;

    //             i_vector++;
    //         }
    //     }
    // }
    if (idProc == 0)
    {
        FILE *arq;
        int n, num, i_vector = 0;
        arq = fopen(argv[1], "rt");

        if (arq == NULL)
        {
            printf("Problemas na CRIACAO do arquivo\n");
        }
        else
        {

            fscanf(arq, "%d", &n);
            vec = (int *)malloc(n * sizeof(int));

            while (fscanf(arq, "%d", &num) != EOF && n != i_vector)
            {
                vec[i_vector] = num;

                i_vector++;
            }
        }

        fclose(arq);
    }
    double time_spent = 0.0;

    double relogio, relogio_final;
    int oddrank;
    int evenrank;
    MPI_Status status;

    int resto = n % npes;
    int local_counts[npes], offsets[npes];
    int soma = 0;

    int nlocal = n / npes;

    int vetorLocal[nlocal];
    int recebeVec[nlocal];
    int wspace[nlocal];

    MPI_Scatter(vec, nlocal, MPI_INT, vetorLocal, nlocal, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    relogio = MPI_Wtime();
    qsort(vetorLocal, nlocal, sizeof(int), IncOrder);

    if (idProc % 2 == 0)
    {
        oddrank = idProc - 1;
        evenrank = idProc + 1;
    }
    else
    {
        oddrank = idProc + 1;
        evenrank = idProc - 1;
    }

    if (oddrank == -1 || oddrank == npes)
        oddrank = MPI_PROC_NULL;
    if (evenrank == -1 || evenrank == npes)
        evenrank = MPI_PROC_NULL;

    for (int i = 0; i < npes; i++)
    {
        if (i % 2 == 1)
        {
            MPI_Sendrecv(vetorLocal, nlocal, MPI_INT, oddrank, 1, recebeVec, nlocal, MPI_INT, oddrank, 1, MPI_COMM_WORLD, &status);
        }
        else
        {
            MPI_Sendrecv(vetorLocal, nlocal, MPI_INT, evenrank, 1, recebeVec, nlocal, MPI_INT, evenrank, 1, MPI_COMM_WORLD, &status);
        }

        CompareSplit(nlocal, vetorLocal, recebeVec, nlocal, wspace, idProc < status.MPI_SOURCE);
    }
    relogio_final = MPI_Wtime();
    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Gather(vetorLocal, nlocal, MPI_INT, vec, nlocal, MPI_INT, 0, MPI_COMM_WORLD);

    if (idProc == 0)
    {

        printf("%f\n", relogio_final - relogio);
    }

    // for (int i = 0; i < nlocal; i++)
    // {
    //     printf("%d idproc=%d ", vetorLocal[i], idProc);

    // }

    free(vec);
    // fclose(arq);
    MPI_Finalize();
}
