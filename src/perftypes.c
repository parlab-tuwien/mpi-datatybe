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
#include <assert.h>
#include <mpi.h>

#include "perftypes.h"
#include "util.h"

/* type constructors for investigating guidelines and performance */
/* types are not committed and not freed; all intermediate types are freed */



/*******************************************************/
/* basic layouts */
/*******************************************************/

int bl_basetype(dictionary_t *dict, MPI_Datatype *t, int* flags)
{
    *flags = 0;
    *flags |= PREDEFINED_DT;
    *t = get_basetype_value_from_dict("b", dict);
    //MPI_Type_dup(basetype, t);

    return MPI_SUCCESS;
}


// elements A, extent (in units of b) B
int bl_tiled(dictionary_t *dict, MPI_Datatype *t, int* flags)
{
    MPI_Datatype t1;
    MPI_Aint lb, eb;
    int A,B;
    MPI_Datatype b;

    *flags = 0;

    A = get_int_value_from_dict("A", dict);
    B = get_int_value_from_dict("B", dict);
    b = get_basetype_value_from_dict("b", dict);

    assert(A>0);
    assert(A<=B); // we allow degenerate case with A==B

    MPI_Type_get_extent(b,&lb,&eb); // get extent of basetype
    MPI_Type_contiguous(A,b,&t1);
    MPI_Type_create_resized(t1,0,B*eb,t);
    MPI_Type_free(&t1); // make sure intermediate type is eventually freed

    return MPI_SUCCESS;
}


// elements A1+A2, extent (in units of b) 2*B
int bl_bucket(dictionary_t *dict, MPI_Datatype *t, int* flags)
{
    MPI_Datatype t1;
    MPI_Aint lb, eb;
    int block[2], displ[2];
    int A1, A2, B;
    MPI_Datatype b;

    *flags = 0;

    A1 = get_int_value_from_dict("A1", dict);
    A2 = get_int_value_from_dict("A2", dict);
    B = get_int_value_from_dict("B", dict);
    b = get_basetype_value_from_dict("b", dict);

    assert(A1>0);
    assert(A2>0); // we allow degerate case with A1==A2
    assert(B>=A1&&B>=A2);

    block[0] = A1;
    block[1] = A2;
    displ[0] = 0;
    displ[1] = B;

    MPI_Type_get_extent(b,&lb,&eb); // get extent of basetype
    MPI_Type_indexed(2,block,displ,b,&t1);
    MPI_Type_create_resized(t1,0,2*B*eb,t);
    MPI_Type_free(&t1); // make sure intermediate type is eventually freed

    return MPI_SUCCESS;
}


// elements 2*A, extent (in units of b) B1+B2
int bl_block(dictionary_t *dict, MPI_Datatype *t, int* flags)
{
    MPI_Datatype t1;
    MPI_Aint lb, eb;

    int displ[2];
    int A, B1, B2;
    MPI_Datatype b;

    *flags = 0;

    A = get_int_value_from_dict("A", dict);
    B1 = get_int_value_from_dict("B1", dict);
    B2 = get_int_value_from_dict("B2", dict);
    b = get_basetype_value_from_dict("b", dict);


    assert(A>0);
    assert(B1>=A&&B2>=A);

    displ[0] = 0;
    displ[1] = B1;

    MPI_Type_get_extent(b,&lb,&eb); // get extent of basetype
    MPI_Type_create_indexed_block(2,A,displ,b,&t1);
    MPI_Type_create_resized(t1,0,(B1+B2)*eb,t);
    MPI_Type_free(&t1); // make sure intermediate type is eventually freed

    return MPI_SUCCESS;
}


// elements A1+A2, corresponding extents (in units of b) B1 and B1
int bl_alternating(dictionary_t *dict, MPI_Datatype *t, int* flags)
{
    MPI_Datatype t1;
    MPI_Aint lb, eb;
    int block[2], displ[2];
    int A1, A2, B1, B2;
    MPI_Datatype b;

    *flags = 0;

    A1 = get_int_value_from_dict("A1", dict);
    A2 = get_int_value_from_dict("A2", dict);
    B1 = get_int_value_from_dict("B1", dict);
    B2 = get_int_value_from_dict("B2", dict);
    b = get_basetype_value_from_dict("b", dict);

    assert(A1>0);
    assert(A2>0); // we allow degenerate case with A1==A2
    assert(B1>=A1 && B2>=A2);

    block[0] = A1;
    block[1] = A2;
    displ[0] = 0;
    displ[1] = B1;

    MPI_Type_get_extent(b,&lb,&eb); // get extent of basetype
    MPI_Type_indexed(2, block, displ, b, &t1);
    MPI_Type_create_resized(t1, 0, (B1 + B2) * eb, t);
    MPI_Type_free(&t1); // make sure intermediate type is eventually freed

    return MPI_SUCCESS;
}


