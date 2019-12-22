#     Project: Remmina Plugin WEBKIT
# Description: Remmina protocol plugin to launch a GTK+ Webkit browser.
#      Author: Fabio Castelli (Muflone) <muflone@muflone.com>
#   Copyright: 2012-2019 Fabio Castelli (Muflone)
#     License: GPL-2+
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

set(REMMINA_PLUGIN_SRCS
  src/remmina-gtkwebkit-browser.c
)
pkg_check_modules(WEBKIT REQUIRED gtk+-3.0 webkitgtk-3.0)
include_directories(${WEBKIT_INCLUDE_DIRS})

add_executable(remmina-gtkwebkit-browser ${REMMINA_PLUGIN_SRCS})
target_link_libraries(remmina-gtkwebkit-browser ${WEBKIT_LIBRARIES})
install(TARGETS remmina-gtkwebkit-browser DESTINATION ${CMAKE_INSTALL_BINDIR})
