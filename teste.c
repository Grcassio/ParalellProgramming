#include <stdio.h>
#include <stdlib.h>
int main(int argc, char const *argv[])
{

    FILE *arq;
    int n, *vec, num, i_vector = 0, *vec2;
    arq = fopen(argv[1], "w+");

    // if (idProc == 0)
    // {
    if (arq == NULL)
    {
        printf("Problemas na CRIACAO do arquivo\n");
    }
    else
    {
        fprintf(arq, "%d\n", 1000000);
        //  fprintf(arq, "%d ", 1);
        //   fprintf(arq, "%d", 2);
        int i = 1000000;
        while (i>0)
        {
            // int r = rand() % 1000000;
             fprintf(arq, "%d ", i);
             i--;
        }
    }
    fclose(arq);
    return 0;
}