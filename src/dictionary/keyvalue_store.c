/*  MPI-Datatybe - MPI Datatype Benchmark
 *  
 *  Copyright 2017 Alexandra Carpen-Amarie, Sascha Hunold, Jesper Larsson Träff
 *      Research Group for Parallel Computing
 *      Faculty of Informatics
 *      Vienna University of Technology, Austria
 *  
 *  <license>
 *      This program is free software: you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, either version 2 of the License, or
 *      (at your option) any later version.
 *  
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *  
 *      You should have received a copy of the GNU General Public License
 *      along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *  </license>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#include "dictionary/dictionary_helpers.h"
#include "keyvalue_store.h"

static const int DEFAULT_DICTIONARY_SIZE = 20;
static const int LEN_KEYVALUE_LIST_BATCH = 10;

int init_dictionary(dictionary_t *hashtable) {
  int i;
  int ret = 1;

  /* Allocate pointers to the head nodes. */
  if ((hashtable->table = calloc(DEFAULT_DICTIONARY_SIZE, sizeof(entry_t *))) == NULL) {
    return ret;
  }
  for (i = 0; i < DEFAULT_DICTIONARY_SIZE; i++) {
    hashtable->table[i] = NULL;
  }
  hashtable->size = DEFAULT_DICTIONARY_SIZE;

  return 0;
}


void cleanup_dictionary(dictionary_t *hashtable) {
  entry_t *last = NULL;
  entry_t *next = NULL;

  if (hashtable->table != NULL) {
    int i;
    for (i = 0; i < hashtable->size; i++) {
      next = hashtable->table[i];
      while (next != NULL) {
        free(next->key);
        free(next->value);
        last = next;
        next = next->next;
        free(last);
      }
    }
    free(hashtable->table);
  }
}

// One-at-a-Time Hash  by Bob Jenkins (http://www.burtleburtle.net/bob/hash/doobs.html)
static int ht_hash(const dictionary_t *hashtable, const char* key) {
  unsigned long hash = 0;
  int i = 0;

  for (i = 0; i < strlen(key); i++) {
    hash += key[i];
    hash += (hash << 10);
    hash ^= (hash >> 6);
  }
  hash += (hash << 3);
  hash ^= (hash >> 11);
  hash += (hash << 15);

  return (hash % hashtable->size);
}

static entry_t *ht_newpair(const char* key, const char* value) {
  entry_t *newpair;

  if (key == NULL || value == NULL) {
    return NULL;
  }

  if ((newpair = malloc(sizeof(entry_t))) == NULL) {
    return NULL;
  }

  newpair->key = strdup(key);
  newpair->value = strdup(value);
  newpair->next = NULL;

  return newpair;
}


int add_element_to_dict(const char* key, const char* value, dictionary_t *hashtable) {
  int bin = 0;
  entry_t *newpair = NULL;
  entry_t *next = NULL;
  entry_t *last = NULL;
  int ret = 0;

  if (key == NULL || value == NULL) {
    return 1;
  }

  bin = ht_hash(hashtable, key);
  next = hashtable->table[bin];

  while (next != NULL && strcmp(next->key, key) != 0) {
    last = next;
    next = next->next;
  }

  /* There's already a pair.  Let's replace that string. */
  if (next != NULL && strcmp(next->key, key) == 0) {
    next->value = strdup(value);

    /* Nope, could't find it.  Time to grow a pair. */
  } else {
    newpair = ht_newpair(key, value);

    if (newpair == NULL) {
      ret = 1;

      /* We're at the start of the linked list in this bin. */
    } else if (next == hashtable->table[bin]) {
      newpair->next = next;
      hashtable->table[bin] = newpair;

      /* We're at the end of the linked list in this bin. */
    } else if (next == NULL) {
      last->next = newpair;

      /* We're in the middle of the list. */
    } else {
      newpair->next = next;
      last->next = newpair;
    }
  }

  return ret;
}



int get_value_from_dict(const dictionary_t *hashtable, const char* key, char** value) {
  int bin = 0;
  entry_t *pair;
  int ret = 1;

  if (key == NULL) {
    *value = NULL;
    return ret;
  }

  bin = ht_hash(hashtable, key);

  /* Step through the bin, looking for our value. */
  pair = hashtable->table[bin];
  while (pair != NULL && strcmp(key, pair->key) != 0) {
    pair = pair->next;
  }

  /* Did we actually find anything? */
  if (pair == NULL || strcmp(key, pair->key) != 0) {
    *value = NULL;
  } else {
    *value = strdup(pair->value);
    ret = 0;
  }
  return ret;
}


int get_int_value_from_dict(const char* key, const dictionary_t *hashtable) {
    char* val = NULL;
    int result = 0;
    int ret;

    ret = get_value_from_dict(hashtable, key, &val);
    if (ret == 0 && val != NULL) {
        result = to_int(val);
    }
    else {
        printf("\nError: required parameter \"%s\" is not specified. \n", key);
        exit(1);
    }

    free(val);
    return result;
}


MPI_Datatype get_basetype_value_from_dict(const char* key, const dictionary_t *hashtable) {
    char* val = NULL;
    MPI_Datatype result = 0;
    int ret;

    ret = get_value_from_dict(hashtable, key, &val);
    if (ret == 0 && val != NULL) {
        result = to_basetype(val);

    }
    else {
        printf("\nError: required parameter \"%s\" is not specified. \n", key);
        exit(1);
    }

    free(val);
    return result;
}


