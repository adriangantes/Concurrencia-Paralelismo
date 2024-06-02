
void *thread_insertar(void *arg) {
    queue q = (queue)arg;

    for (int i = 0; i < 10000; ++i) {
        int elemento = 1;
        q_insert(q, (void*)(intptr_t)elemento);
    }

    return NULL;
}

void *thread_eliminar(void *arg) {
    queue q = (queue)arg;
    int total = 0;

    for (int i = 0; i < 10000; ++i) {
        int elemento = (int)(intptr_t)q_remove(q);
        total += elemento;
    }

    printf("\nTotal acumulado: %d", total);

    return NULL;
}

void probar_cola(struct options opt){

    printf("\nPrueba threads:\n");

    queue q = q_create(5);

    pthread_t threads_insertar[opt.num_threads];
    pthread_t threads_eliminar[opt.num_threads];

    for (int i = 0; i < opt.num_threads; ++i) {
        pthread_create(&threads_insertar[i], NULL, thread_insertar, q);
        pthread_create(&threads_eliminar[i], NULL, thread_eliminar, q);
    }

    for (int i = 0; i < opt.num_threads; ++i) {
        pthread_join(threads_insertar[i], NULL);
        pthread_join(threads_eliminar[i], NULL);
    }

    q_destroy(q);
}