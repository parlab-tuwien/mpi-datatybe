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

#include "comm_patterns.h"
#include "perftypes.h"
#include "util.h"
//@ add_includes

//@ declare_variables



/* templates for what to benchmark */
#define TYPETAG 12345

static const int PROC1 = 0;
static const int PROC2 = 1;

static const int CACHE_LINE_SIZE = 64;

static const char* PATTERN_DYNAMIC = "dynamic";
static const char* PATTERN_BASIC = "basic";


void send_receive_datatype(int rank, void* sendbuf, void* recvbuf, int c,
        int process1, int process2, MPI_Datatype type, MPI_Comm comm) {

    //@ set test_type="datatype"

    //@ initialize_timestamps t1
    //@ initialize_timestamps t2

    //@ start_measurement_loop

    //@ start_sync
    if (rank == process1) {
        //@ measure_timestamp t1
        MPI_Send(sendbuf,c,type,process2,TYPETAG, comm);
        MPI_Recv(recvbuf,c,type,process2,TYPETAG, comm, MPI_STATUS_IGNORE);
        //@ measure_timestamp t2

    } else if (rank == process2) {

        //@ measure_timestamp t1
        MPI_Recv(recvbuf,c,type,process1,TYPETAG, comm, MPI_STATUS_IGNORE);
        MPI_Send(sendbuf,c,type,process1,TYPETAG, comm);
        //@ measure_timestamp t2
    }
    //@ stop_sync
    //@stop_measurement_loop

    //@ print_runtime_array name=runtime end_time=t2 start_time=t1 type=reduce op=max nbytes=nbytes_str typeextent=derivedtype_extent typesize=derivedtype_size realsize=real_size count=c

    //@cleanup_variables
}


void send_receive_pack(int rank, void* sendbuf, void* recvbuf, int c,
        int process1, int process2, MPI_Datatype type, MPI_Comm comm) {

    int position = 0;
    int packsize; // int: mistake in standard?
    void *packbuf;

    MPI_Pack_size(c,type, comm, &packsize); // not to measure

    //packbuf = malloc(packsize);
    posix_memalign(&packbuf, CACHE_LINE_SIZE, packsize);
    assert(packbuf!=NULL);

    //@ set test_type="pack"

    //@ initialize_timestamps t1
    //@ initialize_timestamps t2

    //@ start_measurement_loop

    //@ start_sync
    if (rank == process1) {
        position = 0;

        //@ measure_timestamp t1
        MPI_Pack(sendbuf, c, type, packbuf, packsize, &position, comm);

        MPI_Send(packbuf, packsize, MPI_PACKED, process2,TYPETAG, comm);
        MPI_Recv(packbuf, packsize, MPI_PACKED, process2, TYPETAG,comm, MPI_STATUS_IGNORE);
        position = 0;
        MPI_Unpack(packbuf, packsize, &position, recvbuf, c, type, comm);
        //@ measure_timestamp t2

    } else if (rank == process2) {
        position = 0;

        //@ measure_timestamp t1
        MPI_Recv(packbuf, packsize, MPI_PACKED, process1, TYPETAG, comm, MPI_STATUS_IGNORE);
        MPI_Unpack(packbuf, packsize, &position, recvbuf, c, type, comm);
        position = 0;
        MPI_Pack(recvbuf, c, type, packbuf, packsize, &position, comm);
        MPI_Send(packbuf, packsize, MPI_PACKED, process1, TYPETAG, comm);
        //@ measure_timestamp t2
    }
    //@ stop_sync
    //@stop_measurement_loop

    //@ print_runtime_array name=runtime end_time=t2 start_time=t1 type=reduce op=max nbytes=nbytes_str typeextent=derivedtype_extent typesize=derivedtype_size realsize=real_size count=c

    //@cleanup_variables
    free(packbuf);
}



