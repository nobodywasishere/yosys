#!/bin/bash

set -ex

for file in $(ls *.tex)
do
    pandoc -s $file -o ../docs/$(echo $file | sed "s/\.tex/\.rst/g")
done
