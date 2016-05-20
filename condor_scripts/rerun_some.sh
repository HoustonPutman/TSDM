#!/bin/bash

sdm8=( 19 21 23 )
sdm16=( 7 16 19 20 21 23 )

for i in ${sdm8[@]}; do
    echo "condor_submit /projects/speedway/hpp/data/sniper/SDM_Regular/SDM-8/mcf/$i/run.condor"
    condor_submit /projects/speedway/hpp/data/sniper/SDM_Regular/SDM-8/mcf/$i/run.condor
done

for i in ${sdm16[@]}; do
    echo "condor_submit /projects/speedway/hpp/data/sniper/SDM_Regular/SDM-16/mcf/$i/run.condor"
    condor_submit /projects/speedway/hpp/data/sniper/SDM_Regular/SDM-16/mcf/$i/run.condor
done
