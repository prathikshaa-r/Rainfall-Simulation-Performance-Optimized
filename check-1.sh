#!/bin/bash
bash -c "cd rainfall; ./seq_1.sh 2>out-1"
./check.py 4 sample_4x4.out rainfall/out-1
