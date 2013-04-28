# Remmina Plugin WEBKIT - A Remmina plugin to open a GTK+ Webkit browser
#
# Copyright (C) 2013 Fabio Castelli <muflone@vbsimple.net>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, 
# Boston, MA 02111-1307, USA.

set(REMMINA_PLUGIN_WEBKIT_BROWSER_SRCS
  src/remmina-gtkwebkit-browser.c
)
#pkg_check_modules(WEBKIT REQUIRED gtk+-3.0 webkitgtk-3.0)
pkg_check_modules(WEBKIT REQUIRED gtk+-2.0 webkit-1.0)
MESSAGE( STATUS "WEBKIT_INCLUDE_DIRS: " ${WEBKIT_INCLUDE_DIRS} )
MESSAGE( STATUS "WEBKIT_LIBRARIES: " ${WEBKIT_LIBRARIES} )

include_directories(${WEBKIT_INCLUDE_DIRS})
add_executable(remmina-gtkwebkit-browser ${REMMINA_PLUGIN_WEBKIT_BROWSER_SRCS})
target_link_libraries(remmina-gtkwebkit-browser ${WEBKIT_LIBRARIES})
install(TARGETS remmina-gtkwebkit-browser DESTINATION ${CMAKE_INSTALL_BINDIR})
