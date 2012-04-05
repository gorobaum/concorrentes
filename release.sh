#!/bin/bash

outdir=ep1-thiago-wilson

if [ ! -d $outdir ]; then
  mkdir $outdir
fi

contents="src/ Makefile objs.makefile README relatorio.pdf relatorio1.txt relatorio2.txt arquivo1.txt arquivo2.txt"
cp -r $contents $outdir

tar -caf $outdir.tar.gz $outdir

rm -rf $outdir

