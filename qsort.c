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

int main(int argc, char *argv[])
{
    int npes, idProc;
   
    int *vec;

    FILE *arq;
    int n, num, i_vector = 0;
    arq = fopen(argv[1], "rt");
    double relogio, relogio_final;
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
    relogio = MPI_Wtime();
    qsort(vec, n, sizeof(int), IncOrder);
    relogio_final = MPI_Wtime();


    for (size_t i = 0; i < n; i++)
    {
        printf("%d ",vec[i]);
    }
    
    printf("%f\n", relogio_final - relogio);

    free(vec);
}
