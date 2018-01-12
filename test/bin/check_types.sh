#!/bin/bash


if [ $# != 1 ];
then
echo "testall.sh <path to bin dir>"
exit 1;
fi

TYPEPERF_BIN_DIR=$1

A_values=( 2 8 16 )
B_values=( 48 )
S_values=( 1 8 10 )
nbytes_values=( 10240 2621440 )

basetype="MPI_INT"

echo "##########################################################################"
echo " Comparing Tiled types against the reference TiledIndexed"
echo "##########################################################################"


for nbytes in ${nbytes_values[@]};
do
    for A in ${A_values[@]};
    do
        for B in ${B_values[@]};
        do
            for S in ${S_values[@]};
            do
            ${TYPEPERF_BIN_DIR}/datatype_check --params=layout1:tiled_struct_indexed --params=layout2:ref_TiledStructIndex --params=nbytes_list:${nbytes} --params=b:${basetype} --params=B:${B} --params=S:${S} --params=A:${A}
            done
        done
    done
done


echo
echo
echo "##########################################################################"
echo " Comparing Alternating types against the reference AlternatingIndexed"
echo "##########################################################################"


for nbytes in ${nbytes_values[@]};
do
    for A in ${A_values[@]};
    do
        for B in ${B_values[@]};
        do
            for S in ${S_values[@]};
            do
            A1=$A
            A2=$(( $A * 3 ))
            ${TYPEPERF_BIN_DIR}/datatype_check --params=layout1:alternating_indexed --params=layout2:ref_AlternatingIndexed --params=nbytes_list:${nbytes} --params=b:${basetype} --params=A1:${A1}  --params=A2:${A2} --params=B1:${B} --params=B2:${A2}
            done
        done
    done
done
