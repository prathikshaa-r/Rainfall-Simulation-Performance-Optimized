#!/bin/bash
bash -c "cd rainfall; ./pt_7.sh 2>pt-out-7"
./check.py 4096 measurement_4096x4096.out rainfall/pt-out-7
