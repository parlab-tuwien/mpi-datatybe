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


#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <mpi.h>

int to_int(char* string) {
    char* end = NULL;
    int result;
    errno = 0;

    if (string != NULL) {
        result = strtol(string, &end, 10);
    }

    if((string == NULL) || (errno != 0))
    {
        printf("\nError: unable to convert %s to int\n", string);
        exit(1);
    }

    return result;

}

MPI_Datatype to_basetype(char* string) {
    MPI_Datatype d = MPI_CHAR;
    int ok = 0;

    if (string != NULL && strlen(string) > 0) {
        if (strcmp("MPI_CHAR", string) == 0) {
            d = MPI_CHAR;
            ok =1;
        }
        if (strcmp("MPI_INT", string) == 0) {
            d = MPI_INT;
            ok =1;
        }
        if (strcmp("MPI_FLOAT", string) == 0) {
            d = MPI_FLOAT;
            ok =1;
        }
        if (strcmp("MPI_DOUBLE", string) == 0) {
            d = MPI_DOUBLE;
            ok =1;
        }
        if (strcmp("MPI_SHORT", string) == 0) {
            d = MPI_SHORT;
            ok =1;
        }
        if (strcmp("MPI_BYTE", string) == 0) {
            d = MPI_BYTE;
            ok =1;
        }
    }


    if (!ok) {
        printf("\nError: unable to convert %s to base MPI_Datatype\n", string);
        exit(1);
    }

    return d;
}


MPI_Datatype* to_basetype_list(char* string, int n_types) {
    MPI_Datatype* d = NULL;
    char* args_tok;
    char* save_str = NULL;
    char* s;
    int i=0;
    int ok = 1;

    s = strdup(string);
    if (n_types > 0) {
        d = (MPI_Datatype*)malloc(sizeof(MPI_Datatype) * n_types);

        args_tok = strtok_r(string, "/", &save_str);
        while (args_tok != NULL) {
            if (i == n_types) {
                ok = 0;
                break;
            }
            d[i] = to_basetype(args_tok);
            i++;
            args_tok = strtok_r(NULL, "/", &save_str);
        }
    }
    else {
        ok = 0;
    }

    if (i != n_types) {
        ok = 0;
    }

    if (!ok) {
        printf("\nError: unable to convert %s to (slash-separated) list of %d MPI_Datatypes\n", s, n_types);
        exit(1);
    }

    free(s);
    return d;
}


int* to_int_list(char* string, int n) {
    int* d = NULL;
    char* args_tok;
    char* save_str = NULL;
    char* s;
    int i=0;
    int ok = 1;

    s = strdup(string);
    if (n > 0) {
        d = (int*)malloc(sizeof(int) * n);

        args_tok = strtok_r(string, "/", &save_str);
        while (args_tok != NULL) {
            if (i == n) {
                ok = 0;
                break;
            }
            d[i] = to_int(args_tok);
            i++;
            args_tok = strtok_r(NULL, "/", &save_str);
        }
    }
    else {
        ok = 0;
    }

    if (i != n) {
        ok = 0;
    }

    if (!ok) {
        printf("\nError: unable to convert %s to (slash-separated) list of %d int values\n", s, n);
        exit(1);
    }

    free(s);
    return d;
}




char** to_string_list(char* string, int n) {
    char** list = NULL;
    char* args_tok;
    char* save_str = NULL;
    char* s;
    int i=0;
    int ok = 1;

    s = strdup(string);
    if (n > 0) {
        list = (char**)malloc(sizeof(char*) * n);

        args_tok = strtok_r(string, "/", &save_str);
        while (args_tok != NULL) {
            if (i == n) {
                ok = 0;
                break;
            }
            list[i] = strdup(args_tok);
            i++;
            args_tok = strtok_r(NULL, "/", &save_str);
        }
    }
    else {
        ok = 0;
    }

    if (i != n) {
        ok = 0;
    }

    if (!ok) {
        printf("\nError: unable to convert %s to (slash-separated) list of %d int values\n", s, n);
        exit(1);
    }

    free(s);
    return list;
}

