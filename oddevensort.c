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

    // printf("elmnts: ");
    // for (size_t i = 0; i < nlocal; i++)
    // {
    //     printf("%d ", elmnts[i]);
    // }

    // // printf("nrelmnts:%d ",nrelmnts);
    //  printf("relmnts: ");
    // for (size_t i = 0; i < nrelmnts; i++)
    // {
    //     printf("%d ", relmnts[i]);
    // }
    //  printf("\n");

    for (size_t i = 0; i < nlocal; i++)
        wspace[i] = elmnts[i]; /* Copy the elmnts array into the wspace array */
    // for (size_t i = 0; i < nrelmnts; i++)
    //     wspace[i+nlocal] = relmnts[i]; /* Copy the elmnts array into the wspace array */

    // qsort(wspace, nlocal + nrelmnts, sizeof(int), IncOrder);
    // printf("wspace: ");
    // for (size_t i = 0; i < nlocal; i++)
    // {
    //     printf("%d ", wspace[i]);
    // }
    // printf("\n");
    if (keepsmall)
    { /* Keep the nlocal smaller elements */
        // printf("+ ");
        for (i = j = k = 0; k < nlocal; k++)
        {

            if (j == nlocal || (i < nlocal && wspace[i] < relmnts[j]))
            {
                elmnts[k] = wspace[i];
                // printf("OLA\n");
                i++;
            }
            else if (j >= nrelmnts)
            {
                elmnts[k] = wspace[i];
                // printf("OLA:%d k:%d\n", wspace[i], k);
            }
            else
            {
                elmnts[k] = relmnts[j];
                //  printf("oi ");
                j++;
            }
        }
    }
    else
    { /* Keep the nlocal larger elements */
        // printf("* ");
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
    // printf("-passou->    ");
    // printf("Elmnts: ");
    // for (size_t i = 0; i < nlocal; i++)
    // {
    //     printf("%d ", elmnts[i]);
    // }

    // printf("Relmnts: ");
    // for (size_t i = 0; i < nrelmnts; i++)
    // {
    //     printf("%d ", relmnts[i]);
    // }
    // printf("\n");
}

