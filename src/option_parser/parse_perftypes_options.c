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
// avoid getsubopt bug
#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#include "mpi.h"
#include "dictionary/keyvalue_store.h"
#include "parse_perftypes_options.h"

static const int OUTPUT_ROOT_PROC = 0;

void print_help(void);

static const struct option default_long_options[] = {
    { "params", required_argument, 0, 'p' },
    { 0, 0, 0, 0 }
};
static const char default_opts_str[] = "h";

void print_error_and_exit(const char* error_str) {
  int my_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  if (my_rank == OUTPUT_ROOT_PROC) {
    fprintf(stderr, "\nERROR: %s\n\n", error_str);
  }
  MPI_Finalize();
  exit(1);
}

static void parse_parameters(char* args, dictionary_t* dict_p) {
  char* args_list;
  char* save_str;
  char* key;
  char* value;
  int ok;

  /* Parse the current argument */
  if (args != NULL) {

    args_list = strdup(args);
    key = strtok_r(args_list, ":", &save_str);
    if (key != NULL) {
      value = strtok_r(NULL, ":", &save_str);
      if (value != NULL) {
        ok = add_element_to_dict(key, value, dict_p);
        if (ok != 0) {
          print_error_and_exit("Cannot add parameter to dictionary");
        }
      } else {
        print_error_and_exit("Incorrect parameters");
      }
    } else {
      print_error_and_exit("Incorrect parameters");
    }
    free(args_list);
  }
}

void parse_perftypes_options(dictionary_t* dict_p, int argc, char **argv) {
  int c;
  int printhelp = 0;
  int nprocs, my_rank;
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  opterr = 0;

  while (1) {

    /* getopt_long stores the option index here. */
    int option_index = 0;

    c = getopt_long(argc, argv, default_opts_str, default_long_options, &option_index);

    /* Detect the end of the options. */
    if (c == -1)
      break;

    switch (c) {
    case 'p': /* list of message sizes */
      parse_parameters(optarg, dict_p);
      break;
    case 'h':
      printhelp = 1;
      print_help();
      break;
    case '?':
      break;
    }

  }

  if (printhelp) {
    MPI_Finalize();
    exit(0);
  }

  optind = 1; // reset optind to enable option re-parsing
  opterr = 1; // reset opterr to catch invalid options
}

void print_help(void) {
  int my_rank;

  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  if (my_rank == OUTPUT_ROOT_PROC) {
    printf("\nUSAGE: datatype_bench [options]\n");
    printf("options:\n");
    printf("%-40s %-40s\n", "-h", "print this help");
    printf("%-40s %-40s\n", "--params=key:value", "list of key-value pairs");

    printf("\nRequired parameters:\n");
    printf("%-40s %-40s\n", "--params=root:<process_id>", "");
    printf("%-40s %-40s\n", "--params=b:<mpi_base_datatype>",
        "Possible values: MPI_INT, MPI_CHAR, MPI_FLOAT, MPI_DOUBLE");
    printf("%-40s %-40s\n", "--params=test_type:<type>", "Possible values: datatype, pack");
    printf("%-40s %-40s\n", "--params=pattern:<test_pattern>", "Possible values: pingpong, bcast, allgather");
    printf("%-40s %-40s\n", "--params=layout:<test_layout>",
        "Possible values: tiled, block, bucket, alternating, etc.");
    printf("%-40s %-40s\n", "--params=nbytes_list:<list>", "List of integer values separated by \"/\"");
    printf("\n");

    printf("\nSpecific options for ReproMPI:\n");
    printf("%-40s %-40s\n", "--nrep=<nrep>",
            "set number of experiment repetitions");
    printf("%-40s %-40s\n %50s%s\n", "--summary=<args>",
            "list of comma-separated data summarizing methods (mean, median, min, max)", "",
            "e.g., --summary=mean,max");

    printf("\nWindow-based process synchronization options:\n");
    printf("%-40s %-40s\n", "--window-size=<win>",
            "window size in microseconds for window-based synchronization (default: 1 ms)");
    printf("%-40s %-40s\n", "--wait-time=<wait>",
            "wait time in microseconds before the start of the first window (default: 1 ms)");

    printf("\nSynchronization options for methods using a linear model of the clock drift (HCA, JK):\n");
    printf("%-40s %-40s\n", "--fitpoints=<nfit>",
            "number of fitpoints (default: 20)");
    printf("%-40s %-40s\n", "--exchanges=<nexc>",
            "number of exchanges (default: 10)\n");


    printf("\n%s \\\n%20s%s \\\n%20s%s\n",
        "EXAMPLES: mpirun -np 4 ./bin/reprompibench --params=b:MPI_INT --params=root:0 --params=test_type:datatype",
        "", "--params=pattern:bcast --params=layout:block --params=A:100 --params=B2:103 --params=B1:101",
        "", "--params=nbytes_list:1000/2000/40000 --nrep=4");
    printf("\n%10s%s \\\n%20s%s \\\n%20s%s\n\n",
        "", "mpirun -np 4 ./bin/reprompibench --params=b:MPI_CHAR --params=root:0 --params=test_type:datatype",
        "", "--params=pattern:pingpong --params=layout:tiled --params=A:100 --params=B:103",
        "", "--params=nbytes_list:1000/2000 --nrep=4 --summary=mean");

  }
}