void bcast_pack(int rank, void* bcastbuf, int c,
        MPI_Datatype type, int root_proc, MPI_Comm comm) {
    int position = 0;
    void *packbuf;
    int packsize; // int: mistake in standard?

    //@ set test_type="pack"

    //@ initialize_timestamps t1
    //@ initialize_timestamps t2

    MPI_Pack_size(c, type, comm, &packsize); // not to measure
    //packbuf = malloc(packsize);
    posix_memalign(&packbuf, CACHE_LINE_SIZE, packsize);
    assert(packbuf!=NULL);

    //@ start_measurement_loop

    //@ start_sync
    if (rank == root_proc) {
        position = 0;
        //@ measure_timestamp t1
        MPI_Pack(bcastbuf, c, type, packbuf, packsize, &position, comm);
        MPI_Bcast(packbuf, packsize, MPI_PACKED, root_proc, comm);
        //@ measure_timestamp t2

    } else {
        //@ measure_timestamp t1
        MPI_Bcast(packbuf, packsize, MPI_PACKED, root_proc, comm);
        position = 0;
        MPI_Unpack((char*)packbuf, packsize, &position, bcastbuf, c, type, comm);
        //@ measure_timestamp t2
    }

    //@ stop_sync
    //@stop_measurement_loop

    //@ print_runtime_array name=runtime end_time=t2 start_time=t1 type=reduce op=max nbytes=nbytes_str typeextent=derivedtype_extent typesize=derivedtype_size realsize=real_size count=c

    //@cleanup_variables
    free(packbuf);
}

void bcast_datatype(int rank, void* bcastbuf, int c,
        MPI_Datatype type, int root_proc, MPI_Comm comm) {

    //@ set test_type="datatype"

    //@ initialize_timestamps t1
    //@ initialize_timestamps t2

    //@ start_measurement_loop
    //@ start_sync
    //@ measure_timestamp t1
    MPI_Bcast(bcastbuf, c, type, root_proc, comm);
    //@ measure_timestamp t2

    //@ stop_sync
    //@stop_measurement_loop

    //@ print_runtime_array name=runtime end_time=t2 start_time=t1 type=reduce op=max nbytes=nbytes_str typeextent=derivedtype_extent typesize=derivedtype_size realsize=real_size count=c

    //@cleanup_variables

}


void allgather_pack(int rank, void* sendbuf, void* recvbuf, int c,
        MPI_Datatype type, int root_proc, MPI_Comm comm) {

    int position = 0;
    int j, size;
    int packsize; // int: mistake in standard?
    void *sendpack, *recvpack;
    MPI_Aint lb, extent;

    MPI_Comm_size(comm, &size);

    MPI_Pack_size(c, type, comm, &packsize); // not to measure
    //sendpack = malloc(packsize);
    posix_memalign(&sendpack, CACHE_LINE_SIZE, packsize);
    assert(sendpack!=NULL);
    //recvpack = malloc(packsize*size);
    posix_memalign(&recvpack, CACHE_LINE_SIZE, (size_t)packsize * size);
    assert(recvpack!=NULL);

    MPI_Type_get_extent(type, &lb, &extent);

    //@ initialize_timestamps t1
    //@ initialize_timestamps t2

    //@ start_measurement_loop

    //@ start_sync
    //@ measure_timestamp t1
    position = 0;
    MPI_Pack(sendbuf, c, type, sendpack, packsize, &position, comm);
    MPI_Allgather(sendpack, packsize, MPI_PACKED,
            recvpack, packsize, MPI_PACKED, comm);
    // MPI standard: well defined how packunits are concatenated?
    size_t offset = 0;
    for (j=0; j<size; j++) {
      position = 0;
      MPI_Unpack((char*)recvpack + offset, packsize, &position,
		 (char*)recvbuf + j*c*extent, c, type, comm);
      offset += packsize;

    }
    //@ measure_timestamp t2
    //@ stop_sync
    //@stop_measurement_loop

    //@ print_runtime_array name=runtime end_time=t2 start_time=t1 type=reduce op=max nbytes=nbytes_str typeextent=derivedtype_extent typesize=derivedtype_size realsize=real_size count=c

    //@cleanup_variables
    free(sendpack);
    free(recvpack);
}


void allgather_datatype(int rank, void* sendbuf, void* recvbuf, int c,
        MPI_Datatype type, int root_proc, MPI_Comm comm) {

    //@ set test_type="datatype"

    //@ initialize_timestamps t1
    //@ initialize_timestamps t2

    //@ start_measurement_loop

    //@ start_sync
    //@ measure_timestamp t1
    MPI_Allgather(sendbuf, c, type, recvbuf, c, type, comm);

    //@ measure_timestamp t2
    //@ stop_sync
    //@stop_measurement_loop

    //@ print_runtime_array name=runtime end_time=t2 start_time=t1 type=reduce op=max nbytes=nbytes_str typeextent=derivedtype_extent typesize=derivedtype_size realsize=real_size count=c

    //@cleanup_variables
}


