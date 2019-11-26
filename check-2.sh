#!/bin/bash
bash -c "cd rainfall; ./seq_2.sh 2>out-2"
./check.py 16 sample_16x16.out rainfall/out-2
