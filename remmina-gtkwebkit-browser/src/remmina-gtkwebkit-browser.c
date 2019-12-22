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

#include "config.h"
#include <gtk/gtk.h>
#include <gtk/gtkx.h>
#include <webkit/webkit.h>
#include <stdlib.h>
#include <ctype.h>

static int option_debug = 0;
static int option_has_toolbar = 1;
static int option_has_button_back = 1;
static int option_has_button_forward = 1;
static int option_has_button_go = 1;
static int option_has_uri_entry = 1;
static int option_has_status_bar = 1;
static long int option_socket_id = 0;
static gchar* option_initial_uri;

static GtkWidget* uri_entry;
static GtkStatusbar* statusbar;
static WebKitWebView* web_view;
static guint status_context_id;

static void activate_uri_entry_cb(GtkWidget* entry, gpointer data)
{
  const gchar* uri = gtk_entry_get_text(GTK_ENTRY(entry));
  g_assert(uri);
  webkit_web_view_open(web_view, uri);
}

static void link_hover_cb(WebKitWebView* page, const gchar* title, const gchar* link, gpointer data)
{
  gtk_statusbar_pop(statusbar, status_context_id);
  if (link)
    gtk_statusbar_push(statusbar, status_context_id, link);
}

static void progress_change_cb(WebKitWebView* page, gint progress, gpointer data)
{
  if (option_debug)
    printf("progress_change_cb %d%%\n", progress);
}

static void load_commit_cb(WebKitWebView* page, WebKitWebFrame* frame, gpointer data)
{
  if (option_debug)
    printf("commit_cb\n");
  const gchar* uri = webkit_web_frame_get_uri(frame);
  if (uri)
    gtk_entry_set_text(GTK_ENTRY(uri_entry), uri);
}

static void destroy_cb(GtkWidget* widget, gpointer data)
{
  if (option_debug)
    printf("destroy_cb\n");
  gtk_main_quit();
}

static void go_back_cb(GtkWidget* widget, gpointer data)
{
  if (option_debug)
    printf("go_back_cb\n");
  webkit_web_view_go_back(web_view);
}

static void go_forward_cb(GtkWidget* widget, gpointer data)
{
  if (option_debug)
    printf("go_forward_cb\n");
  webkit_web_view_go_forward(web_view);
}

static GtkWidget* create_browser()
{
  GtkWidget* scrolled_window = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  web_view = WEBKIT_WEB_VIEW(webkit_web_view_new());
  gtk_container_add(GTK_CONTAINER(scrolled_window), GTK_WIDGET(web_view));
  return scrolled_window;
}

static GtkWidget* create_statusbar()
{
  statusbar = GTK_STATUSBAR(gtk_statusbar_new());
  status_context_id = gtk_statusbar_get_context_id(statusbar, "Link Hover");

  return (GtkWidget*) statusbar;
}

static GtkWidget* create_toolbar()
{
  GtkToolItem* item;
  GtkWidget* toolbar;

  toolbar = gtk_toolbar_new();
  gtk_orientable_set_orientation(GTK_ORIENTABLE(toolbar), GTK_ORIENTATION_HORIZONTAL);
  gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_BOTH_HORIZ);

  /* Back button */
  if (option_has_button_back)
  {
    item = gtk_tool_button_new_from_stock(GTK_STOCK_GO_BACK);
    g_signal_connect(G_OBJECT(item), "clicked", G_CALLBACK(go_back_cb), NULL);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), item, -1);
  }

  /* Forward button */
  if (option_has_button_forward)
  {
    item = gtk_tool_button_new_from_stock(GTK_STOCK_GO_FORWARD);
    g_signal_connect(G_OBJECT(item), "clicked", G_CALLBACK(go_forward_cb), NULL);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), item, -1);
  }

  /* URL entry */
  if (option_has_uri_entry)
  {
    item = gtk_tool_item_new();
    gtk_tool_item_set_expand(item, TRUE);
    uri_entry = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(item), uri_entry);
    if (!option_has_button_go)
    {
      /* Disable activation when the go button has been disabled */
      g_signal_connect(G_OBJECT(uri_entry), "activate", G_CALLBACK(activate_uri_entry_cb), NULL);
      gtk_editable_set_editable(GTK_EDITABLE(uri_entry), FALSE);
    }
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), item, -1);
  }

  /* Go button */
  if (option_has_button_go)
  {
    item = gtk_tool_button_new_from_stock(GTK_STOCK_OK);
    g_signal_connect_swapped(G_OBJECT(item), "clicked", G_CALLBACK(activate_uri_entry_cb),(gpointer)uri_entry);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), item, -1);
  }

  return toolbar;
}

