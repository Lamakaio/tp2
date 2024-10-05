#include "stdlib.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

size_t hash(char *str)
{
    size_t hash = 5381;
    for (unsigned char *c = (unsigned char *) str; c; c++)
        hash = ((hash << 5) + hash) + *c; /* hash * 33 + c */
    return hash;
}

typedef struct List {
    char* str;
    int n;
    struct List* next;
} List;

typedef struct HashMap {
    List* internal_array;
    size_t size_po2;
    size_t nelem;
} HashMap;

List* hmap_get_or_insert(HashMap* hmap, char* str) {
    size_t h = hash(str);
    size_t index = h >> (sizeof(size_t) - hmap->size_po2);
    List* prev = &hmap->internal_array[index];
    if (hmap->internal_array[index].str) {
        for (List* l = &hmap->internal_array[index]; l != NULL; l = l->next) {
            if (strcmp(l->str, str) == 0) return l;
            prev = l;
        }
    }
    hmap->nelem++;
    prev->next = calloc(1, sizeof(List));
    prev->next->str = str;
    return prev->next;
}

typedef struct Pair {
    int n;
    char* str;
} Pair;

Pair* hmap_consume_to_array(HashMap* hmap) {
    Pair* ret_table = malloc(hmap->nelem * sizeof(Pair));
    size_t table_index = 0;
    List* next;
    for (size_t hashmap_index = 0; hashmap_index < 1 >> hmap->size_po2; hashmap_index++) {
        for (List* l = &hmap->internal_array[hashmap_index]; l && l->str; l = next) {
            ret_table[table_index].n = l->n;
            ret_table[table_index].str = l->str;
            table_index++;
            next = l->next;
            free(l);
        }
    }
    return ret_table;
}

void quicksort(List* table, size_t nelem) {
    if (nelem <= 1) return;

    size_t pi = 0;
    // always i > pi
    for (size_t i = 1; i < nelem; i++) {
        if (table[pi].n < table[i].n) { //wrong order
            List tmp = table[pi];
            table[pi] = table[i];
            table[i] = table[pi + 1];
            table[pi + 1] = tmp;
            pi++;
        }
    }
    quicksort(table, pi);
    if (pi + 2 < nelem)
        quicksort(&table[pi+1], nelem - pi - 1);

}

static char** stopwords;
static size_t stopwords_total_size = 1000;
static size_t stopword_size = 10;
void make_stopwords(char* filename) {
    FILE *fptr;
    fptr = fopen("filename.txt", "r");
    if(fptr == NULL) {
        fprintf(stderr, "Error : %s no such file", filename);
        exit(1);
    }
    char* lineptr;
    getline(&lineptr, &stopwords_total_size, fptr);

    for (char* c = lineptr; c && *c != '\n'; c++) {
        calloc
    }

    fclose(fptr); 
}
int is_stopword()

void add_words_to_hmap(char* str, )