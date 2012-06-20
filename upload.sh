#!/bin/sh

HOST='bfg.9k'
VERSION=$(./version.sh)
BERYLLIUM='beryllium-v'$VERSION'.zip'
BE3='beryllium-b3-v'$VERSION'.zip'
BE_CUR='beryllium-current.zip'
BE3_CUR='beryllium-b3-current.zip'
BUILD_DIR='build/up/'
DST='pub/beryllium/'

echo $VERSION > $BUILD_DIR/version.txt

rsync -va $BUILD_DIR/ $HOST':'$DST
