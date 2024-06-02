#include <stdlib.h>
#include <pthread.h>

// circular array
typedef struct _queue {
    int size;
    int used;
    int first;
    void **data;
    pthread_mutex_t *mutex_q;
    pthread_cond_t *buffer_full;
    pthread_cond_t *buffer_empty;
} _queue;

#include "queue.h"

queue q_create(int size) {

    queue q = malloc(sizeof(_queue));

    pthread_mutex_t *mutex_q = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init (mutex_q, NULL);
    pthread_cond_t *buffer_full = malloc(sizeof(pthread_cond_t));
    pthread_cond_t *buffer_empty = malloc(sizeof(pthread_cond_t));
    pthread_cond_init(buffer_full, NULL);
    pthread_cond_init(buffer_empty, NULL);

    q->size  = size;
    q->used  = 0;
    q->first = 0;
    q->data  = malloc(size*sizeof(void *));
    q->mutex_q = mutex_q;
    q->buffer_full = buffer_full;
    q->buffer_empty = buffer_empty;

    return q;
}

int q_elements(queue q) {
    int i;

    pthread_mutex_lock(q->mutex_q);
    i = q->used;
    pthread_mutex_unlock(q->mutex_q);

    return i;
}

int q_insert(queue q, void *elem) {
    int i;

    pthread_mutex_lock(q->mutex_q);

    while (q->size == q->used){
        pthread_cond_wait(q->buffer_full, q->mutex_q);
    }

    q->data[(q->first+q->used) % q->size] = elem;
    q->used++;
    i = 1;

    if (q->used > 0){
        pthread_cond_broadcast(q->buffer_empty);
    }

    pthread_mutex_unlock(q->mutex_q);

    return i;
}

void *q_remove(queue q) {
    void *res;

    pthread_mutex_lock(q->mutex_q);


    while (q->used == 0){
        pthread_cond_wait(q->buffer_empty, q->mutex_q);
    }

    res = q->data[q->first];

    q->first = (q->first+1) % q->size;
    q->used--;

    if (q->used < q->size){
        pthread_cond_broadcast(q->buffer_full);
    }

    pthread_mutex_unlock(q->mutex_q);

    return res;
}

void q_destroy(queue q) {
    free(q->data);
    pthread_mutex_destroy(q->mutex_q);
    free(q->mutex_q);
    free(q->buffer_full);
    free(q->buffer_empty);
    free(q);
}
