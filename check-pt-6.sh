#!/bin/bash
bash -c "cd rainfall; ./pt_6.sh 2>pt-out-6"
./check.py 2048 sample_2048x2048.out rainfall/pt-out-6
