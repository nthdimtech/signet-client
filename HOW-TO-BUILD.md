### Get submodules

If you got this source from a Git repository then before you can build you must get the repositores submodules:

	$ git submodule update --init

If you downloaded the source from an archive then the submodules are already included.

### GNU/Linux build steps

If you are running a Debian-based system (Ubuntu, Linux Mint, etc.) then run the following command to install Signet's build dependencies

	$ sudo apt-get install qt5-default libqt5websockets5-dev libqt5x11extras5-dev libgcrypt20-dev zlib1g-dev libx11-dev

If you are not on a Debian based system, build or install the equivalent libraries for Qt5 (with Websockets and x11extras), zlib, libX11, and libgcrypt.

To build simply run qmake and then make:

	$ qmake client/client.pro CONFIG+=release CONFIG+=browser_plugins

	$ make

`make` will create a `signet` binary that you can move elsewhere.

You will need to install Signet's `udev` rule before the client will work. You can find the `udev` rule in `signet-base/signetdev/host/linux/50-signet.rules`. Use `sudo` to copy it to `/etc/udev/rules.d`:

	$ sudo cp signet-base/signetdev/host/linux/50-signet.rules /etc/udev/rules.d

### Mac OS build steps

To install Signet's build dependencies first install [Brew](https://brew.sh)

Once you have Brew, run:

	$ brew install qt5 libgcrypt libgpg-error zlib

To build Signet run:

	$ /usr/local/opt/qt5/bin/qmake client/client.pro CONFIG+=release CONFIG+=browser_plugins
	$ make

The build will create a `signet.app` folder that you can move or copy anywhere. For example:

	$ cp -r signet.app ~/Desktop

### Windows build steps

First install [MSYS2](http://www.msys2.org). MSYS2 will provide a build environment and a package manager to download Signet's dependencies. Once MSYS2 is installed, launch the MSYS2/MinGW-64 shell and run:

	$ pacman -S mingw-w64-x86_64-gcc mingw64-w64-x86_64-make
	$ pacman -S mingw-w64-x86_64-qt5-static mingw-w64-x86_64-zlib mingw-w64-x86_64-libgcrypt
	$ pacman -S mingw-w64-x86_64-jasper mingw-w64-x86_64-openssl
	$ pacman -S mingw-w64-x86_64-dbus mingw-w64-x86_64-libwebp

Now you can build:

	$ export PATH=/mingw64/qt5-static/bin:$PATH
	$ qmake client/client.pro CONFIG+=release CONFIG+=browser_plugins
	$ mingw32-make

This will build `Signet.exe` in the `release` subdirectory. This executable should be self contained and you can copy it anywhere and run it.
