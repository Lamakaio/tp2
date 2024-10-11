#include "stdlib.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static inline int is_letter(char *c) {
    if (*c >= 'A' && *c <= 'Z') *c = *c + 32;
    return *c >= 'a' && *c <= 'z';
}

size_t hash(char *str)
{
    size_t hash = 5381;
    for (unsigned char *c = (unsigned char *) str; *c; c++)
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

static size_t n_stopwords = 0;
static size_t stopword_hashes[150];
static size_t max_line_size = 200;
static size_t hmap_po2_size = 17; //2**17 ~= 130000, sounds ok. 

static List dummy;

//suppose there is no hash collision
//but it's pretty unlikely here. 
//Note that only a collision between one of the top words and one of the stopwords would matter. 
int is_stopword(size_t hash) {
    for (size_t i = 0; i < n_stopwords; i++) {
        if (hash == stopword_hashes[i]) return 1;
    }
    return 0;
}

//Get the lower bits of the hash to get a table index
size_t index_from_hash(size_t h, size_t po2) {
    return (h << (sizeof(size_t)*8 - po2)) >> (sizeof(size_t)*8 - po2);
}

//Get a pointer to the hmap element. Insert it if it wasn't in the table (0-init)
List* hmap_get_or_insert(HashMap* hmap, char* str) {
    size_t h = hash(str);
    if (strlen(str) < 2 || is_stopword(h)) return &dummy;

    size_t index = index_from_hash(h, hmap->size_po2);
    if (hmap->internal_array[index].str) {
        List* prev = &hmap->internal_array[index];
        for (List* l = &hmap->internal_array[index]; l != NULL && l->str != NULL; l = l->next) {
            if (strcmp(l->str, str) == 0) {
                return l;
            }
            prev = l;
        }
        hmap->nelem++;
        prev->next = calloc(1, sizeof(List));
        prev->next->str = malloc(strlen(str) + 1);
        strcpy(prev->next->str, str);
        return prev->next;
    }
    else {
        hmap->nelem++;
        hmap->internal_array[index].str = malloc(strlen(str) + 1);
        strcpy(hmap->internal_array[index].str, str);
        return &hmap->internal_array[index];
    }
}

typedef struct Pair {
    int n;
    char* str;
} Pair;

//Empty the hmap (freeing the memory allocated by hmap_get_or_insert) 
//And construct a table of all elements in it. 
Pair* hmap_consume_to_array(HashMap* hmap) {
    Pair* ret_table = malloc(hmap->nelem * sizeof(Pair));
    size_t table_index = 0;
    List* next;
    for (size_t hashmap_index = 0; hashmap_index < 1 << hmap->size_po2; hashmap_index++) {
        for (List* l = &hmap->internal_array[hashmap_index]; l && l->str; l = next) {
            ret_table[table_index].n = l->n;
            ret_table[table_index].str = l->str;
            table_index++;
            next = l->next;
            if (l != &hmap->internal_array[hashmap_index]) {
                free(l);
            }
        }
    }
    return ret_table;
}

//Quicksort a table of pairs according to the second element. 
void quicksort(Pair* table, size_t nelem) {
    if (nelem <= 1) return;

    size_t pi = 0;
    // always i > pi
    for (size_t i = 1; i < nelem; i++) {
        if (table[pi].n < table[i].n) { //the two elems are in wrong order
            //to avoid copies, we move the pivot one spot to the right
            //then the element on that spot (it's necessarily bigger than pivot anyway), to the element we just compared
            //then the element we just compared, at the pivot's old spot (so before the pivot)
            //Note that this makes the algo non-stable, but we don't care
            Pair tmp = table[pi];
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
static size_t stopwords_total_size = 1000;

//Make a hash array of the stopwords. 
//It's relatively small so iterating over it to test is pretty fast
//As the memory accesses are all concurrent (and so cached)
void make_stopwords_hashes(char* filename) {
    FILE *fptr;
    fptr = fopen(filename, "r");
    if(fptr == NULL) {
        fprintf(stderr, "Error : %s no such file\n", filename);
        exit(1);
    }
    char* lineptr = 0;

    getline(&lineptr, &stopwords_total_size, fptr);
    fclose(fptr); 

    char* prev_str_start = lineptr;
    char* c;
    for (char* c = lineptr; *c ; c++) {
        if (!is_letter(c)) {
            if (c != prev_str_start) {
                *c = 0;
                size_t h = hash(prev_str_start);
                stopword_hashes[n_stopwords++] = h;
            }
            prev_str_start = c+1;
        }
    }
    if (c != prev_str_start) {
        size_t h = hash(prev_str_start);
        stopword_hashes[n_stopwords++] = h;
    }
    free(lineptr);
}

//add all the words in the book to the hashmap
void add_words_to_hmap(char* filename, HashMap *hmap) {
    FILE *fptr;
    fptr = fopen(filename, "r");
    if(fptr == NULL) {
        fprintf(stderr, "Error : %s no such file\n", filename);
        exit(1);
    }
    
    char* lineptr = 0;
    while (getline(&lineptr, &max_line_size, fptr) != -1) {
        char* prev_str_start = lineptr;
        for (char* c = lineptr; *c ; c++) {
            if (!is_letter(c)) {
                if (c != prev_str_start) {
                    *c = 0;
                    List* l = hmap_get_or_insert(hmap, prev_str_start);
                    l->n ++;
                }
                prev_str_start = c+1;
            }
        }
    }
    free(lineptr);
    fclose(fptr);
}

int main(int argc, char** argv) {
    if (argc != 4) {
        fprintf(stderr, "Wrong number of arguments. Syntax is freq [book] [stopwords] [n]");
        exit(1);
    }
    char* end;
    long n = strtol(argv[3], &end, 10);
    if (end == argv[3] || *end != 0) {
        fprintf(stderr, "n must be an integer\n");
        exit(1);
    }

    make_stopwords_hashes(argv[2]);
    HashMap hashmap = {
        .internal_array = calloc(1,(1 << hmap_po2_size) * sizeof(List)),
        .size_po2 = hmap_po2_size,
        .nelem = 0
    };
    add_words_to_hmap(argv[1], &hashmap);

    Pair* table = hmap_consume_to_array(&hashmap);
    quicksort(table, hashmap.nelem);

    for (int i = 0; i < n && i < hashmap.nelem; i++) {
        fprintf(stderr, "%d: \"%s\", with %d occurences\n", i, table[i].str, table[i].n);
    }

    free(hashmap.internal_array);
    for (int i = 0; i < hashmap.nelem; i++) {
        free(table[i].str);
    }
    free(table);
}