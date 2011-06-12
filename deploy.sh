#!/bin/sh

MODNAME="beryllium"
SRC="release"
## destination could include VERSION
DST=$MODNAME

make -j3

## export versioned content
rm -rf $DST
svn export $SRC $DST

## now copy unversioned content
cp build/release-linux-x86_64/baseq3/vm/qagame.qvm $DST/vm/

## zip all together
rm $MODNAME.zip
zip -r -9 $MODNAME $DST/*

rm -rf $DST


PLUGINNAME="b3-beryllium"
PLUGINSRC="b3"
PLUGINDST=$PLUGINNAME

svn export $PLUGINSRC $PLUGINDST

rm $PLUGINNAME.zip
zip -r -9 $PLUGINNAME $PLUGINDST/*

rm -rf $PLUGINDST

