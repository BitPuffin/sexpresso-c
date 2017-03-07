#!/bin/sh

cc -I../include -lcmocka ../src/sexpresso.c test_sexpresso.c -otest-sexpresso
./test-sexpresso
rm test-sexpresso
