# SpaceBrowser
Alternative to mainstream web browsers.

Currently this software is in development. Basic functionality works, but expect issues.

# Features
* almost all configuration and other data stored in SQL
* can use multiple SQL databases: separate your work from leisure
* vertical, tree-style tab browsing
* built-in script blocking: unblock scripts per site or globally
* password manager with GPG encryption

# Installation
Currently the only option is to compile this software by yourself. The procedure is quite easy. Steps below are for a clean Ubuntu 18 installation:

1. sudo apt install git cmake g++ libgpgme-dev
2. Download and install Qt from: https://www.qt.io/download-qt-installer
   There is no need to install all the packages. What you need are "Desktop gcc" and "Qt WebEngine" modules.
3. git clone git@github.com:tuvok/spacebrowser.git
4. git submodule init
5. git submodule update
6. mkdir spacebrowser/build
7. cd spacebrowser/build
8. cmake -DCMAKE_PREFIX_PATH="PATH TO QT" ../
9. make browser -jX # substitute number of cores on your system for X
