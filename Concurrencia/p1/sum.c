#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include "options.h"

struct nums {
    long *increase;
    long *decrease;
    long total;
    int size;
};

struct args {
    int thread_num;		// application defined thread #
    long *iterations;	// number of operations
    struct nums *nums;	// pointer to the counters (shared with other threads)
    pthread_mutex_t *mutex_it;
    pthread_mutex_t *mutex_inc;
    pthread_mutex_t *mutex_dec;
};

struct thread_info {
    pthread_t    id;    // id returned by pthread_create()
    struct args *args;  // pointer to the arguments
};


// Threads run on this function
void *decrease_increase(void *ptr)
{
    struct args *args = ptr;
    struct nums *n = args->nums;
    int i, d;

    while(1) {
        pthread_mutex_lock(args->mutex_it);

        if (*args->iterations <= 0){
            pthread_mutex_unlock(args->mutex_it);
            break;
        }

        (*args->iterations)--;

        pthread_mutex_unlock(args->mutex_it);

        while (1) {
            i = rand() % args->nums->size;
            d = rand() % args->nums->size;

            pthread_mutex_lock(&args->mutex_inc[i]);

            if (pthread_mutex_trylock(&args->mutex_dec[d]) == 0) {
                n->increase[i]++;
                n->decrease[d]--;
                pthread_mutex_unlock(&args->mutex_inc[i]);
                pthread_mutex_unlock(&args->mutex_dec[d]);
                break;
            }

            pthread_mutex_unlock(&args->mutex_inc[i]);
        }
    }

    return NULL;
}

void *increase_fun (void *ptr)
{
    struct args *args = ptr;
    struct nums *n = args->nums;
    int i1, i2;

    while(1) {
        pthread_mutex_lock(args->mutex_it);

        if (*args->iterations <= 0){
            pthread_mutex_unlock(args->mutex_it);
            break;
        }

        (*args->iterations)--;

        pthread_mutex_unlock(args->mutex_it);

        while (1) {
            i1 = rand() % args->nums->size;
            i2 = rand() % args->nums->size;

            while(i1 == i2){
                i2 = rand() % args->nums->size;
            }

            pthread_mutex_lock(&args->mutex_inc[i1]);

            if (pthread_mutex_trylock(&args->mutex_inc[i2]) == 0) {
                n->increase[i1]++;
                n->increase[i2]--;
                pthread_mutex_unlock(&args->mutex_inc[i1]);
                pthread_mutex_unlock(&args->mutex_inc[i2]);
                break;
            }

            pthread_mutex_unlock(&args->mutex_inc[i1]);
        }
    }

    return NULL;
}

// start opt.num_threads threads running on decrease_incresase
struct thread_info *start_threads(struct options opt, struct nums *nums,
                                  pthread_mutex_t *mutex_it, long *iter, pthread_mutex_t *mutex_inc,
                                  pthread_mutex_t *mutex_dec, struct args *args)
{
    int i;
    struct thread_info *threads;

    printf("creating %d threads\n", opt.num_threads);
    threads = malloc(sizeof(struct thread_info) * opt.num_threads * 2);

    if (threads == NULL) {
        printf("Not enough memory\n");
        exit(1);
    }

    // Create num_thread threads running decrease_increase
    for (i = 0; i < opt.num_threads; i++) {
        threads[i].args = malloc(sizeof(struct args));

        threads[i].args->thread_num = i;
        threads[i].args->nums       = nums;
        threads[i].args->iterations = iter;
        threads[i].args->mutex_it = mutex_it;
        threads[i].args->mutex_inc = mutex_inc;
        threads[i].args->mutex_dec = mutex_dec;

        if (0 != pthread_create(&threads[i].id, NULL, decrease_increase, threads[i].args)) {
            printf("Could not create thread #%d", i);
            exit(1);
        }
    }

    for (; i < opt.num_threads * 2; i++) {

        args[i - opt.num_threads].thread_num = i;
        args[i - opt.num_threads].nums       = nums;
        args[i - opt.num_threads].iterations = iter;
        args[i - opt.num_threads].mutex_it = mutex_it;
        args[i - opt.num_threads].mutex_inc = mutex_inc;

        threads[i].args = &args[i - opt.num_threads];

        if (0 != pthread_create(&threads[i].id, NULL, increase_fun, &args[i - opt.num_threads])) {
            printf("Could not create thread #%d", i);
            exit(1);
        }
    }

    return threads;
}

void print_totals(struct nums *nums)
{
    long total;
    long total_inc = 0;
    long total_dec = 0;

    printf ("\n");

    for(int i = 0; i < nums->size; i++){
        printf ("Increase %d = %ld\t", i, nums->increase[i]);
        printf ("Decrease %d = %ld\n", i, nums->decrease[i]);
        total_inc += nums->increase[i];
        total_dec += nums->decrease[i];
    }

    total = total_dec + total_inc;

    printf ("\nTotal Increase = %ld\n", total_inc);
    printf ("Total Decrease = %ld\n", total_dec);
    printf ("Suma total = %ld\n", total);
    printf ("\nTotal = %ld\n\n", nums->total * nums->size);

    if (total == nums->total * nums->size){
        printf("Final: SUCCESS\n");
    }else{
        printf("Final: ERROR\n");
    }
}

// wait for all threads to finish, print totals, and free memory
void wait(struct options opt, struct nums *nums, struct thread_info *threads, struct args *args) {
    // Wait for the threads to finish
    for (int i = 0; i < opt.num_threads * 2; i++)
        pthread_join(threads[i].id, NULL);

    print_totals(nums);

    for (int i = 0; i < opt.num_threads; i++)
        free(threads[i].args);

    free(threads);
}

int main (int argc, char **argv)
{
    pthread_mutex_t *mutex_it = malloc(sizeof (pthread_mutex_t));

    struct options opt;
    struct nums nums;
    struct thread_info *thrs;
    long iter;

    srand(time(NULL));
    pthread_mutex_init(mutex_it, NULL);

    // Default values for the options
    opt.num_threads  = 4;
    opt.iterations   = 100000;
    opt.size         = 10;

    read_options(argc, argv, &opt);

    struct args args[opt.num_threads];

    iter = opt.iterations;
    nums.size = opt.size;
    nums.total = opt.iterations * opt.num_threads;

    pthread_mutex_t *mutex_inc = malloc(opt.size * sizeof(pthread_mutex_t));
    pthread_mutex_t *mutex_dec = malloc(opt.size * sizeof(pthread_mutex_t));

    for (int i = 0; i < opt.size; i++) {
        pthread_mutex_init(&mutex_inc[i], NULL);
        pthread_mutex_init(&mutex_dec[i], NULL);
    }

    nums.increase = malloc(opt.size * sizeof(long));
    nums.decrease = malloc(opt.size * sizeof(long));

    for (int i = 0; i < opt.size; i++) {
        nums.increase[i] = 0;
        nums.decrease[i] = nums.total;
    }

    thrs = start_threads(opt, &nums, mutex_it, &iter, mutex_inc, mutex_dec, args);
    wait(opt, &nums, thrs, args);

    pthread_mutex_destroy(mutex_it);
    free (mutex_it);

    for (int i = 0; i < opt.size; i++) {
        pthread_mutex_destroy(&mutex_inc[i]);
        pthread_mutex_destroy(&mutex_dec[i]);
    }

    free(mutex_inc);
    free(mutex_dec);
    free(nums.increase);
    free(nums.decrease);

    return 0;
}