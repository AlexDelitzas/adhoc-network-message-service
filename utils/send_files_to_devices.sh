#!/bin/bash

sshpass -p 'espx2019' scp ../bin/main_8450 root@10.0.84.50:/root/bin/main

sshpass -p 'espx2019' scp ../bin/main_8448 root@10.0.84.48:/root/bin/main
