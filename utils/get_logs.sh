#!/bin/bash

sshpass -p 'espx2019' scp root@10.0.84.50:/root/logs/* ../logs/

sshpass -p 'espx2019' scp root@10.0.84.48:/root/logs/* ../logs/