// elements A1+A2, corresponding extents (in units of b) B1 and B1
// lower bound modified such that each unit is aligned to B1+B2
int bl_alternating_aligned(dictionary_t *dict, MPI_Datatype *t, int* flags)
{
    MPI_Datatype t1;
    MPI_Aint lb, eb;
    int block[2], displ[2];
    int A1, A2, B1, B2;
    MPI_Datatype b;

    *flags = 0;

    A1 = get_int_value_from_dict("A1", dict);
    A2 = get_int_value_from_dict("A2", dict);
    B1 = get_int_value_from_dict("B1", dict);
    B2 = get_int_value_from_dict("B2", dict);
    b = get_basetype_value_from_dict("b", dict);

    assert(A1>0);
    assert(A2>0); // we allow degenerate case with A1==A2
    assert(B1>=A1 && B2>=A2);

    block[0] = A1;
    block[1] = A2;
    displ[0] = 0;
    displ[1] = B1;

    MPI_Type_get_extent(b,&lb,&eb); // get extent of basetype
    MPI_Type_indexed(2, block, displ, b, &t1);
    MPI_Type_create_resized(t1, -B2* eb, (B1 + B2) * eb, t);
    MPI_Type_free(&t1); // make sure intermediate type is eventually freed

    return MPI_SUCCESS;
}


int bl_tiled_heterogeneous(dictionary_t *dict, MPI_Datatype *t, int* flags)
//int A, int B, int c, MPI_Datatype b[],
{
    int *block;
    MPI_Aint *displ;
    //  MPI_Datatype dtype[c];
    MPI_Aint lb, eb;
    int i;
    int A, B;
    MPI_Datatype *b;
    int c;

    *flags = 0;

    A = get_int_value_from_dict("A", dict);
    B = get_int_value_from_dict("B", dict);
    c = get_int_value_from_dict("c", dict);
    b = get_basetype_list_from_dict("blist", c, dict);

    assert(c>0);
    assert(A>0);
    assert(B>=A); // degenerate case with A==B is important!

    block = (int*)malloc(sizeof(int) * c);
    displ = (MPI_Aint*)malloc(sizeof(MPI_Aint) * c);

    for (i=0; i<c; i++) block[i] = A;
    displ[0] = 0;
    for (i=1; i<c; i++) {
        MPI_Type_get_extent(b[i-1],&lb,&eb); // get extent of basetype
        displ[i] = displ[i-1]+B*eb;
    }
    MPI_Type_create_struct(c,block,displ,b,t);

    free(b);
    free(block);
    free(displ);
    return MPI_SUCCESS;
}


// number of elements is (S1+S2)*A, extent (S1+S2)*B elements
// guideline: should behave as tiled. Hypothesis: it will not
int bl_tiled_struct(dictionary_t *dict, MPI_Datatype *t, int* flags)
//int A, int B, int S1, int S2, MPI_Datatype b,
{
    int c = 2; // can be extended later
    int block[c];
    MPI_Aint displ[c];
    MPI_Datatype dtype[c];
    MPI_Datatype t1, t2;
    MPI_Aint lb, eb;

    //int A;
    int B, S1, S2;
    MPI_Datatype b;

    *flags = 0;

    //A = get_int_value_from_dict("A", dict);
    B = get_int_value_from_dict("B", dict);
    S1 = get_int_value_from_dict("S1", dict);
    S2 = get_int_value_from_dict("S2", dict);
    b = get_basetype_value_from_dict("b", dict);

    assert(S1>0);
    assert(S2>0); // we do not exclude degenerate case with S1==S2

    MPI_Type_get_extent(b,&lb,&eb); // get extent of basetype
    //tiled(A,B,b,&t1);
    bl_tiled(dict, &t1, flags);
    MPI_Type_contiguous(S1,t1,&dtype[0]);
    MPI_Type_contiguous(S2,t1,&dtype[1]); // two different subtypes
    block[0] = 1;
    block[1] = 1;
    displ[0] = 0;
    displ[1] = S1*B*eb;
    MPI_Type_create_struct(2,block,displ,dtype,&t2);
    MPI_Type_create_resized(t2,0,(S1+S2)*B*eb,t);
    MPI_Type_free(&t1);
    MPI_Type_free(&t2);
    MPI_Type_free(&dtype[0]);
    MPI_Type_free(&dtype[1]);

    return MPI_SUCCESS;
}


