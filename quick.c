#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
// #include <iostream>
#include <math.h>
// #include <chrono>
// #include <fstream>

const int MAX_SIZE = 500000;
/*
		Function      read_file
		   Input      file(ifstream object), int array of MAX_SIZE
		  Output      quantity of numbers in file
			 Job      copies numbers from the file to array, returns number of working elements of the array
*/

// int read_file(std::ifstream& input, int *result) {
// 	int i = 0;
// 	while (input >> result[i] && i < MAX_SIZE)
// 	{
// 		i++;
// 	}
// 	return i;
// }
// /*
// 		Function      write_file
// 		   Input      file(ofstream object), size of array, array
// 		  Output      none
// 		     Job      writes numbers from array into file
// */
// void write_file(std::ofstream& output, int arrSize, int *arr) {
// 	if (output.is_open()) {
// 		for (int i = 0; i < arrSize; i++)
// 			output << arr[i] << " ";
// 		output.close();
// 	}
// 	else
// 		std::cout << "Unable to open an output file.\n";
// }

/*
		Function      lomuto_partition
		   Input      array of int numbers, indices of sorting interval
		  Output	  index of pivot, which divides the array
			 Job      divides the array, so first part has numbers only < = pivot; moves pivot to its final place
*/

int lomuto_partition(int *A, int lo, int hi)
{
	int pivot = A[hi];
	int i = lo - 1;
	for (int j = lo; j < hi; j++)
	{
		if (A[j] <= pivot)
		{
			i++;
			int temp = A[i];
			A[i] = A[j];
			A[j] = temp;
		}
	}
	int temp = A[i + 1];
	A[i + 1] = A[hi];
	A[hi] = temp;
	return i + 1;
}

/*
			Function      partition
			   Input      array of int numbers, indices of sorting interval
			  Output	  index of element which divides the array
				 Job      aids to quicksort func
*/

int partition(int *A, int lo, int hi)
{
	int pivot = A[lo];
	int i = lo - 1;
	int j = hi + 1;
	while (1)
	{
		do
			i = i + 1;
		while (A[i] < pivot);

		do
			j = j - 1;
		while (A[j] > pivot);

		if (i >= j)
			return j;

		int temp = A[i];
		A[i] = A[j];
		A[j] = temp;
	}
}

/*
			Function      quicksort
			   Input      array of int numbers, indices of sorting interval
			  Output
				 Job      performs sequiential sorting
*/

void quicksort(int *A, int lo, int hi)
{
	if (lo < hi)
	{
		int p = partition(A, lo, hi);
		quicksort(A, lo, p);
		quicksort(A, p + 1, hi);
	}
}

/*
			Function      sort_recursive
			   Input      array of int numbers, size of array, rank of the current process, max rank, rankIndex
						  (satisfies 2^(rankIndex - 1) <= current_rank < 2^rankIndex))
			  Output      returns 0 after sequential quicksort
				 Job      performs partitioning recursively and sends a part IF there is an available fellow process, otherwise
						  sorts its own set sequentially.
*/

int sort_recursive(int *arr, int arrSize, int currProcRank, int maxRank, int rankIndex)
{
	MPI_Status status;

	// Calculate the rank of sharing process
	int shareProc = currProcRank + pow(2, rankIndex);
	rankIndex++;

	// If no process is available, sort sequentially and return
	if (shareProc > maxRank)
	{
		quicksort(arr, 0, arrSize - 1);
		return 0;
	}

	// Divide array in two parts with the pivot in between
	int j = 0;
	int pivotIndex;
	do
	{
		pivotIndex = lomuto_partition(arr, j, arrSize - 1);
		j++;
	} while (pivotIndex == j - 1);

	// Send partition based on size, sort the remaining partitions, receive sorted partition
	if (pivotIndex <= arrSize - pivotIndex)
	{
		MPI_Send(arr, pivotIndex - 1, MPI_INT, shareProc, pivotIndex, MPI_COMM_WORLD);
		sort_recursive((arr + pivotIndex + 1), (arrSize - pivotIndex - 1), currProcRank, maxRank, rankIndex);
		MPI_Recv(arr, pivotIndex - 1, MPI_INT, shareProc, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	}
	else
	{
		MPI_Send((arr + pivotIndex + 1), arrSize - pivotIndex - 1, MPI_INT, shareProc, pivotIndex + 1, MPI_COMM_WORLD);
		sort_recursive(arr, (pivotIndex + 1), currProcRank, maxRank, rankIndex);
		MPI_Recv((arr + pivotIndex + 1), arrSize - pivotIndex - 1, MPI_INT, shareProc, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	}
}

int main(int argc, char *argv[])
{

	int size, rank;
	double relogio, relogio_final;
	FILE *arq;
	int n, *numbers, num, i_vector = 0;
	arq = fopen(argv[1], "rt");
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	if (arq == NULL)
	{
		printf("Problemas na CRIACAO do arquivo\n");
	}
	else
	{

		fscanf(arq, "%d", &n);
		numbers = (int *)malloc(n * sizeof(int));

		while (fscanf(arq, "%d", &num) != EOF && n != i_vector)
		{
			numbers[i_vector] = num;

			i_vector++;
		}
	}
	// for (size_t i = 0; i < n; i++)
	// {
	// 	printf("%d ", numbers[i]);
	// }
	// if (argc != 4)
	// {
	//     std::cout << "Usage: enter input and output filenames, number of iterations to perform sorting.\n";
	//     return 1;
	// }
	// int iter_count = atoi(argv[3]);
	// This is a lame means to iterate through the program;
	// better to use a script (.bat, python, bash, whichever you like) to gather statistics
	for (int iter = 0; iter < 8; iter++)
	{
		// calculating index for a future search of a neighbor process (see README)
		int rankPower = 0;
		while (pow(2, rankPower) <= rank)
			rankPower++;

		if (rank == 0)
		{
			// auto start = std::chrono::high_resolution_clock::now();
			relogio = MPI_Wtime();
			// std::ifstream input(argv[1]);
			// if (!input) { std::cout << "Input file not found.\n"; return 1; }

			// I use malloc because sometimes MPI has problems with memory allocated with "new"
			// int *numbers = (int *)malloc(MAX_SIZE * sizeof(int));
			// int arraySize = read_file(input, numbers);
			// for (size_t i = 0; i < n; i++)
			// {
			// 	printf("%d ", numbers[i]);
			// }
			sort_recursive(numbers, n, rank, size - 1, rankPower);

			// std::ofstream output(argv[2]);
			// write_file(output, arraySize, numbers);
			// auto finish = std::chrono::high_resolution_clock::now();
			relogio_final = MPI_Wtime();
			// std::cout << double(std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count()) / 1000 << std::endl;
			printf("%f\n", relogio_final - relogio);
		}
		else
		{
			MPI_Status status;
			int subarray_size;
			MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			// Capturing size of the array to receive
			MPI_Get_count(&status, MPI_INT, &subarray_size);

			int source_process = status.MPI_SOURCE;
			int *subarray = (int *)malloc(subarray_size * sizeof(int));

			MPI_Recv(subarray, subarray_size, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			sort_recursive(subarray, subarray_size, rank, size - 1, rankPower);
			MPI_Send(subarray, subarray_size, MPI_INT, source_process, 0, MPI_COMM_WORLD);
		}
	}

	for (size_t i = 0; i < n; i++)
	{
		printf("%d ", numbers[i]);
	}

	printf("\n");
	MPI_Finalize();
	return 0;
}