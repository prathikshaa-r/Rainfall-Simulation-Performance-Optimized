#!/bin/bash
#!/bin/bash
bash -c "cd rainfall; ./seq_3.sh 2>out-3"
./check.py 32 sample_32x32.out rainfall/out-3
