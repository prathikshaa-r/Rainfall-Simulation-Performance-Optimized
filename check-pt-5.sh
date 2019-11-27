#!/bin/bash
bash -c "cd rainfall; ./pt_5.sh 2>pt-out-5"
./check.py 512 sample_512x512.out rainfall/pt-out-5
