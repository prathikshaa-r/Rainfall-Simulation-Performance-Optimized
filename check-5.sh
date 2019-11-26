#!/bin/bash
bash -c "cd rainfall; ./seq_5.sh 2>out-5"
./check.py 512 sample_512x512.out rainfall/out-5
