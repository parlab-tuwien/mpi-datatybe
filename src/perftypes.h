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


#ifndef PERFTYPES_H_
#define PERFTYPES_H_

//#include "data_structures.h"
#include "datatypes_bench.h"
#include "dictionary/keyvalue_store.h"


typedef enum DatatypeFlags {
    PREDEFINED_DT = 0x1
} dt_flag_t;


/***************************************************************/
/* basic layouts */
/***************************************************************/

// MPI base datatype
int bl_basetype(dictionary_t *dict, MPI_Datatype *t, int* flags);

// elements A, extent (in units of b) B
int bl_tiled(dictionary_t *dict, MPI_Datatype *t, int* flags);

// elements A1+A2, extent (in units of b) 2*B
int bl_bucket(dictionary_t *dict, MPI_Datatype *t, int* flags);

// elements 2*A, extent (in units of b) B1+B2
int bl_block(dictionary_t *dict, MPI_Datatype *t, int* flags);

// elements A1+A2, corresponding extents (in units of b) B1 and B1
int bl_alternating(dictionary_t *dict, MPI_Datatype *t, int* flags);

// other layout
int bl_tiled_heterogeneous(dictionary_t *dict, MPI_Datatype *t, int* flags);


// datatype similar to alternating indexed (with B2=A2 and B1+B2=B), but it contains
// a fixed number S of blocks of alternating types in the index
int bl_alternating_indexed_fixed(dictionary_t *dict, MPI_Datatype *t, int* flags);

/***************************************************************/
/* dynamic layouts */
/***************************************************************/

/*
 * generic contiguous type
 * take a basic layout with one sub-type T1 and create a longer type T2 consisting of n/A (n/typesize) many T1s
 */
int dl_contig_type(dictionary_t *dict, MPI_Datatype *t, int* flags);

// elements (n/(A1+A2))*(A1+A2), extent (n/(A1+A2))*(B+A2)
int dl_alternating_repeated(dictionary_t *dict, MPI_Datatype *t, int* flags);

// elements (n/(A1+A2))*(A1+A2), extent (n/(A1+A2))*(B+A2)
// exactly same layout as alternatedense
int dl_alternating_struct(dictionary_t *dict, MPI_Datatype *t, int* flags);

/* alternative, complex descriptions of basic layouts */

// number of elements (n/A)*A, extent n/A*B
int dl_tiled_vector(dictionary_t *dict, MPI_Datatype *t, int* flags);

// number of elements is (S1+S2)*A, extent (S1+S2)*B elements
// guideline: should behave as tiled. Hypothesis: it will not
int bl_tiled_struct(dictionary_t *dict, MPI_Datatype *t, int* flags);

// tiled layout described as a nested vector of n/(S*A) blocks of S blocks
// Can easily be normalized to flat vector.
// Condition: stride of outer vector multiple of stride of inner vector
int dl_vector_tiled(dictionary_t *dict, MPI_Datatype *t, int* flags);

// explicit description of irregular/alternating layout (paper)
int dl_alternating_indexed(dictionary_t *dict, MPI_Datatype *t, int* flags);

// explicit description of block layout (paper)
int dl_block_indexed(dictionary_t *dict, MPI_Datatype *t, int* flags);

/* row column layouts:
   something like a row of B elements, followed by a column of width B
   with the remainder n-B elements*/

// number of elements is n (>=B), extent n*B elements
int dl_rowcol_full_indexed(dictionary_t *dict, MPI_Datatype *t, int* flags);

int dl_rowcol_contiguous_and_indexed(dictionary_t *dict, MPI_Datatype *t, int* flags);

int dl_rowcol_struct(dictionary_t *dict, MPI_Datatype *t, int* flags);

// vector with larger blocks l (fixed number or percentage of n)
// number of elements is n/(A*(l/A))*(A*(l/A)), block stride B*(l/A)
int dl_blocks(dictionary_t *dict, MPI_Datatype *t, int* flags);


// struct equivalent with a repeated tiled layout
// layout: Tiled with A, B
//          xxxx__xxxx__ ... __xxxx
// new datatype: struct of 3 blocks with sizes: (A-A/2) + index(all remaining elements) + (A-A/2)
//          xx + index(xx__xxxx__ ... __xx) + xx
int dl_tiled_struct_indexed_all(dictionary_t *dict, MPI_Datatype *t, int* flags);
int dl_contig_alternating_indexed_fixed(dictionary_t *dict, MPI_Datatype *t, int* flags);
int bl_alternating_aligned(dictionary_t *dict, MPI_Datatype *t, int* flags);

// struct type equivalent with a repeated tiled layout
// layout: Tiled with A, B, S
//          xxxx__xxxx__ ... __xxxx
// new datatype: struct of 4 blocks with sizes: 1 + (n-1) * S * A + (A-1) + (S-1)*A
//          x + [ indexed(xxx__xxxx__ ... __x) + ... + indexed(xxx__xxxx__ ... __x) ] + xxx__ + [ (xxxx__) + (xxxx__) + ... + (xxxx__) ]
// where n = total number of blocks of size S * A
int dl_tiled_struct_indexed_Sblocks(dictionary_t *dict, MPI_Datatype *t, int* flags);

/* instantiates each dl_* datatype */
void instantiate_dynamic_datatype(const pattern_config_t conf,
    const dictionary_t *dict, const size_t nbytes, MPI_Datatype* t,
    int* needed_count, int* flags);


#endif /* PERFTYPES_H_ */










