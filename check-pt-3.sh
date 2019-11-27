#!/bin/bash
#!/bin/bash
bash -c "cd rainfall; ./pt_3.sh 2>pt-out-3"
./check.py 32 sample_32x32.out rainfall/pt-out-3
