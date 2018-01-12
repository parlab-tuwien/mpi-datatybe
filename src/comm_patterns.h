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


#ifndef COMM_PATTERNS_H_
#define COMM_PATTERNS_H_

#include "datatypes_bench.h"
#include "dictionary/keyvalue_store.h"

// between rank 0 and 1 (try even-odd?)
int pingpongpattern(pattern_config_t conf, dictionary_t *dict);

int bcastpattern(pattern_config_t conf, dictionary_t *dict);

// n: max block size in bytes
int allgatherpattern(pattern_config_t conf, dictionary_t *dict);

/* Dynamic patterns: all data are represented by the datatype, counts are 1 */
int pingpongpattern_dynamictype(pattern_config_t conf, dictionary_t *dict);
int bcastpattern_dynamictype(pattern_config_t conf, dictionary_t *dict);
int allgatherpattern_dynamictype(pattern_config_t conf, dictionary_t *dict);

#endif /* COMM_PATTERNS_H_ */










