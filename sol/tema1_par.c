/*
 * APD - Tema 1
 * Octombrie 2020
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
//#include "pthread_barrier_mac.h"
#define min(x, y) (((x) < (y) ? (x) : (y)))


char *in_filename_julia;
char *in_filename_mandelbrot;
char *out_filename_julia;
char *out_filename_mandelbrot;

int width, height, widthMan, heightMan;
int **resultJul, **resultMan;
int P;
pthread_barrier_t barrier;

// structura pentru un numar complex
typedef struct _complex {
    double a;
    double b;
} complex;

// structura pentru parametrii unei rulari
typedef struct _params {
    int is_julia, iterations;
    double x_min, x_max, y_min, y_max, resolution;
    complex c_julia;
} params;

params par, parMan;

// citeste argumentele programului
void get_args(int argc, char **argv)
{
    if (argc < 6) {
        printf("Numar insuficient de parametri:\n\t"
                "./tema1 fisier_intrare_julia fisier_iesire_julia "
                "fisier_intrare_mandelbrot fisier_iesire_mandelbrot\n");
        exit(1);
    }

    in_filename_julia = argv[1];
    out_filename_julia = argv[2];
    in_filename_mandelbrot = argv[3];
    out_filename_mandelbrot = argv[4];
    P = atoi(argv[5]);
}

// citeste fisierul de intrare
void read_input_file(char *in_filename, params* par)
{
    FILE *file = fopen(in_filename, "r");
    if (file == NULL) {
        printf("Eroare la deschiderea fisierului de intrare!\n");
        exit(1);
    }

    fscanf(file, "%d", &par->is_julia);
    fscanf(file, "%lf %lf %lf %lf",
            &par->x_min, &par->x_max, &par->y_min, &par->y_max);
    fscanf(file, "%lf", &par->resolution);
    fscanf(file, "%d", &par->iterations);

    if (par->is_julia) {
        fscanf(file, "%lf %lf", &par->c_julia.a, &par->c_julia.b);
    }

    fclose(file);
}

// scrie rezultatul in fisierul de iesire
void write_output_file(char *out_filename, int **result, int width, int height)
{
    int i, j;

    FILE *file = fopen(out_filename, "w");
    if (file == NULL) {
        printf("Eroare la deschiderea fisierului de iesire!\n");
        return;
    }

    fprintf(file, "P2\n%d %d\n255\n", width, height);
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            fprintf(file, "%d ", result[i][j]);
        }
        fprintf(file, "\n");
    }

    fclose(file);
}

// aloca memorie pentru rezultat
int **allocate_memory(int width, int height)
{
    int **result;
    int i;

    result = malloc(height * sizeof(int*));
    if (result == NULL) {
        printf("Eroare la malloc!\n");
        exit(1);
    }

    for (i = 0; i < height; i++) {
        result[i] = malloc(width * sizeof(int));
        if (result[i] == NULL) {
            printf("Eroare la malloc!\n");
            exit(1);
        }
    }

    return result;
}

// elibereaza memoria alocata
void free_memory(int **result, int height)
{
    int i;
    for (i = 0; i < height; i++) {
        free(result[i]);
    }
    free(result);
}

// ruleaza algoritmul Julia
void run_julia(int thread_id)
{
    int w, h, i;
    int start, end;
    for (w = 0; w < width; w++) {
        start = thread_id * (double)height / P;
        end = min((thread_id + 1) * (double)height / P, height);
        for (h = start; h < end; h++) {
            int step = 0;
            complex z = { .a = w * par.resolution + par.x_min,
                            .b = h * par.resolution + par.y_min };

            while (sqrt(pow(z.a, 2.0) + pow(z.b, 2.0)) < 2.0 && step < par.iterations) {
                complex z_aux = { .a = z.a, .b = z.b };

                z.a = pow(z_aux.a, 2) - pow(z_aux.b, 2) + par.c_julia.a;
                z.b = 2 * z_aux.a * z_aux.b + par.c_julia.b;

                step++;
            }
            
            resultJul[h][w] = step % 256;
        }
        pthread_barrier_wait(&barrier);
    }

    // transforma rezultatul din coordonate matematice in coordonate ecran
    start = thread_id * (double)height/2 / P;
    end = min((thread_id + 1) * (double)height/2 / P, (double)height/2);
    for (i = start; i < end; i++) {
        int *aux = resultJul[i];
        resultJul[i] = resultJul[height - i - 1];
        resultJul[height - i - 1] = aux;
    }
}

// ruleaza algoritmul Mandelbrot
void run_mandelbrot(int thread_id)
{
    int w, h, i;
    int start, end;
    for (w = 0; w < widthMan; w++) {
        start = thread_id * (double)heightMan / P;
        end = min((thread_id + 1) * (double)heightMan / P, heightMan);
        for (h = start; h < end; h++) {
            complex c = { .a = w * parMan.resolution + parMan.x_min,
                            .b = h * parMan.resolution + parMan.y_min };
            complex z = { .a = 0, .b = 0 };
            int step = 0;

            while (sqrt(pow(z.a, 2.0) + pow(z.b, 2.0)) < 2.0 && step < parMan.iterations) {
                complex z_aux = { .a = z.a, .b = z.b };

                z.a = pow(z_aux.a, 2.0) - pow(z_aux.b, 2.0) + c.a;
                z.b = 2.0 * z_aux.a * z_aux.b + c.b;

                step++;
            }

            resultMan[h][w] = step % 256;
        }
        pthread_barrier_wait(&barrier);
    }

    // transforma rezultatul din coordonate matematice in coordonate ecran
    start = thread_id * (double)heightMan/2 / P;
    end = min((thread_id + 1) * (double)heightMan/2 / P, (double)heightMan/2);
    for (i = start; i < end; i++) {
        int *aux = resultMan[i];
        resultMan[i] = resultMan[heightMan - i - 1];
        resultMan[heightMan - i - 1] = aux;
    }
    
    
}

void *run_alg(void *arg)
{
    int thread_id = *(int *)arg;
    
    run_julia(thread_id);
    pthread_barrier_wait(&barrier);

    write_output_file(out_filename_julia, resultJul, width, height);
    

    pthread_barrier_wait(&barrier);
    run_mandelbrot(thread_id);
    pthread_barrier_wait(&barrier);

    write_output_file(out_filename_mandelbrot, resultMan, widthMan, heightMan);

    pthread_exit(NULL);
}


int main(int argc, char *argv[])
{


    // se citesc argumentele programului
    get_args(argc, argv);
    pthread_t tid[P];
    int thread_id[P];
    pthread_barrier_init(&barrier, NULL, P);
    int i;
    
    // Julia:
    // - se citesc parametrii de intrare
    // - se aloca tabloul cu rezultatul
    // - se ruleaza algoritmul
    // - se scrie rezultatul in fisierul de iesire
    // - se elibereaza memoria alocata
   
    read_input_file(in_filename_julia, &par);

    width = (par.x_max - par.x_min) / par.resolution;
    height = (par.y_max - par.y_min) / par.resolution;

    resultJul = allocate_memory(width, height);

    read_input_file(in_filename_mandelbrot, &parMan);
    widthMan = (parMan.x_max - parMan.x_min) / parMan.resolution;
    heightMan = (parMan.y_max - parMan.y_min) / parMan.resolution;
    resultMan = allocate_memory(widthMan, heightMan);
    
    for (i = 0; i < P; i++) {
        thread_id[i] = i;
        pthread_create(&tid[i], NULL, run_alg, &thread_id[i]);
    }
    
    for (i = 0; i < P; i++) {
        pthread_join(tid[i], NULL);
    }
    pthread_barrier_destroy(&barrier);
    

    // Mandelbrot:
    // - se citesc parametrii de intrare
    // - se aloca tabloul cu rezultatul
    // - se ruleaza algoritmul
    // - se scrie rezultatul in fisierul de iesire
    // - se elibereaza memoria alocata

    return 0;
}