// between rank 0 and 1 (try even-odd?)
int pingpongpattern(pattern_config_t conf, dictionary_t *dict)
{
    int size, rank;
    int i;
    size_t nn;
    string_array_t* nbytes_list = NULL;
    void *sendbuf, *recvbuf;
    MPI_Datatype type;
    int c, c0;
    size_t count;

    MPI_Aint lb, extent;
    int typesize;
    char *typesize_str, *real_size_str, *extent_str;
    int flags;

    MPI_Comm_size(conf.comm,&size);
    MPI_Comm_rank(conf.comm,&rank);

    //@ global pattern_type=PATTERN_BASIC

    nbytes_list = get_string_array_from_dict("nbytes_list", dict);

    // create the datatype:
    conf.create_datatype(dict, &type, &flags);

    if ((flags & PREDEFINED_DT) == 0) { // commit derived datatypes
        MPI_Type_commit(&type);
    }

    MPI_Type_get_extent(type,&lb,&extent);
    //MPI_Type_get_true_extent(type,&lb,&extent); // very careful here!
    MPI_Type_size(type,&typesize);

    // time this (but not with simple INC)
    c0 = -1;
    for (i=0; i<nbytes_list->n_elems; i++) {
        count = atol(nbytes_list->elements[i]);
        c = (count/typesize);
        if (c==c0) {
            continue;
        }
        c0 = c;

        nn = (count/typesize)*extent; // effective buffer size

        //sendbuf = malloc(nn);
        posix_memalign(&sendbuf, CACHE_LINE_SIZE, nn);
        assert(sendbuf!=NULL);
        //recvbuf = malloc(nn);
        posix_memalign(&recvbuf, CACHE_LINE_SIZE, nn);
        assert(recvbuf!=NULL);


        /* this is needed to avoid mem leaks */
        typesize_str  = my_int_to_string(typesize);
        extent_str  = my_int_to_string(extent);
        real_size_str = my_int_to_string(c*typesize);

        //@ set nbytes_str=nbytes_list->elements[i]
        //@ set derivedtype_size=typesize_str
        //@ set real_size=real_size_str
        //@ set derivedtype_extent=extent_str

        if (conf.use_datatype) {
            send_receive_datatype(rank, sendbuf, recvbuf, c, PROC1, PROC2, type, conf.comm);
        }
        else {
            send_receive_pack(rank, sendbuf, recvbuf, c, PROC1, PROC2, type, conf.comm);
        }

        free(typesize_str);
        free(extent_str);
        free(real_size_str);

        free(sendbuf);
        free(recvbuf);
    }

    if ((flags & PREDEFINED_DT) == 0) { // free derived datatypes
        MPI_Type_free(&type);
    }

    for (i=0; i<nbytes_list->n_elems; i++) {
        free(nbytes_list->elements[i]);
    }
    free(nbytes_list->elements);
    free(nbytes_list);

    return MPI_SUCCESS; // no...
}

