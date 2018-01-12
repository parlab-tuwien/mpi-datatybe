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


#ifndef DATATYPES_BENCH_H_
#define DATATYPES_BENCH_H_

#include "dictionary/keyvalue_store.h"

typedef int (*type_generator_t)(dictionary_t *dict, MPI_Datatype *t, int* flags);

typedef enum DatatypeTypes  {
    basic,
    dynamic
} dt_type_t;

typedef struct patterncf {
    int root_proc;
    MPI_Comm comm;
    int use_datatype;
    type_generator_t create_datatype;
    char **dt_parameters;
    int nb_params;
    dt_type_t type_info;

} pattern_config_t;

typedef int (*comm_pattern_meas_t)(pattern_config_t conf, dictionary_t *dict);

typedef struct pattern_struct {
    char* name;
    comm_pattern_meas_t function[2];
} pattern_functions_t;

typedef struct layout_struct {
    char* name;
    type_generator_t function;
    dt_type_t type_info;
    char **dt_params;
    int nb_params;
} layout_functions_t;


extern layout_functions_t layout_list[];
extern const int N_LAYOUTS;

#endif /* DATATYPES_BENCH_H_ */
