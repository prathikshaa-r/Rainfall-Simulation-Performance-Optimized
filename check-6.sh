#!/bin/bash
bash -c "cd rainfall; ./seq_6.sh 2>out-6"
./check.py 2048 sample_2048x2048.out rainfall/out-6
