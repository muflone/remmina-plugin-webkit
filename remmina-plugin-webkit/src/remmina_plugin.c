/*
 *     Project: Remmina Plugin WEBKIT
 * Description: Remmina protocol plugin to launch a GTK+ Webkit browser.
 *      Author: Fabio Castelli (Muflone) <muflone@muflone.com>
 *   Copyright: 2012-2019 Fabio Castelli (Muflone)
 *     License: GPL-2+
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of ERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "plugin_config.h"
#include <remmina/remmina_plugin.h>
#include <gtk/gtkx.h>

typedef struct {
  GtkWidget *socket;
  gint socket_id;
  GPid pid;
} RemminaPluginData;

static RemminaPluginService *remmina_plugin_service = NULL;

static void remmina_plugin_webkit_on_plug_added(GtkSocket *socket, RemminaProtocolWidget *gp) {
  TRACE_CALL(__func__);
  RemminaPluginData *gpdata;
  gpdata = (RemminaPluginData*) g_object_get_data(G_OBJECT(gp), "plugin-data");
  remmina_plugin_service->log_printf("[%s] Plugin plug added on socket %d\n", PLUGIN_NAME, gpdata->socket_id);
  remmina_plugin_service->protocol_plugin_signal_connection_opened(gp);
  return;
}

static void remmina_plugin_webkit_on_plug_removed(GtkSocket *socket, RemminaProtocolWidget *gp) {
  TRACE_CALL(__func__);
  remmina_plugin_service->log_printf("[%s] Plugin plug removed\n", PLUGIN_NAME);
  remmina_plugin_service->protocol_plugin_signal_connection_closed(gp);
}

/* Initialize plugin */
static void remmina_plugin_webkit_init(RemminaProtocolWidget *gp) {
  TRACE_CALL(__func__);
  RemminaPluginData *gpdata;
  remmina_plugin_service->log_printf("[%s] Plugin init\n", PLUGIN_NAME);
  /* Instance new GtkSocket */
  gpdata = g_new0(RemminaPluginData, 1);

  gpdata->socket = gtk_socket_new();
  remmina_plugin_service->protocol_plugin_register_hostkey(gp, gpdata->socket);
  gtk_widget_show(gpdata->socket);
  g_signal_connect(G_OBJECT(gpdata->socket), "plug-added", G_CALLBACK(remmina_plugin_webkit_on_plug_added), gp);
  g_signal_connect(G_OBJECT(gpdata->socket), "plug-removed", G_CALLBACK(remmina_plugin_webkit_on_plug_removed), gp);
  gtk_container_add(GTK_CONTAINER(gp), gpdata->socket);
  /* Save reference to plugin data */
  g_object_set_data_full(G_OBJECT(gp), "plugin-data", gpdata, g_free);
}