// generic contiguous type
// take a basic layout with one sub-type T1 and create a longer type T2 consisting of n/A (n/typesize) many T1s
int dl_contig_type(dictionary_t *dict, MPI_Datatype *t, int* flags) {
  MPI_Datatype t1, b;
  int n;
  int typesize;
  char *sub;
  MPI_Aint lb, eb;
  int ret;

  *flags = 0;

  n = get_int_value_from_dict("n", dict);
  b = get_basetype_value_from_dict("b", dict);
  ret = get_value_from_dict(dict, "subtype", &sub);
  if (ret !=0 || sub == NULL) {
    printf("Error: basic layout %s not found\n", sub);
    exit(1);
  }

  MPI_Type_get_extent(b, &lb, &eb);

  if (strcmp(sub, "tiled") == 0) {
    bl_tiled(dict, &t1, flags);
  } else if (strcmp(sub, "bucket") == 0) {
    bl_bucket(dict, &t1, flags);
  } else if (strcmp(sub, "block") == 0) {
    bl_block(dict, &t1, flags);
  } else if (strcmp(sub, "alternating") == 0) {
    bl_alternating(dict, &t1, flags);
  } else if (strcmp(sub, "basetype") == 0) {
    bl_basetype(dict, &t1, flags);
  } else {
    printf("Error: basic layout %s not found\n", sub);
    exit(1);
  }

  MPI_Type_size(t1, &typesize);
  MPI_Type_contiguous(n*eb / typesize, t1, t);
  MPI_Type_commit(t);

  free(sub);
  return MPI_SUCCESS;
}


/*******************************************************/
/* dynamic layouts */
/*******************************************************/


// elements (n/(A1+A2))*(A1+A2), extent (n/(A1+A2))*(B+A2)
int dl_alternating_repeated(dictionary_t *dict, MPI_Datatype *t, int* flags)
{
    MPI_Aint lb, eb;
    MPI_Datatype t1;
    int block[2], displ[2];

    int n;
    int A1, A2, B;
    MPI_Datatype b;

    *flags = 0;

    A1 = get_int_value_from_dict("A1", dict);
    A2 = get_int_value_from_dict("A2", dict);
    B = get_int_value_from_dict("B", dict);
    b = get_basetype_value_from_dict("b", dict);
    n = get_int_value_from_dict("n", dict);

    assert(A1>0);
    assert(A2>0); // we allow degenerate case with A1==A2
    assert(B>=A1&&B>=A2);
    assert(n>A1+A2);

    block[0] = A1;
    block[1] = A2;
    displ[0] = 0;
    displ[1] = B;

    MPI_Type_get_extent(b,&lb,&eb); // get extent of basetype
    MPI_Type_indexed(2,block,displ,b,&t1);
    MPI_Type_contiguous(n/(A1+A2),t1,t); // question: will this be normalized?
    // hypothesis: it will not, countercheck with manually created normalform
    MPI_Type_free(&t1);

    return MPI_SUCCESS;
}


// elements (n/(A1+A2))*(A1+A2), extent (n/(A1+A2))*(B+A2)
// exactly same layout as alternatedense
int dl_alternating_struct(dictionary_t *dict, MPI_Datatype *t, int* flags)
{
    MPI_Aint lb, eb;
    //  MPI_Datatype t1;
    int block[3];
    MPI_Aint displ[3];
    MPI_Datatype dtype[3], t1;
    MPI_Aint newlb, newextent;

    int n;
    int A1, A2, B;
    MPI_Datatype b;

    *flags = 0;

    A1 = get_int_value_from_dict("A1", dict);
    A2 = get_int_value_from_dict("A2", dict);
    B = get_int_value_from_dict("B", dict);
    b = get_basetype_value_from_dict("b", dict);
    n = get_int_value_from_dict("n", dict);

    assert(A1>0);
    assert(A2>0); // we allow degenerate case with A1==A2
    assert(B>=A1&&B>=A2);
    assert(n>A1+A2);

    MPI_Type_get_extent(b,&lb,&eb); // get extent of basetype

    block[0] = A1;
    displ[0] = 0;
    dtype[0] = b;

    block[1] = 1;
    displ[1] = B*eb;
    MPI_Type_vector(n/(A2+A1)-1,(A2+A1),A2+B,b,&dtype[1]);

    block[2] = A2;
    displ[2] = (((n/(A1+A2)-1)*(B+A2))+B)*eb;
    dtype[2] = b;

    MPI_Type_create_struct(3,block,displ,dtype,&t1);
    MPI_Type_get_extent(t1,&lb,&newextent); // get extent of the struct type

    // adjust bounds
    newlb = -A2 * eb;
    MPI_Type_create_resized(t1, newlb, newextent, t);

    MPI_Type_free(&dtype[1]);
    MPI_Type_free(&t1);

    return MPI_SUCCESS;
}



