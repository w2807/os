#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>

#define SIZE 10

typedef struct
{
    int *arr;
    int size;
} ThreadData;

int comp(const void *a, const void *b)
{
    return *(int *)a - *(int *)b;
}

void *sort_half(void *params)
{
    ThreadData *data = (ThreadData *)params;
    qsort(data->arr, data->size, sizeof(int), comp);
    return NULL;
}

void merge_arrays(int *arr, int *arr1, int *arr2, int size)
{
    int i = 0, j = 0, k = 0;
    while (i < size / 2 && j < size / 2)
    {
        if (arr1[i] < arr2[j])
            arr[k++] = arr1[i++];
        else
            arr[k++] = arr2[j++];
    }
    while (i < size / 2)
        arr[k++] = arr1[i++];
    while (j < size / 2)
        arr[k++] = arr2[j++];
}

int main()
{
    pthread_t t1, t2;
    int arr[SIZE] = {0};
    int arr1[SIZE / 2] = {0};
    int arr2[SIZE / 2] = {0};
    for (int i = 0; i < SIZE; i++)
        scanf("%d", &arr[i]);
    for (int i = 0; i < SIZE / 2; i++)
    {
        arr1[i] = arr[i];
        arr2[i] = arr[i + SIZE / 2];
    }
    ThreadData data1 = {arr1, SIZE / 2};
    ThreadData data2 = {arr2, SIZE / 2};
    pthread_create(&t1, NULL, sort_half, &data1);
    pthread_create(&t2, NULL, sort_half, &data2);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    merge_arrays(arr, arr1, arr2, SIZE);
    for (int i = 0; i < SIZE; i++)
        printf("%d ", arr[i]);
    printf("\n");
    return 0;
}