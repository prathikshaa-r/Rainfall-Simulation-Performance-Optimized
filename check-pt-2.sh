#!/bin/bash
bash -c "cd rainfall; ./pt_2.sh 2>pt-out-2"
./check.py 16 sample_16x16.out rainfall/pt-out-2