/* alternative, complex descriptions of basic layouts */

// number of elements (n/A)*A, extent n/A*B
int dl_tiled_vector(dictionary_t *dict, MPI_Datatype *t, int* flags)
{
    MPI_Datatype t1;
    MPI_Aint lb, eb;

    int n;
    int A, B;
    MPI_Datatype b;

    *flags = 0;

    A = get_int_value_from_dict("A", dict);
    B = get_int_value_from_dict("B", dict);
    b = get_basetype_value_from_dict("b", dict);
    n = get_int_value_from_dict("n", dict);

    assert(n>0);
    assert(A>0);
    assert(A<=B); // we allow for degenerate case A=B
    // not required that A divides n
    // total number of elements may be less than n

    MPI_Type_get_extent(b,&lb,&eb); // get extent of basetype

    MPI_Type_vector(n/A,A,B,b,&t1);
    MPI_Type_create_resized(t1,0,(n/A)*B*eb,t);
    MPI_Type_free(&t1); // make sure intermediate type is eventually freed

    return MPI_SUCCESS;
}


// tiled layout described as a nested vector of n/(S*A) blocks of S blocks
// Can easily be normalized to flat vector.
// Condition: stride of outer vector multiple of stride of inner vector
//int vectortiled(int n, int A, int B, int S, MPI_Datatype b, MPI_Datatype *t)
int dl_vector_tiled(dictionary_t *dict, MPI_Datatype *t, int* flags)
{
  MPI_Datatype v;
  int bs;

  int n, A, B, S;
  MPI_Datatype b;

  *flags = 0;

  n = get_int_value_from_dict("n", dict);
  A = get_int_value_from_dict("A", dict);
  B = get_int_value_from_dict("B", dict);
  S = get_int_value_from_dict("S", dict);
  b = get_basetype_value_from_dict("b", dict);

  assert(A>0);
  assert(B>A);
  assert(S>1);

  MPI_Type_size(b,&bs);
  MPI_Type_vector(S,A,B,b,&v);
  MPI_Type_create_hvector(n/(S*A),1,S*B*bs,v,t);
  MPI_Type_free(&v);

  return MPI_SUCCESS;
}


// explicit description of block layout (paper)
int dl_block_indexed(dictionary_t *dict, MPI_Datatype *t, int* flags)
{
  //int index[n/A];
  int *index;
  int i;
  int s;

  int n, A, B1, B2;
  MPI_Datatype b;

  *flags = 0;

  n = get_int_value_from_dict("n", dict);
  A = get_int_value_from_dict("A", dict);
  B1 = get_int_value_from_dict("B1", dict);
  B2 = get_int_value_from_dict("B2", dict);
  b = get_basetype_value_from_dict("b", dict);

  index = (int*)calloc(n/A, sizeof(int));

  assert(A>0);
  assert(B1>=A);
  assert(B2>=A);

  index[0] = 0;
  for (i=1; i<n/A; i++) {
    s = (i%2==1) ? B1 : B2;
    index[i] = index[i-1]+s;
  }
  MPI_Type_create_indexed_block(n/A,A,index,b,t);

  free(index);

  return MPI_SUCCESS;
}


