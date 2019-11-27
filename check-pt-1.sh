#!/bin/bash
bash -c "cd rainfall; ./pt_1.sh 2>pt_out-1"
./check.py 4 sample_4x4.out rainfall/pt_out-1
