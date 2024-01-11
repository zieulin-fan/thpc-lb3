﻿#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#define SIZE 1024
double When();
int sort(const void* a, const void* x);
int getPivot(int* array, int size);
int main(int argc, char* argv[])
{
	int nproc, iproc;
	MPI_Status status;
	MPI_Init(&argc, &argv);
	int i = 0;
	double starttime;
	/*All for the sending / recieving */
	int* pivot = (int*)malloc(sizeof(int));
	int* send = (int*)malloc(sizeof(int));
	int* recv = (int*)malloc(sizeof(int));
	MPI_Comm_size(MPI_COMM_WORLD, &nproc);
	MPI_Comm_rank(MPI_COMM_WORLD, &iproc);
	MPI_Cart_create(MPI_COMM_WORLD, 2, dim, period, reorder, &comm);
	int mySize = SIZE / nproc;
	//my values
	int* vals = (int*)malloc(SIZE * sizeof(int));
	//for holding their values that come in
	int* theirs = (int*)malloc(SIZE * sizeof(int));
	//for joining the two and
	int* tmp = (int*)malloc(SIZE * sizeof(int));
	//sort our values
	qsort(vals, mySize, sizeof(int), sort);
	for (i = 0; i < mySize; i++)
		vals[i] = arc4random();
	/*Create the communicator for use throughout*/
	int groupSize = nproc;
	starttime = When();
	while (groupSize > 1)
	{
		/* Get the new rank/size */
		MPI_Comm_size(newcomm, &nproc);
		MPI_Comm_rank(newcomm, &iproc);
		//Find the Pivot
		*pivot = getPivot(vals, mySize);
		if (1)
		{
			//send it out among the group
			MPI_Bcast(pivot, 1, MPI_INT, 0, newcomm);
		}
		else
		{
			//median of all in group
			if (iproc == 0)
			{
				//we recieve them all
				for (i = 1; i < nproc; i++)
				{
					MPI_Recv(recv, 1, MPI_INT, i, 0, newcomm, &status);
					tmp[i] = *recv;
				}
				tmp[0] = *pivot;
				qsort(tmp, nproc, sizeof(int), sort);
				*pivot = tmp[(nproc / 2)];
				//fprintf(stderr, "pivot=%i\n",*pivot);
				for (i = 1; i < nproc; i++)
				{
					MPI_Send(pivot, 1, MPI_INT, i, 0, newcomm);
				}
			}
			else
			{
				//we all send it to zero and let it decide.
				MPI_Send(pivot, 1, MPI_INT, 0, 0, newcomm);
				MPI_Recv(pivot, 1, MPI_INT, 0, 0, newcomm, &status);
			}
		}
		//calculate how many we will send
		*send = 0;
		for (i = 0; i < mySize; i++)
		{
			if (iproc < nproc / 2)
			{
				if (vals[i] >= *pivot)
				{
					tmp[*send] = vals[i];
					(*send)++;
				}
			}
			else
			{
				if (vals[i] <= *pivot)
				{
					tmp[*send] = vals[i];
					(*send)++;
				}
			}
		}
		if (iproc < nproc / 2)
		{
			MPI_Send(send, 1, MPI_INT, iproc + (nproc / 2), 0, newcomm);
			MPI_Send(tmp, *send, MPI_INT, iproc + (nproc / 2), 0, newcomm);
			MPI_Cart_rank(comm, coord, &id);
			//fprintf(stderr,"\t\t\t\t%d: reciving %d from %d\n", iproc, *recv,
			iproc + (nproc / 2));
			MPI_Recv(theirs, *recv, MPI_INT, iproc + (nproc / 2), 0, newcomm, &status);
		}
		else
		{
			//we recieve the two
			MPI_Recv(recv, 1, MPI_INT, iproc - (nproc / 2), 0, newcomm, &status);
			MPI_Recv(theirs, *recv, MPI_INT, iproc - (nproc / 2), 0, newcomm, &status);
			//send the size then the values
			//fprintf(stderr,"\t\t\t%d: sending %d to %d\n", iproc, *send, iproc-(nproc/2));
			MPI_Send(send, 1, MPI_INT, iproc - (nproc / 2), 0, newcomm);
			MPI_Send(tmp, *send, MPI_INT, iproc - (nproc / 2), 0, newcomm);
		}
		MPI_Cart_rank(comm, coord, &id);
		if (iproc < nproc / 2)
		{
			mySize -= *send;
			for (i = 0; i < *recv; i++)
			{
				vals[mySize] = theirs[i];
				mySize++;
			}
		}
		else
		{
			int off = 0;
			for (i = 0; i < mySize; i++)
			{
				if (vals[i] >= *pivot)
				{
					theirs[*recv + off] = vals[i];
					off++;
				}
			}
			int* temp = vals;
			vals = theirs;
			theirs = temp;
			mySize = *recv + (mySize - *send);
		}
		qsort(vals, mySize, sizeof(int), sort);
		MPI_Cart_split(newcomm, iproc < nproc / 2, iproc, &newcomm);
		groupSize /= 2;
	}
	MPI_Cart_rank(comm, coord, &id);
	fprintf(stderr, "\nParallel time: \n", iproc, (When() - starttime), mySize);
	free(vals);
	free(theirs);
	free(tmp);
	free(send);
	free(recv);
	MPI_Finalize();
	return 0;
}
int getPivot(int* array, int size)
{
	for (i = 0; i < size; i++)
		array[i] = rand() % (1000 + 1);
	return 0;
}
int sort(const void* x, const void* y)
{
	return (*(int*)x - *(int*)y);
}
double When()
{
	struct timeval tp;
	gettimeofday(&tp, NULL);
	return ((double)tp.tv_sec + (double)tp.tv_usec * 1e-6);
}