/* Open connection */
static gboolean remmina_plugin_webkit_open_connection(RemminaProtocolWidget *gp) {
  TRACE_CALL(__func__);
  RemminaFile *remminafile;
  gboolean ret;
  GError *error = NULL;
  gchar *argv[50];  // Contains all the arguments included the password
  gchar *argv_debug[50]; // Contains all the arguments, excluding the password
  gchar *command_line; // The whole command line obtained from argv_debug
  gint argc;
  gint i;
  gchar *option_str;
  gint width;
  gint height;
  RemminaPluginData *gpdata;

  #define GET_PLUGIN_STRING(value) \
    g_strdup(remmina_plugin_service->file_get_string(remminafile, value))
  #define GET_PLUGIN_BOOLEAN(value) \
    remmina_plugin_service->file_get_int(remminafile, value, FALSE)
  #define GET_PLUGIN_INT(value, default_value) \
    remmina_plugin_service->file_get_int(remminafile, value, default_value)
  #define ADD_ARGUMENT(name, value) { \
      argv[argc] = g_strdup(name); \
      argv_debug[argc] = g_strdup(name); \
      argc++; \
      if (value != NULL) { \
        argv[argc] = value; \
        argv_debug[argc++] = g_strdup(g_strcmp0(name, "-p") != 0 ? value : "XXXXX"); \
      } \
    }

  remmina_plugin_service->log_printf("[%s] Plugin open connection\n", PLUGIN_NAME);
  remminafile = remmina_plugin_service->protocol_plugin_get_file(gp);

  gpdata = (RemminaPluginData*) g_object_get_data(G_OBJECT(gp), "plugin-data");

  if (!GET_PLUGIN_BOOLEAN("detached")) {
    width = GET_PLUGIN_INT("resolution_width", 1024);
    height = GET_PLUGIN_INT("resolution_height", 768);
    remmina_plugin_service->protocol_plugin_set_width(gp, width);
    remmina_plugin_service->protocol_plugin_set_height(gp, height);
    gtk_widget_set_size_request(GTK_WIDGET(gp), width, height);
    gpdata->socket_id = gtk_socket_get_id(GTK_SOCKET(gpdata->socket));
  }

  argc = 0;
  // Main executable name
  ADD_ARGUMENT("remmina-gtkwebkit-browser", NULL);
  // Toolbar hidden
  if (GET_PLUGIN_BOOLEAN("no toolbar")) {
    ADD_ARGUMENT("-t", NULL);
  }
  // Back button hidden
  if (GET_PLUGIN_BOOLEAN("no back")) {
    ADD_ARGUMENT("-b", NULL);
  }
  // Forward button hidden
  if (GET_PLUGIN_BOOLEAN("no forward")) {
    ADD_ARGUMENT("-f", NULL);
  }
  // URL entry hidden
  if (GET_PLUGIN_BOOLEAN("no url entry")) {
    ADD_ARGUMENT("-u", NULL);
  }
  // Go button hidden
  if (GET_PLUGIN_BOOLEAN("no go")) {
    ADD_ARGUMENT("-g", NULL);
  }
  // Status bar hidden
  if (GET_PLUGIN_BOOLEAN("no status")) {
    ADD_ARGUMENT("-s", NULL);
  }
  // Socket to get the external window
  ADD_ARGUMENT("-X", g_strdup_printf("%d", gpdata->socket_id));
  // Server address
  option_str = GET_PLUGIN_STRING("server");
  ADD_ARGUMENT(option_str, option_str);
  // End of the arguments list
  ADD_ARGUMENT(NULL, NULL);
  // Retrieve the whole command line
  command_line = g_strjoinv(g_strdup(" "), (gchar **)&argv_debug[0]);
  remmina_plugin_service->log_printf("[WEBKIT] starting %s\n", command_line);
  g_free(command_line);
  // Execute the external process rdesktop
  ret = g_spawn_async(NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, &gpdata->pid, &error);
  remmina_plugin_service->log_printf("[WEBKIT] started remmina-gtkwebkit-browser with GPid %d\n", &gpdata->pid);
  // Free the arguments list
  for (i = 0; i < argc; i++) {
    g_free(argv_debug[i]);
    g_free(argv[i]);
  }
  // Show error message
  if (!ret)
    remmina_plugin_service->protocol_plugin_set_error(gp, "%s", error->message);
  // Show attached window socket ID
  if (!GET_PLUGIN_BOOLEAN("detached")) {
    remmina_plugin_service->log_printf("[%s] attached window to socket %d\n", PLUGIN_NAME, gpdata->socket_id);
    return TRUE;
  } else {
    return FALSE;
  }
}

static gboolean remmina_plugin_webkit_close_connection(RemminaProtocolWidget *gp) {
  TRACE_CALL(__func__);
  remmina_plugin_service->log_printf("[%s] Plugin close connection\n", PLUGIN_NAME);
  remmina_plugin_service->protocol_plugin_signal_connection_closed(gp);
  return FALSE;
}