int parse_arguments(int argc, char* argv[])
{
  int c;
  opterr = 0;
  while ((c = getopt (argc, argv, "tbfugsdX:")) != -1)
  switch (c)
  {
    // Disable toolbar
    case 't':
      option_has_toolbar = 0;
      option_has_button_back = 0;
      option_has_button_forward = 0;
      option_has_uri_entry = 0;
      option_has_button_go = 0;
      break;
    // Disable back button
    case 'b':
      option_has_button_back = 0;
      break;
    // Disable forward button
    case 'f':
      option_has_button_forward = 0;
      break;
    // Disable uri entry
    case 'u':
      option_has_uri_entry = 0;
      option_has_button_go = 0;
      break;
    // Disable go button
    case 'g':
      option_has_button_go = 0;
      break;
    // Disable status bar
    case 's':
      option_has_status_bar = 0;
      break;
    // Debug messages
    case 'd':
      option_debug = 1;
      break;
    // Attach window to a GTK+ socket
    case 'X':
      option_socket_id = strtol(optarg, NULL, 0);
      break;
    // Unexpected option
    case '?':
      if (optopt == 'X')
        fprintf (stderr, "Option -%c requires an argument.\n", optopt);
      else if (isprint(optopt))
        fprintf (stderr, "Unknown option '-%c'.\n", optopt);
      else
        fprintf (stderr, "Unknown option character '\\x%x'.\n", optopt);
        return 1;
    default:
      return 1;
  }

  // Handle URI as last option
  if (optind < argc)
    option_initial_uri = (gchar *) argv[optind];

  return 0;
}

int main(int argc, char* argv[])
{
  GtkWidget *window;
  GtkWidget* vbox;

  gtk_init(&argc, &argv);
  if (parse_arguments(argc, argv))
    return 1;

  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  if (option_has_toolbar)
    gtk_box_pack_start(GTK_BOX(vbox), create_toolbar(), FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), create_browser(), TRUE, TRUE, 0);
  if (option_has_status_bar)
    gtk_box_pack_start(GTK_BOX(vbox), create_statusbar(), FALSE, FALSE, 0);
  g_signal_connect(G_OBJECT(web_view), "load-progress-changed", G_CALLBACK(progress_change_cb), web_view);
  if (uri_entry)
    g_signal_connect(G_OBJECT(web_view), "load-committed", G_CALLBACK(load_commit_cb), web_view);
  if (statusbar)
    g_signal_connect(G_OBJECT(web_view), "hovering-over-link", G_CALLBACK(link_hover_cb), web_view);

  webkit_web_view_open(web_view, option_initial_uri);
  gtk_widget_grab_focus(GTK_WIDGET(web_view));

  if (option_socket_id != 0)
  {
    printf("Attaching window to socket: %ld\n", option_socket_id);
    window = gtk_plug_new(option_socket_id);
  }
  else
  {
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW (window), 800, 600);
  }
  gtk_container_add(GTK_CONTAINER(window), vbox);
  gtk_widget_show_all(window);
  gtk_widget_realize(window);
  g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy_cb), NULL);

  gtk_main();
  return 0;
}
