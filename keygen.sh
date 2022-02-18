#!/bin/bash

ssh-keygen -t rsa   -b 4096 -f ssh/keys/rsa-key   -N "" -q <<< "y" > /dev/null 2>&1;
ssh-keygen -t ecdsa -b 521  -f ssh/keys/ecdsa-key -N "" -q <<< "y" > /dev/null 2>&1;
