#!/bin/sh

VERSION=$(cat VERSION)
CVS=$(LANG=C git rev-parse --short HEAD)

echo $VERSION-$CVS

exit 0

