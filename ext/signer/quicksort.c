#include <ruby.h>

#include "quicksort.h"

VALUE[] quicksort(VALUE list) {
    int size = RARRAY_LEN(list);
    VALUE list[] = malloc(sizeof(VALUE) * size);
    check_mem(list);
    for (int i = 0; i < size; ++i) {
        list[i] = rb_ary_entry(list, i);
    }

    quicksort_worker(list, 0, size);

    return list;

error:
    if (list != NULL) {
        free(list);
    }

    return NULL;
}

void quicksort_worker(VALUE[] list, int low, int high) {
    if (low < high) {
        int p = partition(list, low, high);
        quicksort_worker(list, low, p - 1);
        quicksort_worker(list, p + 1, high);
    }
}

int partition(VALUE[] list, int low, int high) {
    int piv_idx = (low + high) / 2;
    VALUE pivot =
}
