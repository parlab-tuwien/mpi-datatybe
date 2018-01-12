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


#ifndef KEYVALUE_STORE_H_
#define KEYVALUE_STORE_H_

struct entry_s {
  char* key;
  char* value;
  struct entry_s *next;
};

typedef struct entry_s entry_t;

struct hashtable_s {
  int size;
  struct entry_s **table;
};

typedef struct hashtable_s dictionary_t;

typedef struct int_array {
    int* elements;
    int n_elems;
    int max_elem;
} int_array_t;

typedef struct string_array {
    char** elements;
    int n_elems;
} string_array_t;



int init_dictionary(dictionary_t *hashtable);
void cleanup_dictionary(dictionary_t *hashtable);
int add_element_to_dict(const char* key, const char* val, dictionary_t *hashtable);
int remove_element_from_dict(const char* key, dictionary_t *hashtable);

int get_value_from_dict(const dictionary_t *hashtable, const char* key, char** value);
int get_int_value_from_dict(const char* key, const dictionary_t *hashtable);
MPI_Datatype get_basetype_value_from_dict(const char* key, const dictionary_t *hashtable);
MPI_Datatype* get_basetype_list_from_dict(const char* key, const int n, const dictionary_t *hashtable);
int_array_t* get_int_array_from_dict(const char* key, const dictionary_t *hashtable);
string_array_t* get_string_array_from_dict(const char* key, const dictionary_t *hashtable);

void print_dictionary(FILE* f, const dictionary_t *hashtable);

void copy_dict_entry(const char* key, const dictionary_t *dict_source, dictionary_t *dict_dest);


#endif /* KEYVALUE_STORE_H_ */
