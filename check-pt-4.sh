#!/bin/bash
bash -c "cd rainfall; ./pt_4.sh 2>pt-out-4"
./check.py 128 sample_128x128.out rainfall/pt-out-4
