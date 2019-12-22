# Remmina Plugin WEBKIT

**Author**: Fabio Castelli (Muflone) <muflone@muflone.com>

**Home page**: http://www.muflone.com/remmina-plugin-webkit/

**Copyright**: 2012-2019 Fabio Castelli (Muflone)

**License**: GPL-2+

**Status**: [![Build Status](https://travis-ci.org/muflone/remmina-plugin-webkit.svg?branch=master)](https://travis-ci.org/muflone/remmina-plugin-webkit)

## Description

A [**Remmina**](https://github.com/freerdp/remmina) protocol plugin to launch
a GTK+ Webkit browser.

![Main window](http://www.muflone.com/resources/remmina-plugin-webkit/archive/latest/english/general.png)

## Install instructions

Download and extract [**Remmina Plugin Builder**](https://github.com/muflone/remmina-plugin-builder/releases/):

    wget -O remmina-plugin-builder.tar.gz https://github.com/muflone/remmina-plugin-builder/archive/1.2.3.0.tar.gz
    tar --extract --verbose --gzip --file remmina-plugin-builder.tar.gz
  
Copy the plugin source files to the **remmina-plugin-to-build** directory:

    cp --recursive remmina-plugin-webkit CMakeLists.txt remmina-plugin-builder-1.2.3.0/remmina-plugin-to-build/

Build the plugin using Remmina Plugin Builder:

    cd remmina-plugin-builder-1.2.3.0
    cmake -DCMAKE_INSTALL_PREFIX=/usr .
    make
  
Install the plugin into the Remmina plugins directory (may need sudo or root
access):

    sudo make install

You'll find it in the remmina connection editor.

## License

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
