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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mpi.h>

#include "datatypes_bench.h"
#include "perftypes.h"
#include "comm_patterns.h"
#include "option_parser/parse_perftypes_options.h"
#include "dictionary/keyvalue_store.h"

//@ add_includes
//@ declare_variables

static char* pattern_key = "pattern";
static char* datatype_create_key = "layout";
static char* root_key = "root";
static char* test_type_key = "test_type";

static pattern_functions_t pattern_list[] = {
    { "pingpong",
        {   [basic] = pingpongpattern,
            [dynamic] = pingpongpattern_dynamictype}
    },
    { "bcast",
        {   [basic] = bcastpattern,
            [dynamic] = bcastpattern_dynamictype}
    },
    { "allgather",
        {   [basic] = allgatherpattern,
            [dynamic] = allgatherpattern_dynamictype}
    }
};

static const int N_PATTERNS = sizeof(pattern_list) / sizeof(pattern_list[0]);

char *params1[] = { "A", "B" };
char *params2[] = { "A1", "A2", "B" };
char *params3[] = { "A", "B1", "B2" };
char *params4[] = { "A1", "A2", "B1", "B2" };
char *params5[] = { "A", "B", "c", "blist" };
char *params6[] = { "A", "B", "S1", "S2" };
char *params7[] = { "A", "B", "S" };
char *params8[] = { "A" };
char *params9[] = { "A", "B", "l" };
char *params10[] = { "A1", "A2", "B", "S" };

layout_functions_t layout_list[] = {
    { "tiled", bl_tiled, basic, params1, sizeof(params1) / sizeof(params1[0]) },
    { "bucket", bl_bucket, basic, params2, sizeof(params2) / sizeof(params2[0]) },
    { "block", bl_block, basic, params3, sizeof(params3) / sizeof(params3[0]) },
    { "alternating", bl_alternating, basic, params4, sizeof(params4) / sizeof(params4[0]) },
    { "tiled_heterogeneous", bl_tiled_heterogeneous, basic, params5, sizeof(params5) / sizeof(params5[0]) },
    { "tiled_struct", bl_tiled_struct, basic, params6, sizeof(params6) / sizeof(params6[0]) },
    { "contig_type", dl_contig_type, dynamic, NULL, -1 }, // here we need to be careful, give some "useful" defaults
    { "alternating_repeated", dl_alternating_repeated, dynamic, params2, sizeof(params2) / sizeof(params2[0]) },
    { "alternating_struct", dl_alternating_struct, dynamic, params2, sizeof(params2) / sizeof(params2[0]) },
    { "tiled_vector", dl_tiled_vector, dynamic, params1, sizeof(params1) / sizeof(params1[0]) },
    { "vector_tiled", dl_vector_tiled, dynamic, params7, sizeof(params7) / sizeof(params7[0]) },
    { "block_indexed", dl_block_indexed, dynamic, params3, sizeof(params3) / sizeof(params3[0]) },
    { "alternating_indexed", dl_alternating_indexed, dynamic, params4, sizeof(params4) / sizeof(params4[0]) },
    { "rowcol_full_indexed", dl_rowcol_full_indexed, dynamic, params8, sizeof(params8) / sizeof(params8[0]) },
    { "rowcol_contiguous_and_indexed", dl_rowcol_contiguous_and_indexed, dynamic, params8, sizeof(params8) / sizeof(params8[0]) },
    { "rowcol_struct", dl_rowcol_struct, dynamic, params8, sizeof(params8) / sizeof(params8[0]) },
    { "blocks", dl_blocks, dynamic, params9, sizeof(params9) / sizeof(params9[0]) },
    { "tiled_struct_indexed_all", dl_tiled_struct_indexed_all, dynamic, params1, sizeof(params1) / sizeof(params1[0]) },
    { "alternating_indexed_fixed", bl_alternating_indexed_fixed, basic, params10, sizeof(params10) / sizeof(params10[0]) },
    { "contig_alternating_indexed_fixed", dl_contig_alternating_indexed_fixed, dynamic, params10, sizeof(params10) / sizeof(params10[0]) },
    { "alternating_aligned", bl_alternating_aligned, basic, params4, sizeof(params4) / sizeof(params4[0]) },
    { "tiled_struct_indexed_Sblocks", dl_tiled_struct_indexed_Sblocks, dynamic, params7, sizeof(params7) / sizeof(params7[0]) },
    { "basetype", bl_basetype, basic, NULL, -1 }
};

