#!/bin/bash

if [ $# != 2 ];
then
echo "testall.sh <gen install dir> <mpibench lib dir>"
exit 1;
fi

DTBENCH_GEN_DIR=$1
echo "generated code in ${DTBENCH_GEN_DIR}"

MPIBENCH_INST_DIR=$2
echo "installed mpi lib in ${MPIBENCH_INST_DIR}"

export LD_LIBRARY_PATH=${MPIBENCH_INST_DIR}/lib:$LD_LIBRARY_PATH
export DYLD_LIBRARY_PATH=${MPIBENCH_INST_DIR}/lib:$DYLD_LIBRARY_PATH


echo "################################################################"
echo "################################################################"
echo " basic types "

for pattern in bcast allgather pingpong;
do
  for ttype in datatype pack;
  do
  mpirun -np 2 ${DTBENCH_GEN_DIR}/reprompibench --params=b:MPI_INT --params=root:0 --params=nbytes_list:950 --params=test_type:${ttype} --params=pattern:${pattern} --params=A:100 --params=layout:tiled --params=B:103 --nrep=2

  mpirun -np 2 ${DTBENCH_GEN_DIR}/reprompibench --params=b:MPI_INT --params=root:0 --params=nbytes_list:950 --params=test_type:${ttype} --params=pattern:${pattern} --params=layout:bucket --params=A1:100 --params=A2:101 --params=B:103 --nrep=2

  mpirun -np 2 ${DTBENCH_GEN_DIR}/reprompibench --params=b:MPI_INT --params=root:0 --params=nbytes_list:950 --params=test_type:${ttype} --params=pattern:${pattern} --params=layout:block --params=A:100 --params=B1:101 --params=B2:102 --nrep=2

  mpirun -np 2 ${DTBENCH_GEN_DIR}/reprompibench --params=b:MPI_INT --params=root:0 --params=nbytes_list:950 --params=test_type:${ttype} --params=pattern:${pattern} --params=layout:alternating --params=A1:100 --params=A2:105 --params=B1:102 --params=B2:106 --nrep=2

  done
done


echo "################################################################"
echo "################################################################"
echo " dynamic types "

for pattern in bcast allgather pingpong;
do
  mpirun -np 2 ${DTBENCH_GEN_DIR}/reprompibench --params=b:MPI_INT --params=root:0 --params=nbytes_list:950 --params=test_type:datatype --params=pattern:${pattern} --params=layout:alternating_repeated --params=A1:100 --params=A2:101 --params=B:102 --nrep=2

  mpirun -np 2 ${DTBENCH_GEN_DIR}/reprompibench --params=b:MPI_INT --params=root:0 --params=nbytes_list:950 --params=test_type:datatype --params=pattern:${pattern} --params=layout:alternating_struct --params=A1:100 --params=A2:101 --params=B:102 --nrep=2

  mpirun -np 2 ${DTBENCH_GEN_DIR}/reprompibench --params=b:MPI_INT --params=root:0 --params=nbytes_list:950 --params=test_type:datatype --params=pattern:${pattern} --params=layout:tiled_vector --params=A:100 --params=B:102 --nrep=2

  mpirun -np 2 ${DTBENCH_GEN_DIR}/reprompibench --params=b:MPI_INT --params=root:0 --params=nbytes_list:950 --params=test_type:datatype --params=pattern:${pattern} --params=layout:vector_tiled --params=A:100 --params=B:102 --params=S:2 --nrep=2

  mpirun -np 2 ${DTBENCH_GEN_DIR}/reprompibench --params=b:MPI_INT --params=root:0 --params=nbytes_list:950 --params=test_type:datatype --params=pattern:${pattern} --params=layout:tiled_struct --params=A:10 --params=B:12 --params=S1:2 --params=S2:3 --nrep=2

  mpirun -np 2 ${DTBENCH_GEN_DIR}/reprompibench --params=b:MPI_INT --params=root:0 --params=nbytes_list:950 --params=test_type:datatype --params=pattern:${pattern} --params=layout:block_indexed --params=A:100 --params=B1:102 --params=B2:106 --nrep=2

  mpirun -np 2 ${DTBENCH_GEN_DIR}/reprompibench --params=b:MPI_INT --params=root:0 --params=nbytes_list:950 --params=test_type:datatype --params=pattern:${pattern} --params=layout:alternating_indexed --params=A1:100 --params=A2:101 --params=B1:102 --params=B2:106 --nrep=2

  mpirun -np 2 ${DTBENCH_GEN_DIR}/reprompibench --params=b:MPI_INT --params=root:0 --params=nbytes_list:950 --params=test_type:datatype --params=pattern:${pattern} --params=layout:blocks --params=A:100 --params=B:102 --params=l:200 --nrep=2

  mpirun -np 2 ${DTBENCH_GEN_DIR}/reprompibench --params=b:MPI_INT --params=root:0 --params=nbytes_list:950 --params=test_type:datatype --params=pattern:${pattern} --params=layout:tiled_struct_indexed_all --params=A:10 --params=B:12 --nrep=2

  mpirun -np 2 ${DTBENCH_GEN_DIR}/reprompibench --params=b:MPI_INT --params=root:0 --params=nbytes_list:950 --params=test_type:datatype --params=pattern:${pattern} --params=layout:alternating_indexed_fixed --params=A1:100 --params=A2:101 --params=B:206 --params=S:3 --nrep=2

  mpirun -np 2 ${DTBENCH_GEN_DIR}/reprompibench --params=b:MPI_INT --params=root:0 --params=nbytes_list:950 --params=test_type:datatype --params=pattern:${pattern} --params=layout:contig_alternating_indexed_fixed --params=A1:10 --params=A2:11 --params=B:26 --params=S:3 --nrep=2

  mpirun -np 2 ${DTBENCH_GEN_DIR}/reprompibench --params=b:MPI_INT --params=root:0 --params=nbytes_list:950 --params=test_type:datatype --params=pattern:${pattern} --params=layout:alternating_aligned --params=A1:100 --params=A2:101 --params=B1:102 --params=B2:106 --nrep=2

  mpirun -np 2 ${DTBENCH_GEN_DIR}/reprompibench --params=b:MPI_INT --params=root:0 --params=nbytes_list:950 --params=test_type:datatype --params=pattern:${pattern} --params=layout:tiled_struct_indexed_Sblocks --params=A:100 --params=B:102 --params=S:1 --nrep=2
done

echo "################################################################"
echo "################################################################"
echo " contiguous types "

for pattern in pingpong;
do
mpirun -np 2 ${DTBENCH_GEN_DIR}/reprompibench --params=b:MPI_INT --params=root:0 --params=nbytes_list:950 --params=test_type:datatype --params=pattern:${pattern} --params=layout:contig_type --params=subtype:tiled --params=A:100 --params=B:101 --nrep=2
mpirun -np 2 ${DTBENCH_GEN_DIR}/reprompibench --params=b:MPI_INT --params=root:0 --params=nbytes_list:950 --params=test_type:datatype --params=pattern:${pattern} --params=layout:contig_type --params=subtype:bucket  --params=A1:100 --params=A2:101 --params=B:103 --nrep=2
mpirun -np 2 ${DTBENCH_GEN_DIR}/reprompibench --params=b:MPI_INT --params=root:0 --params=nbytes_list:950 --params=test_type:datatype --params=pattern:${pattern} --params=layout:contig_type --params=subtype:block --params=A:100 --params=B1:101 --params=B2:102 --nrep=2
mpirun -np 2 ${DTBENCH_GEN_DIR}/reprompibench --params=b:MPI_INT --params=root:0 --params=nbytes_list:950 --params=test_type:datatype --params=pattern:${pattern} --params=layout:contig_type --params=subtype:alternating --params=A1:100 --params=A2:105 --params=B1:102 --params=B2:106 --nrep=2
done

echo "################################################################"
echo "################################################################"
echo " row col types "

for pattern in pingpong;
do
mpirun -np 2 ${DTBENCH_GEN_DIR}/reprompibench --params=b:MPI_INT --params=root:0 --params=nbytes_list:950 --params=test_type:datatype --params=pattern:${pattern} --params=layout:rowcol_full_indexed --params=A:100 --nrep=2
mpirun -np 2 ${DTBENCH_GEN_DIR}/reprompibench --params=b:MPI_INT --params=root:0 --params=nbytes_list:950 --params=test_type:datatype --params=pattern:${pattern} --params=layout:rowcol_contiguous_and_indexed --params=A:100 --nrep=2
mpirun -np 2 ${DTBENCH_GEN_DIR}/reprompibench --params=b:MPI_INT --params=root:0 --params=nbytes_list:950 --params=test_type:datatype --params=pattern:${pattern} --params=layout:rowcol_struct --params=A:100 --nrep=2
done




echo "################################################################"
echo "################################################################"
echo " MPI predifined datatypes "
for pattern in pingpong allgather bcast;
do
mpirun -np 2 ${DTBENCH_GEN_DIR}/reprompibench --params=b:MPI_INT --params=root:0 --params=nbytes_list:950 --params=test_type:datatype --params=pattern:${pattern} --params=layout:basetype --params=A:100 --nrep=2
mpirun -np 2 ${DTBENCH_GEN_DIR}/reprompibench --params=b:MPI_SHORT --params=root:0 --params=nbytes_list:950 --params=test_type:datatype --params=pattern:${pattern} --params=layout:basetype --params=A:100 --nrep=2
mpirun -np 2 ${DTBENCH_GEN_DIR}/reprompibench --params=b:MPI_DOUBLE --params=root:0 --params=nbytes_list:950 --params=test_type:datatype --params=pattern:${pattern} --params=layout:basetype --params=A:100 --nrep=2
done
