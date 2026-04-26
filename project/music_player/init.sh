#!/bin/bash

id=$(ipcs -m | grep 4d2 | awk '{print $2}')

if [ ! -z $id ]; then
    ipcrm -m $id
fi