int bcastpattern(pattern_config_t conf, dictionary_t *dict)
{
    int size, rank;
    int i;
    size_t nn;
    void *bcastbuf;
    MPI_Datatype type;
    int c, c0;
    size_t  count;
    MPI_Aint lb, extent;
    int typesize;
    string_array_t* nbytes_list = NULL;
    char *typesize_str, *real_size_str, *extent_str;
    int flags;

    MPI_Comm_size(conf.comm,&size);
    MPI_Comm_rank(conf.comm,&rank);

    //@ global pattern_type=PATTERN_BASIC

    nbytes_list = get_string_array_from_dict("nbytes_list", dict);

    // create datatype
    conf.create_datatype(dict, &type, &flags);

    if ((flags & PREDEFINED_DT) == 0) { // commit derived datatypes
        MPI_Type_commit(&type);
    }

    MPI_Type_get_extent(type,&lb,&extent);
    //MPI_Type_get_true_extent(type,&lb,&extent); // very careful here!
    MPI_Type_size(type,&typesize);

    // time this (but not with simple INC)
    c0 = -1;
    for (i=0; i<nbytes_list->n_elems; i++) {
        count = atol(nbytes_list->elements[i]);
        c = (count/typesize);
        if (c==c0) {
            continue;
        }
        c0 = c;
        if( c <= 0 ) {
          fprintf(stderr, "count=%ld typesize=%d invalid...skipping case\n", count, typesize);
          continue;
        }

        nn = (count/typesize)*extent; // effective buffer size
        // bcastbuf = malloc(nn);
        posix_memalign(&bcastbuf, CACHE_LINE_SIZE, nn);
        assert(bcastbuf!=NULL);

        /* this is needed to avoid mem leaks */
        typesize_str  = my_int_to_string(typesize);
        extent_str  = my_int_to_string(extent);
        real_size_str = my_int_to_string(c*typesize);

        //@ set nbytes_str=nbytes_list->elements[i]
        //@ set derivedtype_size=typesize_str
        //@ set real_size=real_size_str
        //@ set derivedtype_extent=extent_str

        if (conf.use_datatype) {
            bcast_datatype(rank, bcastbuf, c, type, conf.root_proc, conf.comm);
        }
        else {
            bcast_pack(rank, bcastbuf, c, type, conf.root_proc, conf.comm);
        }

        free(typesize_str);
        free(extent_str);
        free(real_size_str);

        free(bcastbuf);
    }

    if ((flags & PREDEFINED_DT) == 0) { // free derived datatypes
        MPI_Type_free(&type);
    }

    for (i=0; i<nbytes_list->n_elems; i++) {
        free(nbytes_list->elements[i]);
    }
    free(nbytes_list->elements);
    free(nbytes_list);

    return MPI_SUCCESS; // no...
}

// n: max block size in bytes
int allgatherpattern(pattern_config_t conf, dictionary_t *dict)
{
    int size, rank;
    int i;
    size_t nn;
    //int n;
    void *sendbuf, *recvbuf;
    MPI_Datatype type;
    int c, c0;
    size_t count;
    string_array_t* nbytes_list = NULL;

    MPI_Aint lb, extent;
    int typesize;
    char *typesize_str, *real_size_str, *extent_str;
    int flags;

    MPI_Comm_size(conf.comm,&size);
    MPI_Comm_rank(conf.comm,&rank);

    //@ global pattern_type=PATTERN_BASIC

    nbytes_list = get_string_array_from_dict("nbytes_list", dict);

    // create datatype
    conf.create_datatype(dict, &type, &flags);

    if ((flags & PREDEFINED_DT) == 0) { // commit derived datatypes
        MPI_Type_commit(&type);
    }

    MPI_Type_get_extent(type,&lb,&extent);
    //MPI_Type_get_true_extent(type,&lb,&extent); // very careful here!
    MPI_Type_size(type,&typesize);

    // time this (but not with simple INC)
    c0 = -1;
    for (i=0; i<nbytes_list->n_elems; i++) {
        count = atol(nbytes_list->elements[i]);
        c = (count/typesize);
        if (c==c0) {
            continue;
        }
        c0 = c;
        if( c <= 0 ) {
          fprintf(stderr, "count=%ld typesize=%d invalid...skipping case\n", count, typesize);
          continue;
        }

        nn = c*extent; // effective buffer size
        //sendbuf = malloc(nn);
        posix_memalign(&sendbuf, CACHE_LINE_SIZE, nn);
        assert(sendbuf!=NULL);
        //recvbuf = malloc(nn * size);
        posix_memalign(&recvbuf, CACHE_LINE_SIZE, nn * size);
        assert(recvbuf!=NULL);

        /* this is needed to avoid mem leaks */
        typesize_str  = my_int_to_string(typesize);
        extent_str  = my_int_to_string(extent);
        real_size_str = my_int_to_string(c*typesize);

        //@ set nbytes_str=nbytes_list->elements[i]
        //@ set derivedtype_size=typesize_str
        //@ set real_size=real_size_str
        //@ set derivedtype_extent=extent_str

        if (conf.use_datatype) {
            allgather_datatype(rank, sendbuf, recvbuf, c, type, conf.root_proc, conf.comm);
        }
        else {
            allgather_pack(rank, sendbuf, recvbuf, c, type, conf.root_proc, conf.comm);
        }

        free(typesize_str);
        free(extent_str);
        free(real_size_str);

        free(sendbuf);
        free(recvbuf);
    }

    if ((flags & PREDEFINED_DT) == 0) { // free derived datatypes
        MPI_Type_free(&type);
    }

    for (i=0; i<nbytes_list->n_elems; i++) {
        free(nbytes_list->elements[i]);
    }
    free(nbytes_list->elements);
    free(nbytes_list);

    return MPI_SUCCESS; // no...
}


