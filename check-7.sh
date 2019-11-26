#!/bin/bash
bash -c "cd rainfall; ./seq_7.sh 2>out-7"
./check.py 4096 measurement_4096x4096.out rainfall/out-7
