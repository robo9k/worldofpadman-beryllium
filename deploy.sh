#!/bin/sh

BUILD_DIR='build'
VERSION=`./version.sh`

BERYLLIUM='beryllium-v'$VERSION
SRC='release'
DST=$BUILD_DIR'/'$BERYLLIUM
ZIP=$BERYLLIUM'.zip'
SYMLINK='beryllium-current.zip'

## TODO: Fix Makefile git dependencies so we can remove make clean
## ^ Njet, we also need to remove old beryllium versions
rm -rf $BUILD_DIR
make clean
make -j3

## export versioned content
rm -rf $DST
#svn export $SRC $DST
cp -R $SRC $DST

## now copy unversioned content
cp build/release-linux-x86_64/baseq3/vm/qagame.qvm $DST/vm/

## next prepare and copy template files
./replace.sh "__VERSION__" $VERSION $SRC/README > $DST/README

## zip all together
rm $ZIP
cd $BUILD_DIR
zip -r -9 $ZIP $BERYLLIUM/*

rm $SYMLINK
ln -s $ZIP $SYMLINK

cd ..

rm -rf $DST


BERYLLIUM_B3='beryllium-b3-v'$VERSION
SRC='b3'
DST=$BUILD_DIR'/'$BERYLLIUM_B3
ZIP=$BERYLLIUM_B3'.zip'
SYMLINK='beryllium-b3-current.zip'

#svn export $PLUGINSRC $PLUGINDST
cp -R $SRC $DST

rm $ZIP
cd $BUILD_DIR
zip -r -9 $ZIP $BERYLLIUM_B3/*

rm $SYMLINK
ln -s $ZIP $SYMLINK

cd ..

rm -rf $DST