/* Dynamic patterns: all data are represented by the datatype, counts are 1 */

int pingpongpattern_dynamictype(pattern_config_t conf, dictionary_t *dict)
{
    int size, rank;
    int i;
    size_t nn;
    void *sendbuf, *recvbuf;
    MPI_Datatype type;
    int c;
    size_t c0;
    MPI_Aint lb, extent;
    int typesize;
    string_array_t* nbytes_list = NULL;
    size_t nbytes;
    char *typesize_str, *real_size_str, *extent_str;
    int flags;

    MPI_Comm_size(conf.comm,&size);
    MPI_Comm_rank(conf.comm,&rank);

    //@ global pattern_type=PATTERN_DYNAMIC

    nbytes_list = get_string_array_from_dict("nbytes_list", dict);

    // time this (but not with simple INC)
    c0 = -1;
    for (i=0; i<nbytes_list->n_elems; i++) {
        nbytes = atol(nbytes_list->elements[i]);

        // create datatype
        instantiate_dynamic_datatype(conf, dict, nbytes, &type, &c, &flags);
        if (nbytes==c0) {
            continue;
        }
        c0 = nbytes;


        MPI_Type_get_extent(type,&lb,&extent);
        MPI_Type_size(type,&typesize);

        nn = c * extent; // effective buffer size
        //sendbuf = malloc(nn);
        posix_memalign(&sendbuf, CACHE_LINE_SIZE, nn);
        assert(sendbuf!=NULL);
        //recvbuf = malloc(nn);
        posix_memalign(&recvbuf, CACHE_LINE_SIZE, nn);
        assert(recvbuf!=NULL);

        /* this is needed to avoid mem leaks */
        typesize_str  = my_int_to_string(typesize);
        extent_str  = my_int_to_string(extent);
        real_size_str = my_int_to_string(c*typesize);

        //@ set nbytes_str=nbytes_list->elements[i]
        //@ set derivedtype_size=typesize_str
        //@ set real_size=real_size_str
        //@ set derivedtype_extent=extent_str

        if (conf.use_datatype) {
            send_receive_datatype(rank, sendbuf, recvbuf, c, PROC1, PROC2, type, conf.comm);
        }
        else {
            send_receive_pack(rank, sendbuf, recvbuf, c, PROC1, PROC2, type, conf.comm);
        }

        free(typesize_str);
        free(extent_str);
        free(real_size_str);

        free(sendbuf);
        free(recvbuf);
        if ((flags & PREDEFINED_DT) == 0) { // free derived datatypes
            MPI_Type_free(&type);
        }
    }


    for (i=0; i<nbytes_list->n_elems; i++) {
        free(nbytes_list->elements[i]);
    }
    free(nbytes_list->elements);
    free(nbytes_list);

    return MPI_SUCCESS; // no...
}