// explicit description of irregular/alternating layout (paper)
int dl_alternating_indexed(dictionary_t *dict, MPI_Datatype *t, int* flags)
{
  //int block[n];
  //int index[n];
  int *block;
  int *index;
  int i, j;

  int n, A1, A2, B1, B2;
  MPI_Datatype b;

  *flags = 0;

  n = get_int_value_from_dict("n", dict);
  A1 = get_int_value_from_dict("A1", dict);
  A2 = get_int_value_from_dict("A2", dict);
  B1 = get_int_value_from_dict("B1", dict);
  B2 = get_int_value_from_dict("B2", dict);
  b = get_basetype_value_from_dict("b", dict);

  assert(A1>0);
  assert(A2>0);
  assert(B1>=A1);
  assert(B2>=A2);

  block = (int*)calloc(n, sizeof(int));
  index = (int*)calloc(n, sizeof(int));

  block[0] = A1;
  index[0] = 0;
  i = A1; j = 1;
  while (i<n) {
    if (j%2==1) {
      block[j] = A2;
      index[j] = index[j-1]+B1; // previous block
      i += A2;
    } else {
      block[j] = A1;
      index[j] = index[j-1]+B2;
      i += A1;
    }
    j++;
  }
  if (i>n) j--;
  MPI_Type_indexed(j,block,index,b,t);

  free(block);

  return MPI_SUCCESS;
}


/* row column layouts:
   something like a row of A elements, followed by a column of width A
   with the remainder n-A elements*/

// number of elements is n (>=A), extent n*A elements
int dl_rowcol_full_indexed(dictionary_t *dict, MPI_Datatype *t, int* flags)
{
    int i;
    int n;
    int A;
    MPI_Datatype b;
    int *displ;

    *flags = 0;

    A = get_int_value_from_dict("A", dict);
    b = get_basetype_value_from_dict("b", dict);
    n = get_int_value_from_dict("n", dict);

    displ = (int*)calloc(n, sizeof(int));

    assert(A>0);
    assert(n>=A);

    for (i=0; i<A; i++) {
        displ[i] = i;
    }
    displ[A] = A;
    for (i=A+1; i<n; i++) {
        displ[i] = displ[i-1]+A;
    }
    MPI_Type_create_indexed_block(n,1,displ,b,t);

    free(displ);

    return MPI_SUCCESS;
}


int dl_rowcol_contiguous_and_indexed(dictionary_t *dict, MPI_Datatype *t, int* flags)
{
    int i;
    int n;
    int A;
    MPI_Datatype b;
    int *block;
    int *displ;

    *flags = 0;

    A = get_int_value_from_dict("A", dict);
    b = get_basetype_value_from_dict("b", dict);
    n = get_int_value_from_dict("n", dict);

    block = (int*)calloc(n, sizeof(int));
    displ = (int*)calloc(n, sizeof(int));

    assert(A>0);
    assert(n>=A);

    block[0] = A;
    displ[0] = 0;
    block[1] = 1;
    displ[1] = A;
    for (i=2; i<n-A+1; i++) {
        block[i] = 1;
        displ[i] = displ[i-1]+A;
    }
    MPI_Type_indexed(n-A+1,block,displ,b,t);

    free(block);
    free(displ);

    return MPI_SUCCESS;
}


int dl_rowcol_struct(dictionary_t *dict, MPI_Datatype *t, int* flags)
{
    MPI_Aint lb, eb;
    int block[2];
    MPI_Aint displ[2];
    MPI_Datatype dtype[2];

    int n;
    int A;
    MPI_Datatype b;

    *flags = 0;

    A = get_int_value_from_dict("A", dict);
    b = get_basetype_value_from_dict("b", dict);
    n = get_int_value_from_dict("n", dict);

    assert(A>0);
    assert(n>=A);

    MPI_Type_get_extent(b,&lb,&eb);

    block[0] = A;
    displ[0] = 0;
    dtype[0] = b;

    block[1] = 1;
    displ[1] = A*eb;
    MPI_Type_vector(n-A,1,A,b,&dtype[1]);

    MPI_Type_create_struct(2,block,displ,dtype,t);
    MPI_Type_free(&dtype[1]);

    return MPI_SUCCESS;
}


// vector with larger blocks l (fixed number or percentage of n)
// number of elements is n/(A*(l/A))*(A*(l/A)), block stride B*(l/A)
int dl_blocks(dictionary_t *dict, MPI_Datatype *t, int* flags)
{
    int n, l;
    int A, B;
    MPI_Datatype b;

    *flags = 0;

    A = get_int_value_from_dict("A", dict);
    B = get_int_value_from_dict("B", dict);
    b = get_basetype_value_from_dict("b", dict);
    n = get_int_value_from_dict("n", dict);
    l = get_int_value_from_dict("l", dict);

    assert(B>A);
    assert(l>=A);

    MPI_Type_vector(n/(A*(l/A)),(A*(l/A)),B*(l/A),b,t);

    return MPI_SUCCESS;
}


