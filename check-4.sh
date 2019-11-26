#!/bin/bash
bash -c "cd rainfall; ./seq_4.sh 2>out-4"
./check.py 128 sample_128x128.out rainfall/out-4
