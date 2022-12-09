gcc main.c predictor.c -o main

n=13
n2=9
pc=10
l=10

echo 'gshare:'
echo ''

bunzip2 -kc ../traces/fp_1.bz2 | ./main --gshare:$n

echo ''

bunzip2 -kc ../traces/fp_2.bz2 | ./main --gshare:$n

echo ''

bunzip2 -kc ../traces/int_1.bz2 | ./main --gshare:$n

echo ''

bunzip2 -kc ../traces/int_2.bz2 | ./main --gshare:$n

echo ''

bunzip2 -kc ../traces/mm_1.bz2 | ./main --gshare:$n

echo ''

bunzip2 -kc ../traces/mm_2.bz2 | ./main --gshare:$n

echo ''

echo 'tournament:'
echo ''

bunzip2 -kc ../traces/fp_1.bz2 | ./main --tournament:$n2:$pc:$l

echo ''

bunzip2 -kc ../traces/fp_2.bz2 | ./main --tournament:$n2:$pc:$l

echo ''

bunzip2 -kc ../traces/int_1.bz2 | ./main --tournament:$n2:$pc:$l

echo ''

bunzip2 -kc ../traces/int_2.bz2 | ./main --tournament:$n2:$pc:$l

echo ''

bunzip2 -kc ../traces/mm_1.bz2 | ./main --tournament:$n2:$pc:$l

echo ''

bunzip2 -kc ../traces/mm_2.bz2 | ./main --tournament:$n2:$pc:$l

echo ''

echo 'custom:'
echo ''

bunzip2 -kc ../traces/fp_1.bz2 | ./main --custom

echo ''

bunzip2 -kc ../traces/fp_2.bz2 | ./main --custom

echo ''

bunzip2 -kc ../traces/int_1.bz2 | ./main --custom

echo ''

bunzip2 -kc ../traces/int_2.bz2 | ./main --custom

echo ''

bunzip2 -kc ../traces/mm_1.bz2 | ./main --custom

echo ''

bunzip2 -kc ../traces/mm_2.bz2 | ./main --custom

echo ''