// struct equivalent with a repeated tiled layout
// layout: Tiled with A, B
//          xxxx__xxxx__ ... __xxxx
// new datatype: struct of 3 blocks with sizes: (A-A/2) + index(all remaining elements) + (A-A/2)
//          xx + index(xx__xxxx__ ... __xx) + xx
int dl_tiled_struct_indexed_all(dictionary_t *dict, MPI_Datatype *t, int* flags)
{
    MPI_Aint lb, eb;
    MPI_Datatype t1;
    int block[3];
    MPI_Aint displ[3];
    MPI_Datatype dtype[3];
    int *tmp_block;
    int *tmp_index;
    int i,j;
    int tmp_nelems, tmp_nblocks;
    MPI_Datatype tmp_type;
    MPI_Aint tmp_extent;
    int outer_block;

    int n;
    int A, B;
    MPI_Datatype b;

    *flags = 0;

    A = get_int_value_from_dict("A", dict);
    B = get_int_value_from_dict("B", dict);
    b = get_basetype_value_from_dict("b", dict);
    n = get_int_value_from_dict("n", dict);

    assert(A>=1);
    assert(B>=A);
    assert(n>A);

    MPI_Type_get_extent(b,&lb,&eb); // get extent of basetype

    outer_block = A - A/2;
    // create inner indexed datatype
    tmp_nelems = n - 2 * outer_block;
    tmp_nblocks = 2 + (tmp_nelems - 2 * (A - outer_block))/A;
    tmp_block = (int*)calloc(tmp_nblocks, sizeof(int));
    tmp_index = (int*)calloc(tmp_nblocks, sizeof(int));

    // first block in the indexed type has A-A/2 elements
    tmp_block[0] = A - outer_block;
    tmp_index[0] = 0;
    i = A - outer_block;
    j = 1;

    // inner blocks in the indexed type have A elements
    while (i < tmp_nelems - (A - outer_block)) {
        tmp_block[j] = A;

        if (j == 1) {   // the extent of the first block is B-(A-A/2)
            tmp_index[j] = tmp_index[j-1] + B - outer_block;
        }
        else {  // all other blocks have an extent of B
            tmp_index[j] = tmp_index[j-1] + B;
        }
        i += A;
        j++;
    }

    // add last block in the indexed datatype
    if (j > 1) {    // inner indexed datatype is not empty
        tmp_block[j] = A - outer_block;
        tmp_index[j] = tmp_index[j-1] + B;
        i += A - outer_block;
        j++;
    }
    else {
        tmp_block[j] = A - outer_block;
        tmp_index[j] = tmp_index[j-1] + B - outer_block;
        i += A - outer_block;
        j++;
    }
    if (i>tmp_nelems) {
        j--;
    }

    MPI_Type_indexed(j,tmp_block,tmp_index,b,&t1);
    MPI_Type_create_resized(t1, 0, (tmp_index[j-1] + tmp_block[j-1]) * eb, &tmp_type);
    MPI_Type_get_extent(tmp_type, &lb, &tmp_extent);
    free(tmp_block);
    free(tmp_index);

    // create final struct datatype
    block[0] = outer_block;
    displ[0] = 0;
    dtype[0] = b;

    block[1] = 1;
    displ[1] = outer_block * eb;
    dtype[1] = tmp_type;

    block[2] = outer_block;
    displ[2] = tmp_extent + displ[1];
    dtype[2] = b;

    //printf("j=%d tmp_nelems=%d index_extent=%d displ=%d i=%d\n", j,tmp_nelems ,tmp_extent,  displ[2], i);

    MPI_Type_create_struct(3,block,displ,dtype,t);

    MPI_Type_free(&t1);
    MPI_Type_free(&tmp_type);

    return MPI_SUCCESS;
}


