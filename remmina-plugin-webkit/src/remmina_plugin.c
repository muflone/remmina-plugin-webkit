/*
 *     Project: Remmina Plugin WEBKIT
 * Description: Remmina protocol plugin to launch a GTK+ Webkit browser.
 *      Author: Fabio Castelli (Muflone) <muflone@vbsimple.net>
 *   Copyright: 2012-2016 Fabio Castelli (Muflone)
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
#if GTK_VERSION == 3
  # include <gtk/gtkx.h>
#endif

typedef struct _RemminaPluginData
{
  GtkWidget *socket;
  gint socket_id;
  GPid pid;
} RemminaPluginData;

static RemminaPluginService *remmina_plugin_service = NULL;

static void remmina_plugin_on_plug_added(GtkSocket *socket, RemminaProtocolWidget *gp)
{
  RemminaPluginData *gpdata;
  gpdata = (RemminaPluginData*) g_object_get_data(G_OBJECT(gp), "plugin-data");
  remmina_plugin_service->log_printf("[%s] remmina_plugin_on_plug_added socket %d\n", PLUGIN_NAME, gpdata->socket_id);
  remmina_plugin_service->protocol_plugin_emit_signal(gp, "connect");
  return;
}

static void remmina_plugin_on_plug_removed(GtkSocket *socket, RemminaProtocolWidget *gp)
{
  remmina_plugin_service->log_printf("[%s] remmina_plugin_on_plug_removed\n", PLUGIN_NAME);
  remmina_plugin_service->protocol_plugin_close_connection(gp);
}

static void remmina_plugin_init(RemminaProtocolWidget *gp)
{
  remmina_plugin_service->log_printf("[%s] remmina_plugin_init\n", PLUGIN_NAME);
  RemminaPluginData *gpdata;

  gpdata = g_new0(RemminaPluginData, 1);
  g_object_set_data_full(G_OBJECT(gp), "plugin-data", gpdata, g_free);

  gpdata->socket = gtk_socket_new();
  remmina_plugin_service->protocol_plugin_register_hostkey(gp, gpdata->socket);
  gtk_widget_show(gpdata->socket);
  g_signal_connect(G_OBJECT(gpdata->socket), "plug-added", G_CALLBACK(remmina_plugin_on_plug_added), gp);
  g_signal_connect(G_OBJECT(gpdata->socket), "plug-removed", G_CALLBACK(remmina_plugin_on_plug_removed), gp);
  gtk_container_add(GTK_CONTAINER(gp), gpdata->socket);
}

static gboolean remmina_plugin_open_connection(RemminaProtocolWidget *gp)
{
  remmina_plugin_service->log_printf("[%s] remmina_plugin_open_connection\n", PLUGIN_NAME);
  #define GET_PLUGIN_STRING(value) \
    g_strdup(remmina_plugin_service->file_get_string(remminafile, value))
  #define GET_PLUGIN_BOOLEAN(value) \
    remmina_plugin_service->file_get_int(remminafile, value, FALSE)
  #define GET_PLUGIN_INT(value, default_value) \
    remmina_plugin_service->file_get_int(remminafile, value, default_value)

  RemminaPluginData *gpdata;
  RemminaFile *remminafile;
  gboolean ret;
  GError *error = NULL;
  gchar *argv[50];
  gint argc;
  gint i;
  gint width, height;

  gpdata = (RemminaPluginData*) g_object_get_data(G_OBJECT(gp), "plugin-data");
  remminafile = remmina_plugin_service->protocol_plugin_get_file(gp);

  if (!GET_PLUGIN_BOOLEAN("detached"))
  {
	width = GET_PLUGIN_INT("resolution_width", 1024);
    height = GET_PLUGIN_INT("resolution_height", 768);
    remmina_plugin_service->protocol_plugin_set_width(gp, width);
    remmina_plugin_service->protocol_plugin_set_height(gp, height);
    gtk_widget_set_size_request(GTK_WIDGET(gp), width, height);
    gpdata->socket_id = gtk_socket_get_id(GTK_SOCKET(gpdata->socket));
  }

  argc = 0;
  argv[argc++] = g_strdup("remmina-gtkwebkit-browser");
  if (GET_PLUGIN_BOOLEAN("no toolbar"))
  {
    argv[argc++] = g_strdup("-t");
  }
  if (GET_PLUGIN_BOOLEAN("no back"))
  {
    argv[argc++] = g_strdup("-b");
  }
  if (GET_PLUGIN_BOOLEAN("no forward"))
  {
    argv[argc++] = g_strdup("-f");
  }
  if (GET_PLUGIN_BOOLEAN("no url entry"))
  {
    argv[argc++] = g_strdup("-u");
  }
  if (GET_PLUGIN_BOOLEAN("no go"))
  {
    argv[argc++] = g_strdup("-g");
  }
  if (GET_PLUGIN_BOOLEAN("no status"))
  {
    argv[argc++] = g_strdup("-s");
  }

  argv[argc++] = g_strdup("-X");
  argv[argc++] = g_strdup_printf("%d", gpdata->socket_id);
  argv[argc++] = GET_PLUGIN_STRING("server");
  argv[argc++] = NULL;

  ret = g_spawn_async(NULL, argv, NULL, G_SPAWN_SEARCH_PATH,
    NULL, NULL, &gpdata->pid, &error);

  for (i = 0; i < argc; i++)
    g_free(argv[i]);

  if (!ret)
    remmina_plugin_service->protocol_plugin_set_error(gp, "%s", error->message);

  if (!GET_PLUGIN_BOOLEAN("detached"))
  {
    remmina_plugin_service->log_printf("[%s] attached window to socket %d\n", PLUGIN_NAME, gpdata->socket_id);
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

static gboolean remmina_plugin_close_connection(RemminaProtocolWidget *gp)
{
  remmina_plugin_service->log_printf("[%s] remmina_plugin_close_connection\n", PLUGIN_NAME);
  remmina_plugin_service->protocol_plugin_emit_signal(gp, "disconnect");
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
static const RemminaProtocolSetting remmina_plugin_basic_settings[] =
{
  { REMMINA_PROTOCOL_SETTING_TYPE_SERVER, NULL, NULL, FALSE, NULL, NULL },
  { REMMINA_PROTOCOL_SETTING_TYPE_RESOLUTION, NULL, NULL, FALSE, NULL, NULL },
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
 * f) Unused pointer
 */
