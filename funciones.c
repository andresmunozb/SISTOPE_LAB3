#include <stdio.h>
#include <stdlib.h>
#include "funciones.h"


/*
 * Funcion wave_create
 * Funcion que inicializa un puntero de estructura Wave_t.
 * @param int n:número de filas.int m:número de columnas.int t:número de pasos.int h,numero de hebras.
 * @return Puntero de estructura Wave_t
*/
Wave_t* wave_create(int n,int m,int t, int h){
    Wave_t* wave = (Wave_t*) malloc(sizeof(Wave_t));
    wave->row = n;
    wave->col = m;
    wave->steps = t;
    wave->data = (float***) malloc (sizeof(float**)*t);
    wave->barriers = (pthread_barrier_t*) malloc(sizeof(pthread_barrier_t)*t);
    int i,j,k;
    for(i=0;i<t;i++){
        wave->data[i] = (float**) malloc (sizeof(float*)*n);
        pthread_barrier_init(&wave->barriers[i], NULL, h );//Se inicializan las barreras de cada etapa (paso) con la cantidad de hebras que se ejecutaran
        //Esto sirve para esperar a todas las hebras antes de pasar a la siguiente etapa.

        for(j=0;j<n;j++){
            wave->data[i][j] = (float*) malloc(sizeof(float)*m);
            for(k=0;k<m;k++){
                wave->data[i][j][k] = 0.0;
            }
        }
    }
    return wave;
}


/*
 * Funcion wave_destroy
 * Funcion que libera la memoria pedida en un puntero de estructura Wave_t inicializado.
 * @param Wave_t* wave:Puntero a estructura Wave_t que se desea destruir.
*/
void wave_destroy(Wave_t* wave){
    int i,j;  
    for(i=0;i<wave->steps;i++){
        pthread_barrier_destroy(&wave->barriers[i]);

        for ( j = 0; j < wave->row; j++)
        {
            free(wave->data[i][j]);
        }
        free(wave->data[i]);
    }
    free(wave->data);

    free(wave->barriers);
    free(wave);
}


/*
 * Funcion wave_show
 * Funcion que muestra por consola la estructura Wave_t.
 * @param Wave_t* wave:Puntero a estructura Wave_t que se desea mostrar.
*/
void wave_show(Wave_t *wave){
    int i,j,k;
    for (i=0;i<wave->steps;i++){
        printf("Step:%d\n",i );
        for(j=0;j<wave->row;j++){
            for(k=0;k<wave->col;k++){
                printf("%09.4f   ",wave->data[i][j][k]);

            }
            printf("\n");
        }
        printf("\n");
        
    }
}

/*
 * Funcion threads_init
 * Funcion que inicializa un arreglo de punteros de estructuras Thread_t.
 * @param int n:número de filas de la una matriz.int m:número de columnas de una matriz.int int_threads: número de hebras.
 * @return Arreglo de punteros de estructuras Thread_t.
*/
Thread_t** threads_init(int n, int m, int int_threads){
    int casillas = n*m;
    int i,j;
    int positions;
    int modulo = casillas%int_threads;

    //Se pide memoria para el arerglo de Threads_t
    Thread_t** threads = (Thread_t**)malloc(sizeof(Thread_t*)*int_threads+1);
    threads[int_threads] = NULL; //Para saber cual es el final del arreglo.

    if(modulo == 0){
        positions = casillas/int_threads;
    }else{
        positions = (casillas/int_threads) + 1;
    }
    //Se pide memoria para la maxima cantidad de posiciones en cada hebra.
    for(i = 0; i < int_threads; i++){
        threads[i] = (Thread_t*)malloc(sizeof(Thread_t));
        threads[i]->positions = (Position_t*)malloc(sizeof(Position_t)*positions);
        threads[i]->int_pos = 0;
    }

    //Se añaden las posiciones correspondientes que calcularan cada hebra.
    Position_t pos;
    int counter = 0;
    for(i=1;i<n-1;i++){
        for(j=1;j<m-1;j++){
            pos.row = i;
            pos.col = j;
            threads[counter%int_threads]->positions[threads[counter%int_threads]->int_pos] = pos;
            threads[counter%int_threads]->int_pos++;
            counter++;
        }
    }
    return threads;
}

/*
 * Funcion threads_destroy
 * Funcion que libera la memoria pedida en un arreglo de punteros de estructuras Thread_t inicializado.
 * @param Thread_t** threads:Arreglo de punteros de estructuras Thread_t.
*/
void threads_destroy(Thread_t** threads){

    int i = 0;
    while(threads[i] != NULL){
        free(threads[i]);
        free(threads[i]->positions); 
        i++;
    }
    free(threads);
}
/*
 * Funcion threads_show
 * Funcion que muestra por consola un arreglo de punteros de la estructura Thread_t.
 * @param Thread_t** threads:Arreglo de punteros de estructuras Thread_t.
*/
void threads_show(Thread_t** threads){
    int i = 0; 
    int j;
    while(threads[i] != NULL){
        printf("Hebra %d\n", i);
        printf("int_pos: %d\n",threads[i]->int_pos);
        for(j = 0; j < threads[i]->int_pos; j++){
            printf("%d,%d\n",threads[i]->positions[j].row,threads[i]->positions[j].col);
        }

        i++;
        printf("\n");
    }
}

/*
 * Funcion secuencial
 * Funcion que se encarga de calcular la ecuacion de Schroedinger de manera secuencial.
 * @param Wave_t* wave:Puntero a estructura Wave_t a la cual se aplicarán los calculos.
*/
void secuencial(Wave_t* wave){
    int i,j,k;
    float c,dt,dd,pd;
    c = 1.0;
    dt = 0.1;
    dd = 2.0;
    pd = ((c*c))*((dt/dd)*(dt/dd));

    for ( i = 1; i < wave->steps; i++){
        for ( j = 1; j < wave->row-1; j++){
            for( k = 1; k < wave->col-1;k++){
                calculate(wave,pd,i,j,k);
            }

        }
    }
}


/*
 * Funcion calculate
 * Funcion que se encarga de calcular la ecuacion de Schroedinger a una posicion especifica en un paso determinado.
 * @param Wave_t* wave:Puntero a estructura Wave_t a la cual se aplicará el calculo.float pd:constante. int i:paso int j:fila. int k:columna.
*/
void calculate(Wave_t* wave,float pd, int i, int j, int k){

    float up,down,right,left,before,before2;

    if(i>1){
    
        up = wave->data[i-1][j-1][k];
        down = wave->data[i-1][j+1][k];
        right = wave->data[i-1][j][k+1];
        left = wave->data[i-1][j][k-1];
        before = wave->data[i-1][j][k];
        before2 = wave->data[i-2][j][k];
        wave->data[i][j][k] = (pd*(up+right+left+down+before-(4*before)))+(2*before)-before2;
    }
    else if(i==1){
        up = wave->data[0][j-1][k];
        down = wave->data[0][j+1][k];
        right = wave->data[0][j][k+1];
        left = wave->data[0][j][k-1];
        before = wave->data[0][j][k];
        wave->data[1][j][k] = ((pd/2)*(up+right+left+down-(4*before)))+before;
    }
}
