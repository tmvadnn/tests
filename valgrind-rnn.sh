#!/bin/bash

valgrind --tool=memcheck -v --leak-check=full --show-leak-kinds=all --track-origins=yes  --suppressions=/home/ozapatam/Projects/root/etc/valgrind-root.supp  --log-file=report.valgrind  root.exe -l -b -q  RNN_test.C
