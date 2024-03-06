# !/bin/bash

echo "Enter '.c' file to build: "

fileName=$@

gcc -g -std=c11 $fileName -o out -lpthread

echo "done" 