static const RemminaProtocolSetting remmina_plugin_advanced_settings[] =
{
  { REMMINA_PROTOCOL_SETTING_TYPE_CHECK, "no status", N_("Disable status bar"), TRUE, NULL, NULL },
  { REMMINA_PROTOCOL_SETTING_TYPE_CHECK, "no toolbar", N_("Disable toolbar"), FALSE, NULL, NULL },
  { REMMINA_PROTOCOL_SETTING_TYPE_CHECK, "no back", N_("Disable button Back"), TRUE, NULL, NULL },
  { REMMINA_PROTOCOL_SETTING_TYPE_CHECK, "no forward", N_("Disable button Forward"), FALSE, NULL, NULL },
  { REMMINA_PROTOCOL_SETTING_TYPE_CHECK, "no url entry", N_("Disable URL location"), TRUE, NULL, NULL },
  { REMMINA_PROTOCOL_SETTING_TYPE_CHECK, "no go", N_("Disable button Go/Refresh"), FALSE, NULL, NULL },
  { REMMINA_PROTOCOL_SETTING_TYPE_END, NULL, NULL, FALSE, NULL, NULL }
};

/* Protocol plugin definition and features */
static RemminaProtocolPlugin remmina_plugin =
{
  REMMINA_PLUGIN_TYPE_PROTOCOL,                 // Type
  PLUGIN_NAME,                                  // Name
  PLUGIN_DESCRIPTION,                           // Description
  GETTEXT_PACKAGE,                              // Translation domain
  PLUGIN_VERSION,                               // Version number
  PLUGIN_APPICON,                               // Icon for normal connection
  PLUGIN_APPICON,                               // Icon for SSH connection
  remmina_plugin_basic_settings,                // Array for basic settings
  remmina_plugin_advanced_settings,             // Array for advanced settings
  REMMINA_PROTOCOL_SSH_SETTING_NONE,            // SSH settings type
  NULL,                                         // Array for available features
  remmina_plugin_init,                          // Plugin initialization
  remmina_plugin_open_connection,               // Plugin open connection
  remmina_plugin_close_connection,              // Plugin close connection
  NULL,                                         // Query for available features
  NULL,                                         // Call a feature
  NULL                                          // Send a keystroke
};

G_MODULE_EXPORT gboolean remmina_plugin_entry(RemminaPluginService *service)
{
  remmina_plugin_service = service;

  if (!service->register_plugin((RemminaPlugin *) &remmina_plugin))
  {
    return FALSE;
  }
  return TRUE;
}