/* Array of RemminaProtocolSetting for basic settings.
 * Each item is composed by:
 * a) RemminaProtocolSettingType for setting type
 * b) Setting name
 * c) Setting description
 * d) Compact disposition
 * e) Values for REMMINA_PROTOCOL_SETTING_TYPE_SELECT or REMMINA_PROTOCOL_SETTING_TYPE_COMBO
 * f) Unused pointer
 */
static const RemminaProtocolSetting remmina_plugin_webkit_basic_settings[] = {
  { REMMINA_PROTOCOL_SETTING_TYPE_SERVER, "server", NULL, FALSE, NULL, NULL },
  { REMMINA_PROTOCOL_SETTING_TYPE_RESOLUTION, "resolution", NULL, FALSE, NULL, NULL },
  { REMMINA_PROTOCOL_SETTING_TYPE_CHECK, "detached", N_("Detached window"), FALSE, NULL, NULL },
  { REMMINA_PROTOCOL_SETTING_TYPE_END, NULL, NULL, FALSE, NULL, NULL }
};

/* Array of RemminaProtocolSetting for basic settings.
 * Each item is composed by:
 * a) RemminaProtocolSettingType for setting type
 * b) Setting name
 * c) Setting description
 * d) Compact disposition
 * e) Values for REMMINA_PROTOCOL_SETTING_TYPE_SELECT or REMMINA_PROTOCOL_SETTING_TYPE_COMBO
 * f) Setting tooltip
 */
static const RemminaProtocolSetting remmina_plugin_webkit_advanced_settings[] = {
  { REMMINA_PROTOCOL_SETTING_TYPE_CHECK, "no status", N_("Disable status bar"), TRUE, NULL, NULL },
  { REMMINA_PROTOCOL_SETTING_TYPE_CHECK, "no toolbar", N_("Disable toolbar"), FALSE, NULL, NULL },
  { REMMINA_PROTOCOL_SETTING_TYPE_CHECK, "no back", N_("Disable button Back"), TRUE, NULL, NULL },
  { REMMINA_PROTOCOL_SETTING_TYPE_CHECK, "no forward", N_("Disable button Forward"), FALSE, NULL, NULL },
  { REMMINA_PROTOCOL_SETTING_TYPE_CHECK, "no url entry", N_("Disable URL location"), TRUE, NULL, NULL },
  { REMMINA_PROTOCOL_SETTING_TYPE_CHECK, "no go", N_("Disable button Go/Refresh"), FALSE, NULL, NULL },
  { REMMINA_PROTOCOL_SETTING_TYPE_END, NULL, NULL, FALSE, NULL, NULL }
};

/* Protocol plugin definition and features */
static RemminaProtocolPlugin remmina_plugin = {
  REMMINA_PLUGIN_TYPE_PROTOCOL,                 // Type
  PLUGIN_NAME,                                  // Name
  PLUGIN_DESCRIPTION,                           // Description
  GETTEXT_PACKAGE,                              // Translation domain
  PLUGIN_VERSION,                               // Version number
  PLUGIN_APPICON,                               // Icon for normal connection
  PLUGIN_APPICON,                               // Icon for SSH connection
  remmina_plugin_webkit_basic_settings,         // Array for basic settings
  remmina_plugin_webkit_advanced_settings,      // Array for advanced settings
  REMMINA_PROTOCOL_SSH_SETTING_NONE,            // SSH settings type
  NULL,                                         // Array for available features
  remmina_plugin_webkit_init,                   // Plugin initialization
  remmina_plugin_webkit_open_connection,        // Plugin open connection
  remmina_plugin_webkit_close_connection,       // Plugin close connection
  NULL,                                         // Query for available features
  NULL,                                         // Call a feature
  NULL,                                         // Send a keystroke
  NULL                                          // Screenshot support
};

G_MODULE_EXPORT gboolean remmina_plugin_entry(RemminaPluginService *service) {
  TRACE_CALL(__func__);
  remmina_plugin_service = service;

  if (!service->register_plugin((RemminaPlugin *) &remmina_plugin)) {
    return FALSE;
  }
  return TRUE;
}
