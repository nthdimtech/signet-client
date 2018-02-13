## Building

To build the Signet client from source you need a Qt5 development environment/packages and libgcrypt and one or more platform specific packages. If you have downloaded the source from Git you must download the submodules with:

	$ git submodule update --init

### GNU/Linux systems

On Debian based systems (Ubuntu, Linux Mint, etc) you can install Signet's build dependencies with the command:

	$ sudo apt-get install qt5-default libqt5websockets5-dev libqt5x11extras5-dev libgcrypt20-dev zlib1g-dev libx11-dev

On other GNU/Linux systems build or install the equivalent libraries for Qt5 (With Websockets and x11extras), zlib, and libgcrypt. Now run qmake and make:

	$ qmake client/client.pro
	$ make

After building is finished a `signet` binary will be created which can be moved anywhere else. You will need to install a udev rule before the client will work. It is contained in 'signet-base/signetdev/host/linux/50-signet.rules'. You must install it with sudo to `/etc/udev/rules.d` with:

	$ sudo cp signet-base/signetdev/host/linux/50-signet.rules /etc/udev/rules.d

### MacOS

To install Signet's build dependencies first install [Brew](https://brew.sh)

Once Brew is installed run:

	$ brew install qt5 libgcrypt libgpg-error zlib

To build Signet run:

	$ /usr/local/opt/qt5/bin/qmake client/client.pro
	$ make

After building is finished a `signet.app` folder will be created which you can move or copy anywhere. For example:

	$ cp -r signet.app ~/Desktop

### Windows

First install [MSYS2](www.msys2.com). MSYS2 will provide a build environment and a package manager to download Signet's dependencies. Once MSYS2 is installed launch the MSYS2/MinGW-64 shell and run:

	$ pacman -S mingw-w64-x86_64-gcc mingw64-w64-x86_64-make
	$ pacman -S mingw-w64-x86_64-qt5-static mingw-w64-x86_64-zlib mingw-w64-x86_64-libgcrypt
	$ pacman -S mingw-w64-x86_64-jasper mingw-w64-x86_64-openssl

Now you can build:

	$ export PATH=/mingw64/qt5-static/bin:$PATH
	$ qmake client/client.pro CONFIG+=release
	$ mingw32-make

This will build `Signet.exe` in the current directory. This executable should be self contained and can be copied anywhere by itself.
