#include <stdio.h>
#include <math.h>
#include <mpi.h>

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

        if (rank == 0){
            for (int dest = 1; dest < numprocs; dest++){
                MPI_Send(&n, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
            }
        }else{
            MPI_Recv(&n, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        if (n == 0) break;

        h   = 1.0 / (double) n;
        sum = 0.0;

        for (i = rank+1; i <= n; i += numprocs) {
            x = h * ((double)i - 0.5);
            sum += 4.0 / (1.0 + x*x);
        }

        if (rank == 0){
            for (int source = 1; source < numprocs; source++){
                double received_sum;
                MPI_Recv(&received_sum, 1, MPI_DOUBLE, source, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                sum += received_sum;
            }
        }else{
            MPI_Send(&sum, 1, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD);
        }

        if (rank == 0) {
            pi = h * sum;
            printf("pi is approximately %.16f, Error is %.16f\n", pi, fabs(pi - PI25DT));
        }
    }

    MPI_Finalize ();

    return 0;
}

// más de 1 segundo con el código original
// 10000000000