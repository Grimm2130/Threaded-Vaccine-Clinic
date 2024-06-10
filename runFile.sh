# !/bin/bash

echo "Enter '.c' file to build: "

fileName=VaccineClinic.c

gcc -g -D DEBUG=1 -D LOG_LEVEL_3=1 ds/lookup_buffer.c ds/queue.c VaccineClinic.c -o VaccineClinic -lpthread

echo "done" 