const int N_LAYOUTS = sizeof(layout_list) / sizeof(layout_list[0]);

void execute_pattern(char* pattern, pattern_config_t config, dictionary_t *dict) {
  int i;
  int found = 0;

  if (pattern == NULL) {
    printf("Error: no pattern specified.\n");
    exit(1);
  }

  for (i = 0; i < N_PATTERNS; i++) {
    if (strcmp(pattern, pattern_list[i].name) == 0) {
      pattern_list[i].function[config.type_info](config, dict);
      found = 1;
      break;
    }
  }

  if (!found) {
    printf("Error: no pattern found with the name %s.\n", pattern);
    exit(1);
  }

}

void get_create_function(const char* name, type_generator_t *out_generator, char ***out_dt_params, int *nb_params,
    dt_type_t *type_info) {
  int i;
  int found = 0;

  if (name == NULL) {
    printf("Error: unknown datatype creation function: %s\n", name);
    exit(1);
  }

  for (i = 0; i < N_LAYOUTS; i++) {
    if (strcmp(name, layout_list[i].name) == 0) {
      *out_generator = layout_list[i].function;
      *out_dt_params = layout_list[i].dt_params;
      *nb_params = layout_list[i].nb_params;
      *type_info = layout_list[i].type_info;
      found = 1;
      break;
    }
  }

  if (0 == found) {
    printf("Error: unknown datatype creation function: %s\n", name);
    exit(1);
  }
}

int main(int argc, char *argv[]) {
  int rank, root_proc;
  pattern_config_t config;
  dictionary_t dict;
  char* selected_pattern;
  char* test_type;
  char* selected_layout;
  int ret;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  init_dictionary(&dict);

  parse_perftypes_options(&dict, argc, argv);

  //@ initialize_bench
  root_proc = get_int_value_from_dict(root_key, &dict);

  ret = get_value_from_dict(&dict, test_type_key, &test_type);
  if (ret != 0 || test_type == NULL) {
    printf("\nError: required parameter \"%s\" is not specified. \n", test_type_key);
    exit(1);
  }
  ret = get_value_from_dict(&dict, pattern_key, &selected_pattern);
  if (ret != 0 || selected_pattern == NULL) {
    printf("\nError: required parameter \"%s\" is not specified. \n", pattern_key);
    exit(1);
  }
  ret = get_value_from_dict(&dict, datatype_create_key, &selected_layout);
  if (ret != 0 || selected_layout == NULL) {
    printf("\nError: required parameter \"%s\" is not specified. \n", datatype_create_key);
    exit(1);
  }

  get_create_function(selected_layout, &config.create_datatype, &config.dt_parameters, &config.nb_params,
      &config.type_info);
  config.comm = MPI_COMM_WORLD;
  config.root_proc = root_proc;

  config.use_datatype = 0;
  if (test_type != NULL) {
    if (strcmp(test_type, "datatype") == 0) {
      config.use_datatype = 1;
    } else if (strcmp(test_type, "pack") == 0) {
      config.use_datatype = 0;
    }
  }

  execute_pattern(selected_pattern, config, &dict);

  //@cleanup_bench
  free(test_type);
  free(selected_layout);
  free(selected_pattern);
  cleanup_dictionary(&dict);
  MPI_Finalize();

  return 0;
}
