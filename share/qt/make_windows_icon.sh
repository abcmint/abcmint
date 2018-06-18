#!/bin/bash
# create multiresolution windows icon
ICON_SRC=../../src/qt/res/icons/abcmint.png
ICON_DST=../../src/qt/res/icons/abcmint.ico
convert ${ICON_SRC} -resize 16x16 abcmint-16.png
convert ${ICON_SRC} -resize 32x32 abcmint-32.png
convert ${ICON_SRC} -resize 48x48 abcmint-48.png
convert abcmint-16.png abcmint-32.png abcmint-48.png ${ICON_DST}

