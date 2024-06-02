#include <stdio.h>
#include <math.h>
#include <mpi.h>

int MPI_BinomialColectivaRec(void * buff , int count , MPI_Datatype datatype ,
                          int root , MPI_Comm comm, int iter){
    int numprocs, rank;
    MPI_Comm_size (comm, &numprocs);
    MPI_Comm_rank (comm, &rank);

    if(root == rank){
        for (int i = iter;; i++) {
            int dest = rank + pow(2,i);
            if (dest < numprocs) {
                MPI_Send(buff, count, datatype, dest, 0, comm);
            }else{
                break;
            }
        }
    }else{
        MPI_Recv(buff, count, datatype, MPI_ANY_SOURCE, 0, comm, MPI_STATUS_IGNORE);
        MPI_BinomialColectivaRec(buff, count, datatype, rank, comm, log2(rank) + 1);
    }
}

int MPI_BinomialColectiva(void * buff , int count , MPI_Datatype datatype ,
                          int root , MPI_Comm comm){

    MPI_BinomialColectivaRec(buff, count, datatype, root, comm, 0);

    MPI_Barrier(comm);

    return MPI_SUCCESS;
}

int MPI_FlattreeColectiva(void * buff , void * recvbuff , int count ,
                          MPI_Datatype datatype , MPI_Op op , int root , MPI_Comm comm ){

    int numprocs, rank;
    MPI_Comm_size (comm, &numprocs);
    MPI_Comm_rank (comm, &rank);

    if (rank == root){
        double total = *(double *)buff;
        for (int i = 1; i < numprocs; i++){
            double sum;
            MPI_Recv(&sum, 1, MPI_DOUBLE, MPI_ANY_SOURCE, 1, comm, MPI_STATUS_IGNORE);
            total += sum;
        }
        *(double *)recvbuff = total;
    }else{
        MPI_Send(buff, 1, MPI_DOUBLE, root, 1, comm);
    }

    /*
    double total = 0;

    total += *(double *)buff;

    MPI_Barrier(comm);

    *(double *)recvbuff = total;
    */

    return MPI_SUCCESS;
}

int main(int argc, char *argv[])
{
    int numprocs, rank;
    int i, done = 0, n;
    double PI25DT = 3.141592653589793238462643;
    double pi, h, sum, x, local_sum;

    MPI_Init (&argc , &argv);
    MPI_Comm_size (MPI_COMM_WORLD , &numprocs);
    MPI_Comm_rank (MPI_COMM_WORLD , &rank);

    while (!done)
    {
        if (rank == 0) {
            printf("Enter the number of intervals: (0 quits) \n");
            scanf("%d",&n);
        }

        //MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_BinomialColectiva(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

        if (n == 0) break;

        h   = 1.0 / (double) n;
        sum = 0.0;

        for (i = rank+1; i <= n; i += numprocs) {
            x = h * ((double)i - 0.5);
            sum += 4.0 / (1.0 + x*x);
        }

        //MPI_Reduce(&sum, &pi, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
        MPI_FlattreeColectiva(&sum, &pi, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

        if (rank == 0) {
            pi *= h;
            printf("pi is approximately %.16f, Error is %.16f\n", pi, fabs(pi - PI25DT));
        }
    }

    MPI_Finalize ();

    return 0;
}

// más de 1 segundo con el código original
// 10000000000