#!/bin/bash


echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Profile Start ..."
#./pin -t ./source/tools/ManualExamples/obj-intel64/proccount.so -- ./TestHEAAN bsp 500
#date
#./pin -t ./source/tools/ManualExamples/obj-intel64/proccount.so -- ./TestHEAAN ins NORMAL_ADD 100

#date
#./pin -t ./source/tools/ManualExamples/obj-intel64/proccount.so -- ./TestHEAAN ins HE_ADD 200
#date

#date
#./pin -t ./source/tools/ManualExamples/obj-intel64/proccount.so -- ./TestHEAAN ins NORMAL_MULT 300

#date
#./pin -t ./source/tools/ManualExamples/obj-intel64/proccount.so -- ./TestHEAAN ins HE_MULT 400

#date

#date
#./pin -t ./source/tools/ManualExamples/obj-intel64/proccount.so -- ./TestHEAAN bsp 500
#date

date
./pin -t ./source/tools/ManualExamples/obj-intel64/proccount.so -- /home/ubisun/workspace/IDASH2017/IDASH2017/run/IDASH2017 /home/ubisun/workspace/IDASH2017/IDASH2017/data/1_training_data.csv 1 1 5 1 -1 1 0 1 /home/ubisun/workspace/IDASH2017/IDASH2017/data/1_testing_data.csv 10000
date



echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Profile Done."
