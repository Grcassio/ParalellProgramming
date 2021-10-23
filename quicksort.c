#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

void swap(int *arr, int i, int j)
{
    int t = arr[i];
    arr[i] = arr[j];
    arr[j] = t;
}

void quicksort(int *arr, int start, int end)
{
    int pivot, index;

    if (end <= 1)
        return;

    pivot = arr[start + end / 2];
    swap(arr, start, start + end / 2);

    index = start;

    for (int i = start + 1; i < start + end; i++)
    {

        if (arr[i] < pivot)
        {
            index++;
            swap(arr, i, index);
        }
    }

    swap(arr, start, index);

    quicksort(arr, start, index - start);
    quicksort(arr, index + 1, start + end - index - 1);
}

int *merge(int *arr1, int n1, int *arr2, int n2)
{

    int *result = (int *)malloc((n1 + n2) * sizeof(int));
    int i = 0;
    int j = 0;
    int k;

    for (k = 0; k < n1 + n2; k++)
    {
        if (i >= n1)
        {
            result[k] = arr2[j];
            j++;
        }
        else if (j >= n2)
        {
            result[k] = arr1[i];
            i++;
        }

        else if (arr1[i] < arr2[j])
        {
            result[k] = arr1[i];
            i++;
        }

        else
        {
            result[k] = arr2[j];
            j++;
        }
    }
    return result;
}

int main(int argc, char *argv[])
{
    int n;
    int *vec = NULL;
    int n_chunks, tam_chunk;
    int *chunk;

    double relogio,relogio_final;
    MPI_Status status;

    int npes, idProc;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &npes);
    MPI_Comm_rank(MPI_COMM_WORLD, &idProc);

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

    MPI_Barrier(MPI_COMM_WORLD);

    relogio = MPI_Wtime();

    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (n % npes == 0)
    {

        n_chunks = n / npes;
    }
    else
    {

        n_chunks = n / (npes - 1);
    }

    chunk = (int *)malloc(n_chunks * sizeof(int));

    MPI_Scatter(vec, n_chunks, MPI_INT, chunk, n_chunks, MPI_INT, 0, MPI_COMM_WORLD);
    free(vec);

    if (n >= n_chunks * (idProc + 1))
    {

        tam_chunk = n_chunks;
    }
    else
    {

        tam_chunk = n - n_chunks * idProc;
    }

    quicksort(chunk, 0, tam_chunk);

    for (int step = 1; step < npes; step = 2 * step)
    {

        if (idProc % (2 * step) != 0)
        {

            MPI_Send(chunk, tam_chunk, MPI_INT, idProc - step, 0, MPI_COMM_WORLD);
            break;
        }

        if (idProc + step < npes)
        {

            int tam_chunk_recebido;
            if (n >= n_chunks * (idProc + 2 * step))
            {
                tam_chunk_recebido = n_chunks * step;
            }
            else
            {
                tam_chunk_recebido = n - n_chunks * (idProc + step);
            }

            int *chunk_received;
            chunk_received = (int *)malloc(tam_chunk_recebido * sizeof(int));
            

            MPI_Recv(chunk_received, tam_chunk_recebido, MPI_INT, idProc + step, 0, MPI_COMM_WORLD, &status);

            vec = merge(chunk, tam_chunk, chunk_received, tam_chunk_recebido);

            free(chunk);
            free(chunk_received);
            chunk = vec;
            tam_chunk = tam_chunk + tam_chunk_recebido;
        }
    }

    relogio_final = MPI_Wtime();

    if (idProc == 0)
    {

        // for (int i = 0; i < n; i++)
        // {
        //     printf("%d  ", chunk[i]);
        // }
        // printf("\n");
        printf("%f\n", relogio_final - relogio);
    }

    MPI_Finalize();
    return 0;
}