// datatype similar to alternating indexed (with B2=A2 and B1+B2=B), but it contains
// a fixed number S of blocks of alternating types in the index
int bl_alternating_indexed_fixed(dictionary_t *dict, MPI_Datatype *t, int* flags)
{
    MPI_Aint lb, eb;
    //  MPI_Datatype t1;
    int *block;
    int *index;
    int i,j;

    int S;
    int A1, A2, B, B1, B2;
    MPI_Datatype b;

    *flags = 0;

    A1 = get_int_value_from_dict("A1", dict);
    A2 = get_int_value_from_dict("A2", dict);
    B = get_int_value_from_dict("B", dict);
    b = get_basetype_value_from_dict("b", dict);
    S = get_int_value_from_dict("S", dict);

    assert(A1>0);
    assert(A2>0); // we allow degenerate case with A1==A2
    assert(B>=A1 + A2);
    assert(S>=2);   // an alternating type has at least 2 blocks, of sizes A1 and A2

    MPI_Type_get_extent(b,&lb,&eb); // get extent of basetype

    // create indexed datatype (containing S blocks of alternating
    // elements - where each of them has A1 + A2 elements)
    block = (int*)calloc(S, sizeof(int));
    index = (int*)calloc(S, sizeof(int));

    B2 = A2;
    B1 = B - B2;

    // first block contains A1 elements
    block[0] = A1;
    index[0] = 0;
    i = A1;
    j = 1;

    // inner blocks are similar to a tiled type with (A1 + A2) elements and extent B
    while (i < (S - 1)* (A1+A2) - A2) {
        block[j] = A1 + A2;

        if (j == 1) {   // first block starts at extent B1
            index[j] = index[j-1] + B1 ;
        }
        else {    // all other blocks have extents of B=B1+B2
            index[j] = index[j-1] + B1 + B2;
        }
        i += A1 + A2;
        j++;
    }

    // final block contains A2 elements
    block[j] = A2;
    if (j == 1) {   // first block starts at extent B1
        index[j] = index[j-1] + B1;
    }
    else {    // all other blocks have extents of B=B1+B2
        index[j] = index[j-1] + B1 + B2;
    }
    i += A1 + A2;
    j++;

    MPI_Type_indexed(j,block,index,b,t);

    free(block);
    free(index);

    return MPI_SUCCESS;
}


// contiguous type consisting of n basetype elements grouped into alternating_indexed_fixed datatypes
int dl_contig_alternating_indexed_fixed(dictionary_t *dict, MPI_Datatype *t, int* flags)
{
    MPI_Datatype t1, t2, b;
    int n;
    int typesize;
    MPI_Aint lb, eb, newlb, newextent;
    int A2;

    *flags = 0;

    n = get_int_value_from_dict("n", dict);
    b = get_basetype_value_from_dict("b", dict);
    MPI_Type_get_extent(b, &lb, &eb);

    // need this to set the lower bound such that each block in middle part of the datatype starts at a multiple of B=B1+B2
    A2 = get_int_value_from_dict("A2", dict);

    // create simple datatype
    bl_alternating_indexed_fixed(dict, &t1, flags);
    MPI_Type_size(t1, &typesize);

    // create contiguous type with a total of n basetype elements
    MPI_Type_contiguous(n*eb / typesize, t1, &t2);
    MPI_Type_get_extent(t2, &lb, &newextent);

    // adjust bounds
    newlb = -A2 * eb;
    MPI_Type_create_resized(t2, newlb, newextent, t);

    MPI_Type_free(&t1);
    MPI_Type_free(&t2);
    return MPI_SUCCESS;
}