int main(int argc, char *argv[])
{
    int npes, idProc;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &npes);
    MPI_Comm_rank(MPI_COMM_WORLD, &idProc);

    // int vec[8] = {8, 7, 6, 5, 4, 3, 2, 1};
    // int n = 8;
    FILE *arq;
    int n, *vec, num, i_vector = 0;
    arq = fopen(argv[1], "rt");

    // if (idProc == 0)
    // {
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
    // }
    double time_spent = 0.0;

    clock_t begin = clock();
    double relogio, relogio_final;
    int oddrank;
    int evenrank;
    MPI_Status status;

    // Trata o caso onde n não é divisivel por p
    int resto = n % npes;
    int local_counts[npes], offsets[npes];
    int soma = 0;

    // for (int i = 0; i < npes; i++)
    // {
    //     local_counts[i] = n / npes;

    //     if (resto > 0)
    //     {
    //         local_counts[i] += 1;
    //         resto--;
    //     }

    //     offsets[i] = soma;
    //     soma = soma + local_counts[i];
    // }

    // int nlocal = local_counts[idProc];

    // int vetorLocal[local_counts[idProc] + 10];
    // int recebeVec[local_counts[idProc] + 10];
    // int wspace[n];

    int nlocal = n / npes;

    int vetorLocal[nlocal];
    int recebeVec[nlocal];
    int wspace[nlocal];
    // MPI_Scatterv(vec, local_counts, offsets, MPI_INT, vetorLocal, local_counts[idProc], MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(vec, nlocal, MPI_INT, vetorLocal, nlocal, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    relogio = MPI_Wtime();
    qsort(vetorLocal, nlocal, sizeof(int), IncOrder);
    // printf("->%d: ", idProc);
    // for (int i = 0; i < local_counts[idProc]; i++)
    // {
    //     printf("%d ", vetorLocal[i]);
    // }
    // printf("\n");
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

    // printf("idProc:");
    // printf("%d oddrank:%d evenrank:%d\n", idProc, oddrank, evenrank);
    // printf("idProc:%d \n",idProc);
    // for (size_t i = 0; i < npes; i++)
    // {
    //     printf("%d ",local_counts[i]);
    // }
    //  printf("\n");

    // int y = npes;
    // if (npes == n)
    // {
    //     y;
    // }

    // for (int i = 0; i < npes - 1; i++)
    // {

    //     // printf("idProc:%d\n",idProc);
    //     // printf("vetorLocal:");
    //     // for (size_t i = 0; i < local_counts[idProc]; i++)
    //     // {
    //     //     printf("%d ", vetorLocal[i]);
    //     // }
    //     // printf("\n");
    //     // printf("local_counts:%d\n", local_counts[idProc]);
    //     // printf("evenrank:%d\n", evenrank);
    //     // printf("recebeVec: ");
    //     // for (size_t i = 0; i < local_counts[evenrank]; i++)
    //     // {
    //     //     printf("%d ", recebeVec[i]);
    //     // }
    //     // printf("\n");

    //     if (i % 2 == 1)
    //     {
    //         // int x = local_counts[oddrank];
    //         // if (oddrank < 0)
    //         // {

    //         //     x = local_counts[idProc];
    //         // }
    //         // printf("%d\n",i);
    //         // printf("->idProc:%d nrelemnts:%d oddrank:%d\n", idProc,local_counts[x],x);
    //         MPI_Sendrecv(vetorLocal, local_counts[idProc], MPI_INT, oddrank, 1, recebeVec, local_counts[oddrank], MPI_INT, oddrank, 1, MPI_COMM_WORLD, &status);
    //         CompareSplit(local_counts[idProc], vetorLocal, recebeVec, x, wspace, idProc < status.MPI_SOURCE);
    //     }
    //     else
    //     {
    //         // printf("%d\n",i);
    //         // printf("->idProc:%d nrelemnts:%d\n", idProc,local_counts[evenrank]);

    //         MPI_Sendrecv(vetorLocal, local_counts[idProc], MPI_INT, evenrank, 1, recebeVec, local_counts[evenrank], MPI_INT, evenrank, 1, MPI_COMM_WORLD, &status);
    //         //printf("idProc:%d nrelemnts:%d\n", idProc,local_counts[evenrank]);
    //         CompareSplit(local_counts[idProc], vetorLocal, recebeVec, local_counts[evenrank], wspace, idProc < status.MPI_SOURCE);
    //     }
    //     // printf("-------------------------------------------\n");

    //     // printf("idProc:%d\n", idProc);
    //     // printf("vetorLocal: ");
    //     // for (size_t i = 0; i < local_counts[idProc]; i++)
    //     // {
    //     //     printf("%d ", vetorLocal[i]);
    //     // }
    //     // printf("\n");
    //     // printf("recebeVec: ");
    //     // for (size_t i = 0; i < local_counts[evenrank]; i++)
    //     // {
    //     //     printf("%d ", recebeVec[i]);
    //     // }
    //     // printf("\n");
    //     // printf("-------------------------------------------\n");

    //     // CompareSplit(local_counts[idProc], vetorLocal, recebeVec, wspace, idProc < status.MPI_SOURCE);
    // }
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

    clock_t end = clock();
    time_spent += (double)(end - begin) / CLOCKS_PER_SEC;

    // printf("The elapsed time is %f seconds", time_spent);
    if (idProc == 0)
    {

        printf("RESULTADO:\n");

        // for (int i = 0; i < n; i++)
        // {
        //     printf("%d ", vec[i]);
        // }
        // printf("\n");
        printf("The elapsed time is %f seconds\n", time_spent);
        printf("Relogio %f seconds\n", relogio_final-relogio);
    }

    // for (int i = 0; i < nlocal; i++)
    // {
    //     printf("%d idproc=%d ", vetorLocal[i], idProc);

    // }

    free(vec);
    fclose(arq);
    MPI_Finalize();
}

// #2
// The elapsed time is 0.698566 seconds
// Relogio 0.676104 seconds

// #4
// The elapsed time is 0.505575 seconds
// Relogio 0.347731 seconds

// #8
// The elapsed time is 0.452401 seconds
// Relogio 0.488273 seconds

// #16
// The elapsed time is 0.466413 seconds
// Relogio 0.694515 seconds
