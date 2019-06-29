#!/bin/bash

#
# Dependencies: build-essential libfreetype6-dev libharfbuzz-dev libgl1-mesa-dev libfontconfig-dev
#

$1/configure \
-platform linux-g++ -static -opengl desktop \
-opensource -confirm-license \
-prefix $2 \
-skip qt3d \
-skip qtcanvas3d \
-skip qtdatavis3d \
-skip qtgamepad \
-skip qtactiveqt \
-skip qtcharts \
-skip qtgraphicaleffects \
-skip qtimageformats \
-skip qtpurchasing \
-skip qttools \
-skip qtwayland \
-skip qtwebchannel \
-skip qtwebengine \
-skip qtwebview \
-skip qtlocation \
-skip qtfeedback \
-skip qtdeclarative \
-nomake examples -nomake tests \
-no-icu -no-mtdev -no-eglfs -no-linuxfb -no-libudev -no-egl -no-gstreamer -no-gbm -no-xcb-native-painting \
-qt-libjpeg -qt-libpng -qt-xcb -qt-pcre