void instantiate_dynamic_datatype(const pattern_config_t conf,
    const dictionary_t *dict, const size_t nbytes, MPI_Datatype* t,
    int* needed_count, int* flags) {

  dictionary_t typedict;
  //char* layout;
  MPI_Datatype base;
  int i;
  size_t n;
  int typesize;
  int basesize;
  char *n_str;

  *flags = 0;

  base = get_basetype_value_from_dict("b", dict);
  MPI_Type_size(base, &basesize);

  n = nbytes / basesize;
  n_str = my_int_to_string(n);

  init_dictionary(&typedict);
  add_element_to_dict("n", n_str, &typedict);
  copy_dict_entry("b", dict, &typedict);
  *needed_count = 1;

  /* these params are different for each type
   * thus, copy them into separate dict and then call create_datatype
   */
  if( conf.dt_parameters != NULL ) {
    for(i=0; i<conf.nb_params; i++) {
      //printf("copy params %s\n", conf.dt_parameters[i]);
      copy_dict_entry(conf.dt_parameters[i], dict, &typedict);
    }
  } else {
    // this should only be for contiguous type
    char *subtype;
    int found = 0;
    int ret;

    // subtype needed later, keep it
    copy_dict_entry("subtype", dict, &typedict);

    // now check the subtype and copy the right parameters
    ret = get_value_from_dict(dict, "subtype", &subtype);
    if (ret !=0 || subtype == NULL ) {
      printf("\nError: required parameter \"%s\" is not specified. \n",
          "subtype");
      exit(1);
    }

    for(i=0; i<N_LAYOUTS; i++) {
      if( strcmp(layout_list[i].name, subtype) == 0) {
        int j;
        // copy the params of that specific subtype
        for(j=0; j<layout_list[i].nb_params; j++) {
          copy_dict_entry(layout_list[i].dt_params[j], dict, &typedict);
        }
        found = 1;
        break;
      }
    }

    if (found == 0) {
      printf("\nError: layout description for \"%s\" is not found. \n",
          subtype);
      exit(1);
    }

    free(subtype);
  }


  conf.create_datatype(&typedict, t, flags);
  MPI_Type_commit(t);

  // check whether t is larger than nbytes
  // if so, set needed_count to 0
  MPI_Type_size(*t, &typesize);
  if( typesize > nbytes ) {
    fprintf(stderr, "WARNING: type is larger than nbytes (%d > %ld)\n", typesize, nbytes);
    *needed_count = 0;
  }

  free(n_str);
  cleanup_dictionary(&typedict);
}


// struct type equivalent with a repeated tiled layout
// layout: Tiled with A, B, S
//          xxxx__xxxx__ ... __xxxx
// new datatype: struct of 4 blocks with sizes: 1 + (n-1) * S * A + (A-1) + (S-1)*A
//          x + [ indexed(xxx__xxxx__ ... __x) + ... + indexed(xxx__xxxx__ ... __x) ] + xxx__ + [ (xxxx__) + (xxxx__) + ... + (xxxx__) ]
// where n = total number of blocks of size S * A
int dl_tiled_struct_indexed_Sblocks(dictionary_t *dict, MPI_Datatype *t, int* flags) {
  MPI_Datatype TiledStructIndex;
  MPI_Datatype it, aatiled;
  int A, B, S, N;
  MPI_Aint lb, eb;
  MPI_Datatype BASETYPE, b;
  int *bbl, i;
  int *bix;
  int tsibl[4];
  MPI_Aint tsidi[4];
  MPI_Datatype tsidt[4];
  int n;

  *flags = 0;
  N = get_int_value_from_dict("n", dict);
  BASETYPE = get_basetype_value_from_dict("b", dict);
  MPI_Type_get_extent(BASETYPE, &lb, &eb);

  A = get_int_value_from_dict("A", dict);
  B = get_int_value_from_dict("B", dict);
  S = get_int_value_from_dict("S", dict);

  assert(A>=2);
  assert(B>=A);
  assert(N>=A*S);

  MPI_Type_contiguous(A, BASETYPE, &b);
  MPI_Type_create_resized(b, 0, B * eb, &aatiled);
  MPI_Type_free(&b);

  bix = (int*) malloc((S + 1) * sizeof(int));
  bbl = (int*) malloc((S + 1) * sizeof(int));
  bix[0] = 0;
  bbl[0] = A - 1;
  for (i = 1; i < S; i++) {
    bix[i] = i * B - 1;
    bbl[i] = A;
  }
  bix[i] = i * B - 1;
  bbl[i] = 1;
  MPI_Type_indexed(S + 1, bbl, bix, BASETYPE, &it);
  free(bix);
  free(bbl);

  n = N / (A * S);

  tsibl[0] = 1;
  tsidi[0] = 0 * eb;
  tsidt[0] = BASETYPE;
  tsibl[1] = n - 1;
  tsidi[1] = 1 * eb;
  tsidt[1] = it;
  tsibl[2] = A - 1;
  tsidi[2] = (B * S * (n - 1) + 1) * eb;
  tsidt[2] = BASETYPE;
  tsibl[3] = S - 1;
  tsidi[3] = (B * S * (n - 1) + B) * eb;
  tsidt[3] = aatiled;

  MPI_Type_create_struct(4, tsibl, tsidi, tsidt, &b);
  MPI_Type_create_resized(b, 0, (N / A) * B * eb, &TiledStructIndex);
  MPI_Type_free(&it);
  MPI_Type_free(&b);
  MPI_Type_free(&aatiled);

  *t = TiledStructIndex;
  return MPI_SUCCESS;
}

