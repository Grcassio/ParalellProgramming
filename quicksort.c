#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// Function to swap two numbers
void swap(int *arr, int i, int j)
{
    int t = arr[i];
    arr[i] = arr[j];
    arr[j] = t;
}

// Function that performs the Quick Sort
// for an array arr[] starting from the
// index start and ending at index end
void quicksort(int *arr, int start, int end)
{
    int pivot, index;

    // Base Case
    if (end <= 1)
        return;

    // Pick pivot and swap with first
    // element Pivot is middle element
    pivot = arr[start + end / 2];
    swap(arr, start, start + end / 2);

    // Partitioning Steps
    index = start;

    // Iterate over the range [start, end]
    for (int i = start + 1; i < start + end; i++)
    {

        // Swap if the element is less
        // than the pivot element
        if (arr[i] < pivot)
        {
            index++;
            swap(arr, i, index);
        }
    }

    // Swap the pivot into place
    swap(arr, start, index);

    // Recursive Call for sorting
    // of quick sort function
    quicksort(arr, start, index - start);
    quicksort(arr, index + 1, start + end - index - 1);
}

// Function that merges the two arrays
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

        // Indices in bounds as i < n1
        // && j < n2
        else if (arr1[i] < arr2[j])
        {
            result[k] = arr1[i];
            i++;
        }

        // v2[j] <= v1[i]
        else
        {
            result[k] = arr2[j];
            j++;
        }
    }
    return result;
}

// Driver Code
int main(int argc, char *argv[])
{
    int n;
    int *vec = NULL;
    int chunk_size, own_chunk_size;
    int *chunk;

    double time_taken;
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

    // Blocks all process until reach this point
    MPI_Barrier(MPI_COMM_WORLD);

    // Starts Timer
    time_taken -= MPI_Wtime();

    // BroadCast the Size to all the
    // process from root process
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (n % npes == 0)
    {
        chunk_size = n / npes;
    }
    else
    {
        chunk_size = n / (npes - 1);
    }

    // Calculating total size of chunk
    // according to bits
    chunk = (int *)malloc(chunk_size * sizeof(int));

    // Scatter the chuck size data to all process
    MPI_Scatter(vec, chunk_size, MPI_INT, chunk, chunk_size, MPI_INT, 0, MPI_COMM_WORLD);
    free(vec);

    // Compute size of own chunk and
    // then sort them
    // using quick sort

    if (n >= chunk_size * (idProc + 1))
    {
        own_chunk_size = chunk_size;
    }
    else
    {
        own_chunk_size = n - chunk_size * idProc;
    }
   

    // Sorting array with quick sort for every
    // chunk as called by process
    quicksort(chunk, 0, own_chunk_size);
    
    for (int step = 1; step < npes; step = 2 * step)
    {
      
        if (idProc % (2 * step) != 0)
        {
            printf("idProc: %d step: %d\n", idProc,step );
            
            MPI_Send(chunk, own_chunk_size, MPI_INT, idProc - step, 0, MPI_COMM_WORLD);
            break;
        }

        if (idProc + step < npes)
        {
            // int received_chunk_size = (n >= chunk_size * (idProc + 2 * step))
            //                               ? (chunk_size * step)
            //                               : (n - chunk_size * (idProc + step));
            
            
            int received_chunk_size;
            if (n >= chunk_size * (idProc + 2 * step))
            {
                received_chunk_size = chunk_size * step;
            }
            else
            {
                received_chunk_size = n - chunk_size * (idProc + step);
            }

            int *chunk_received;
            chunk_received = (int *)malloc(received_chunk_size * sizeof(int));
            MPI_Recv(chunk_received, received_chunk_size, MPI_INT, idProc + step, 0, MPI_COMM_WORLD, &status);

            vec = merge(chunk, own_chunk_size, chunk_received, received_chunk_size);

            free(chunk);
            free(chunk_received);
            chunk = vec;
            own_chunk_size = own_chunk_size + received_chunk_size;
        }
    }

    // Stop the timer
    time_taken += MPI_Wtime();

    // Opening the other file as taken form input
    // and writing it to the file and giving it
    // as the output
    if (idProc == 0)
    {

        for (int i = 0; i < n; i++)
        {
            printf("%d  ", chunk[i]);
        }
        printf("\n");
    }

    MPI_Finalize();
    return 0;
}