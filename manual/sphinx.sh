#!/bin/bash

set -ex

for file in $(ls *.tex)
do
    if [ $file != "manual.tex" ]
    then
        pandoc -s $file -o ../docs/$(echo $file | sed "s/\.tex/\.rst/g")
    fi
done

cp -r $(ls -d -- */) ../docs/