int bcastpattern_dynamictype(pattern_config_t conf, dictionary_t *dict)
{
    int size, rank;
    int i;
    size_t nn;
    void *bcastbuf;
    MPI_Datatype type;
    int c;
    size_t c0;
    MPI_Aint lb, extent;
    int typesize;
    string_array_t* nbytes_list = NULL;
    size_t nbytes;
    char *typesize_str, *real_size_str, *extent_str;
    int flags;

    MPI_Comm_size(conf.comm,&size);
    MPI_Comm_rank(conf.comm,&rank);

    //@ global pattern_type=PATTERN_DYNAMIC

    nbytes_list = get_string_array_from_dict("nbytes_list", dict);

    // time this (but not with simple INC)
    c0 = -1;
    for (i=0; i<nbytes_list->n_elems; i++) {
        nbytes = atol(nbytes_list->elements[i]);

        // create datatype
        instantiate_dynamic_datatype(conf, dict, nbytes, &type, &c, &flags);
        if (nbytes==c0) {
            continue;
        }
        c0 = nbytes;


        MPI_Type_get_extent(type,&lb,&extent);
        MPI_Type_size(type,&typesize);

        nn = c * extent;
        //bcastbuf = malloc(nn);
        posix_memalign(&bcastbuf, CACHE_LINE_SIZE, nn);
        assert(bcastbuf!=NULL);

        /* this is needed to avoid mem leaks */
        typesize_str  = my_int_to_string(typesize);
        extent_str  = my_int_to_string(extent);
        real_size_str = my_int_to_string(c*typesize);

        //@ set nbytes_str=nbytes_list->elements[i]
        //@ set derivedtype_size=typesize_str
        //@ set real_size=real_size_str
        //@ set derivedtype_extent=extent_str

        if (conf.use_datatype) {
            bcast_datatype(rank, bcastbuf, c, type, conf.root_proc, conf.comm);
        }
        else {
            bcast_pack(rank, bcastbuf, c, type, conf.root_proc, conf.comm);
        }

        free(typesize_str);
        free(extent_str);
        free(real_size_str);

        free(bcastbuf);

        if ((flags & PREDEFINED_DT) == 0) { // free derived datatypes
            MPI_Type_free(&type);
        }
    }

    for (i=0; i<nbytes_list->n_elems; i++) {
        free(nbytes_list->elements[i]);
    }
    free(nbytes_list->elements);
    free(nbytes_list);

    return MPI_SUCCESS; // no...
}



int allgatherpattern_dynamictype(pattern_config_t conf, dictionary_t *dict)
{
    int size, rank;
    int i;
    size_t nn;
    void *sendbuf, *recvbuf;
    MPI_Datatype type;
    int c;
    size_t c0;
    MPI_Aint lb, extent;
    string_array_t* nbytes_list = NULL;
    int typesize;
    size_t nbytes;
    char *typesize_str, *real_size_str, *extent_str;
    int flags;

    MPI_Comm_size(conf.comm,&size);
    MPI_Comm_rank(conf.comm,&rank);

    //@ global pattern_type=PATTERN_DYNAMIC

    nbytes_list = get_string_array_from_dict("nbytes_list", dict);

    // time this
    c0 = -1;
    for (i=0; i<nbytes_list->n_elems; i++) {
        nbytes = atol(nbytes_list->elements[i]);

        // create datatype
        instantiate_dynamic_datatype(conf, dict, nbytes, &type, &c, &flags);
        if (nbytes==c0) {
            continue;
        }
        c0 = nbytes;

        MPI_Type_get_extent(type,&lb,&extent);
        MPI_Type_size(type,&typesize);

        nn = c * extent;
        posix_memalign(&sendbuf, CACHE_LINE_SIZE, nn);
        assert(sendbuf!=NULL);
        //recvbuf = malloc(nn * size);
        posix_memalign(&recvbuf, CACHE_LINE_SIZE, nn * size);
        assert(recvbuf!=NULL);

        /* this is needed to avoid mem leaks */
        typesize_str  = my_int_to_string(typesize);
        extent_str  = my_int_to_string(extent);
        real_size_str = my_int_to_string(c*typesize);

        //@ set nbytes_str=nbytes_list->elements[i]
        //@ set derivedtype_size=typesize_str
        //@ set real_size=real_size_str
        //@ set derivedtype_extent=extent_str

        if (conf.use_datatype) {
            allgather_datatype(rank, sendbuf, recvbuf, c, type, conf.root_proc, conf.comm);
        }
        else {
            allgather_pack(rank, sendbuf, recvbuf, c, type, conf.root_proc, conf.comm);
        }

        if ((flags & PREDEFINED_DT) == 0) { // free derived datatypes
            MPI_Type_free(&type);
        }

        free(typesize_str);
        free(extent_str);
        free(real_size_str);

        free(sendbuf);
        free(recvbuf);
    }

    for (i=0; i<nbytes_list->n_elems; i++) {
        free(nbytes_list->elements[i]);
    }
    free(nbytes_list->elements);
    free(nbytes_list);

    return MPI_SUCCESS; // no...
}

