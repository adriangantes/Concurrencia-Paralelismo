#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <mpi.h>

#define DEBUG 1

#define N 113

int main(int argc, char *argv[] ) {

  int i, j;
  int tam_row, tam_res;
  float vector[N]; //El tamaño del vector por el que multiplicamos siempre es el mismo
  struct timeval  tv1, tv2, ts1, ts2, tg1, tg2;
  int rank, numprocs;

  MPI_Init (&argc , &argv);
  MPI_Comm_size (MPI_COMM_WORLD , &numprocs);
  MPI_Comm_rank (MPI_COMM_WORLD , &rank);

  //CALCULAMOS
  int tam = N / numprocs; //Numero de filas que tiene cada proceso
  int tam_add = N % numprocs; //Número de filas sobrantes

  //ASIGNAMOS
  if (rank == 0){ //El proceso 0 tiene toda la matriz
      tam_row = N;
      tam_res = N;
  }else{ //Mientras que del resto queremos todas las columnas pero solo de x flias
      tam_row = tam;
      tam_res = tam;
  }

  //CREAMOS
  float *matrix = (float *) malloc(tam_row * N * sizeof(float)); //Reservamos memoria a la matriz
  float *result = (float *) malloc(tam_res * sizeof(float)); // Reservar memoria para el resultado

  if (matrix == NULL || result == NULL) {
      printf("Error al asignar memoria.\n");
      exit(0);
  }

  //INICIALIZAMOS MATRIZ Y VECTOR
  if (rank == 0){
      for(i=0;i<N;i++) {
          vector[i] = i;
          for(j=0;j<N;j++) {
              *(matrix + i*N + j) = i+j;
          }
      }
  }/*else{
      for(i=0;i<N;i++) {
          vector[i] = i;
      }
  }*/

  //Tiempo de comunicación para enviar
  gettimeofday(&ts1, NULL);
  MPI_Scatter(matrix, tam * N, MPI_FLOAT, matrix, tam * N, MPI_FLOAT, 0, MPI_COMM_WORLD); //El tamaño es una matriz
  MPI_Bcast(vector, N, MPI_FLOAT, 0, MPI_COMM_WORLD);
  gettimeofday(&ts2, NULL);

  //Tiempo de computación
  gettimeofday(&tv1, NULL);

  for(i = 0; i < tam; i++) {
      result[i] = 0;
      for(j = 0; j < N; j++) {
          result[i] += *(matrix + i * N + j) * vector[j];
      }
  }

  if (rank == 0){
      for(i = N - tam_add; i < N; i++) {
          result[i] = 0;
          for(j = 0; j < N; j++) {
              result[i] += *(matrix + i * N + j) * vector[j];
          }
      }
  }

  gettimeofday(&tv2, NULL);

  //Tiempo de comunicación para recibir
  gettimeofday(&tg1, NULL);
  MPI_Gather(result, tam, MPI_FLOAT, result, tam, MPI_FLOAT, 0, MPI_COMM_WORLD); //El tamaño que manda es un vector ya que se multiplica una matriz por un vector
  gettimeofday(&tg2, NULL);


  int microseconds_comun = ((ts2.tv_usec - ts1.tv_usec)+ 1000000 * (ts2.tv_sec - ts1.tv_sec)) + ((tg2.tv_usec - tg1.tv_usec)+ 1000000 * (tg2.tv_sec - tg1.tv_sec));
  int microseconds_compu = (tv2.tv_usec - tv1.tv_usec)+ 1000000 * (tv2.tv_sec - tv1.tv_sec);

  MPI_Barrier(MPI_COMM_WORLD);

  //Mostramos el resultado
  if (rank == 0) {
      for (i = 0; i < N; i++) {
          printf("%f\t", result[i]);
      }
  }

  MPI_Barrier(MPI_COMM_WORLD);

  int total_comun, total_compu;
  MPI_Reduce(&microseconds_comun, &total_comun, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&microseconds_compu, &total_compu, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

  if (rank == 0){
      printf ("\n\nMedia comunicacion = %lf segundos\nMedia computacion = %lf segundos\n", (double) (total_comun/numprocs)/1E6, (double) (total_compu/numprocs)/1E6);
  }

  free(matrix);
  free(result);

  MPI_Finalize();

  return 0;
}

