#include "sıralama.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define MAX_KELIME_UZUNLUGU 100

typedef struct {
    char **kelimeler;
    int left;
    int right;
    int depth;
    int maxDepth;
} SortParams;

void merge(char **array, int left, int middle, int right) {
    int i, j, k;
    int n1 = middle - left + 1;
    int n2 = right - middle;

    char **L = malloc(n1 * sizeof(char *));
    char **R = malloc(n2 * sizeof(char *));

    for (i = 0; i < n1; i++)
        L[i] = array[left + i];
    for (j = 0; j < n2; j++)
        R[j] = array[middle + 1 + j];

    i = 0;
    j = 0;
    k = left;
    while (i < n1 && j < n2) {
        if (strcmp(L[i], R[j]) <= 0) {
            array[k] = L[i];
            i++;
        } else {
            array[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1) {
        array[k] = L[i];
        i++;
        k++;
    }

    while (j < n2) {
        array[k] = R[j];
        j++;
        k++;
    }

    free(L);
    free(R);
}

int partition(char **array, int low, int high) {
    char *pivot = array[high];
    int i = (low - 1);

    for (int j = low; j <= high - 1; j++) {
        if (strcmp(array[j], pivot) < 0) {
            i++;
            char *temp = array[i];
            array[i] = array[j];
            array[j] = temp;
        }
    }
    char *temp = array[i + 1];
    array[i + 1] = array[high];
    array[high] = temp;
    return (i + 1);
}

void *mergeSortThread(void *arg) {
    SortParams *params = (SortParams *)arg;
    ParalelMergeSort(params->kelimeler, params->left, params->right, params->depth + 1, params->maxDepth);
    return NULL;
}

void ParalelMergeSort(char **kelimeler, int left, int right, int depth, int maxDepth) {
    if (left < right) {
        int middle = left + (right - left) / 2;

        pthread_t threadLeft, threadRight;
        int threadCreated = 0;

        if (depth < maxDepth) {
            ParalelMergeSort(kelimeler, left, middle, depth + 1, maxDepth);
            ParalelMergeSort(kelimeler, middle + 1, right, depth + 1, maxDepth);
        } else {
            pthread_create(&threadLeft, NULL, mergeSortThread, (void *)&(SortParams){kelimeler, left, middle, depth, maxDepth});
            pthread_create(&threadRight, NULL, mergeSortThread, (void *)&(SortParams){kelimeler, middle + 1, right, depth, maxDepth});
            threadCreated = 1;
        }

        if (threadCreated) {
            pthread_join(threadLeft, NULL);
            pthread_join(threadRight, NULL);
        }

        merge(kelimeler, left, middle, right);
    }
}

void *quickSortThread(void *arg) {
    SortParams *params = (SortParams *)arg;
    ParalelQuickSort(params->kelimeler, params->left, params->right, params->depth + 1, params->maxDepth);
    return NULL;
}

void ParalelQuickSort(char **kelimeler, int low, int high, int depth, int maxDepth) {
    if (low < high) {
        int pi = partition(kelimeler, low, high);

        pthread_t threadLeft, threadRight;
        int threadCreated = 0;

        if (depth < maxDepth) {
            SortParams paramsLeft = {kelimeler, low, pi - 1, depth, maxDepth};
            SortParams paramsRight = {kelimeler, pi + 1, high, depth, maxDepth};
            pthread_create(&threadLeft, NULL, quickSortThread, &paramsLeft);
            pthread_create(&threadRight, NULL, quickSortThread, &paramsRight);
            threadCreated = 1;
        } else {
            ParalelQuickSort(kelimeler, low, pi - 1, depth + 1, maxDepth);
            ParalelQuickSort(kelimeler, pi + 1, high, depth + 1, maxDepth);
        }

        if (threadCreated) {
            pthread_join(threadLeft, NULL);
            pthread_join(threadRight, NULL);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Kullanım: %s <inputfile> <outputfile> <# of threads> <algorithm>\n", argv[0]);
        return 1;
    }

    char *girisDosyasi = argv[1];
    char *cikisDosyasi = argv[2];
    int isParcacigiSayisi = atoi(argv[3]);
    char *algoritma = argv[4];

    FILE *dosya = fopen(girisDosyasi, "r");
    if (dosya == NULL) {
        perror("Dosya açılamadı");
        return 1;
    }

    char **kelimeler = malloc(sizeof(char*) * 1000);
    char tampon[MAX_KELIME_UZUNLUGU];
    int kelimeSayisi = 0;

    while (fgets(tampon, MAX_KELIME_UZUNLUGU, dosya) != NULL) {
        kelimeler[kelimeSayisi] = strdup(tampon);
        kelimeSayisi++;
    }
    fclose(dosya);

    if (strcmp(algoritma, "merge") == 0) {
        ParalelMergeSort(kelimeler, 0, kelimeSayisi - 1, 0, isParcacigiSayisi);
    } else if (strcmp(algoritma, "quick") == 0) {
        ParalelQuickSort(kelimeler, 0, kelimeSayisi - 1, 0, isParcacigiSayisi);
    }

    FILE *cikisDosyasiPtr = fopen(cikisDosyasi, "w");
    for (int i = 0; i < kelimeSayisi; i++) {
        fprintf(cikisDosyasiPtr, "%s", kelimeler[i]);
    }
    fclose(cikisDosyasiPtr);

    for (int i = 0; i < kelimeSayisi; i++) {
        free(kelimeler[i]);
    }
    free(kelimeler);

    return 0;
}