MPI_Datatype* get_basetype_list_from_dict(const char* key, const int n, const dictionary_t *hashtable) {
    char* val = NULL;
    MPI_Datatype* result = NULL;
    int ret;

    ret = get_value_from_dict(hashtable, key, &val);
    if (ret == 0 && val != NULL) {
        result = to_basetype_list(val, n);
    }
    else {
        printf("\nError: required parameter \"%s\" is not specified. \n", key);
        exit(1);
    }

    free(val);
    return result;
}


int_array_t* get_int_array_from_dict(const char* key, const dictionary_t *hashtable) {
    char* val = NULL;
    int_array_t* arr = NULL;
    int ret;

    arr = (int_array_t*)malloc(sizeof(int_array_t));
    arr->n_elems = 0;
    arr->max_elem = 0;
    arr->elements = (int*)malloc(sizeof(int) * LEN_KEYVALUE_LIST_BATCH);

    ret = get_value_from_dict(hashtable, key, &val);
    if (ret == 0 && val != NULL) {
        char* args_tok;
        char* save_str = NULL;

        args_tok = strtok_r(val, "/", &save_str);
        while (args_tok != NULL) {
            arr->elements[arr->n_elems] = to_int(args_tok);

            if (arr->elements[arr->n_elems] > arr->max_elem) {
                arr->max_elem = arr->elements[arr->n_elems];
            }

            arr->n_elems++;
            args_tok = strtok_r(NULL, "/", &save_str);

            if (arr->n_elems % LEN_KEYVALUE_LIST_BATCH == 0) {
                arr->elements = realloc(arr->elements, (arr->n_elems + LEN_KEYVALUE_LIST_BATCH) * sizeof(char*));
            }
        }
    }
    else {
        printf("\nError: required parameter \"%s\" is not specified. \n", key);
        exit(1);
    }

    free(val);
    return arr;
}


string_array_t* get_string_array_from_dict(const char* key, const dictionary_t *hashtable) {
    char* val = NULL;
    string_array_t* arr = NULL;
    int ret;

    arr = (string_array_t*)malloc(sizeof(string_array_t));
    arr->n_elems = 0;
    arr->elements = (char**)malloc(sizeof(char*) * LEN_KEYVALUE_LIST_BATCH);

    ret = get_value_from_dict(hashtable, key, &val);
    if (ret == 0 && val != NULL) {
        char* args_tok;
        char* save_str = NULL;

        args_tok = strtok_r(val, "/", &save_str);
        while (args_tok != NULL) {
            arr->elements[arr->n_elems] = strdup(args_tok);

            arr->n_elems++;
            args_tok = strtok_r(NULL, "/", &save_str);

            if (arr->n_elems % LEN_KEYVALUE_LIST_BATCH == 0) {
                arr->elements = realloc(arr->elements, (arr->n_elems + LEN_KEYVALUE_LIST_BATCH) * sizeof(char*));
            }
        }
    }
    else {
        printf("\nError: required parameter \"%s\" is not specified. \n", key);
        exit(1);
    }

    free(val);
    return arr;
}



int remove_element_from_dict(const char* key, dictionary_t *hashtable) {
  int bin = 0;
  int ret = 1;
  entry_t *pair;
  entry_t *last;

  if (key == NULL) {
    return ret;
  }

  bin = ht_hash(hashtable, key);
  pair = hashtable->table[bin];
  last = pair;

  while (pair != NULL && strcmp(key, pair->key) != 0) {
    last = pair;
    pair = pair->next;
  }

  if (pair != NULL && strcmp(key, pair->key) == 0) {
    // key was found
    if (pair ==  hashtable->table[bin]) { // only one key in the bucket
      free(pair->key);
      free(pair->value);

      hashtable->table[bin] = NULL;
      free(pair);
    } else {
      last->next = pair->next;

      free(pair->key);
      free(pair->value);
      free(pair);
    }
    ret = 0;
  }

  return ret;
}


int get_nkeys_from_dict(const dictionary_t* hashtable) {
  int i;
  int nkeys = 0;

  for (i = 0; i < hashtable->size; i++) {
    if (hashtable->table[i] != NULL) {
      entry_t *cur;
      cur = hashtable->table[i];
      while (cur != NULL) {
        nkeys++;
        cur = cur->next;
      }
    }
  }

  return nkeys;
}


void print_dictionary(FILE* f, const dictionary_t *hashtable) {
  int i;
  int nkeys;

  nkeys = get_nkeys_from_dict(hashtable);

  if (nkeys > 0) {
    fprintf(f, "#Key-value parameters:\n");
    for (i = 0; i < hashtable->size; i++) {
      if (hashtable->table[i] != NULL) {
        entry_t *cur;
        cur = hashtable->table[i];
        while (cur != NULL) {
          fprintf(f, "#@%s=%s\n", cur->key, cur->value);
          cur = cur->next;
        }
      }
    }
    fprintf(f, "# \n");
  }
}

void copy_dict_entry(const char* key, const dictionary_t *dict_source, dictionary_t *dict_dest) {
    char* str;
    int ret;

    ret = get_value_from_dict(dict_source, key, &str);
    if (ret!= 0 || str == NULL) {
        printf("\nError: required parameter \"%s\" is not specified. \n", key);
        exit(1);
    }

    add_element_to_dict(key, str, dict_dest);
    free(str);
}


