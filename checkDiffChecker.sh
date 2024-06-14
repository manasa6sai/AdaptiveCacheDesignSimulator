make clean
make
echo "=================VALIDATION PASSED 0==================="

$1 16 1024 2 0 0 0 0 ./traces/gcc_trace.txt > ./validation_runs/V0.txt
 diff -iw ./validation_runs/V0.txt ./validation_runs/validation0.txt

echo "=================VALIDATION PASSED 1==================="
$1 16 1024 1 0 0 0 0 ./traces/perl_trace.txt > ./validation_runs/V1.txt
diff -iw ./validation_runs/V1.txt ./validation_runs/validation1.txt  

echo "=================VALIDATION PASSED 2==================="

$1 16 1024 2 0 0 1 0 ./traces/gcc_trace.txt > ./validation_runs/V2.txt
diff -iw ./validation_runs/V2.txt ./validation_runs/validation2.txt

echo "=================VALIDATION PASSED 3==================="


$1 16 1024 2 0 0 2 0 ./traces/vortex_trace.txt > ./validation_runs/V3.txt 
diff -iw ./validation_runs/V3.txt ./validation_runs/validation3.txt

echo "=================VALIDATION PASSED 4==================="


$1 16 1024 2 8192 4 0 0 ./traces/gcc_trace.txt > ./validation_runs/V4.txt 
diff -iw ./validation_runs/V4.txt ./validation_runs/validation4.txt

echo "=================VALIDATION PASSED 5==================="


$1 16 1024 1 8192 4 0 0 ./traces/go_trace.txt > ./validation_runs/V5.txt 
 diff -iw ./validation_runs/V5.txt ./validation_runs/validation5.txt

 echo "=================VALIDATION PASSED 6==================="


$1 16 1024 2 8192 4 0 1 ./traces/gcc_trace.txt > ./validation_runs/V6.txt
diff -iw ./validation_runs/V6.txt ./validation_runs/validation6.txt      

echo "=================VALIDATION PASSED 7==================="


$1 16 1024 1 8192 4 0 1 ./traces/compress_trace.txt > ./validation_runs/V7.txt  
diff -iw ./validation_runs/V7.txt ./validation_runs/validation7.txt   