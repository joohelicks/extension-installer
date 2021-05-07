#include <gtk/gtk.h>
#include <libsoup/soup.h>
#include <json-glib/json-glib.h>
#include <glib.h>
#include <stdio.h>
#include <string.h>

typedef struct extension_t {
  char *name;
  char *description;
} extension;

typedef struct extensions_t {
  extension *extensions;
  int total;
  int numpages;
} extension_list;

GtkBuilder *builder;
GtkWidget *window;


SoupMessage* download_json(char *query, char *value)
{
  if (query == NULL || value == NULL) {
    return NULL;
  }

  GString *URL = g_string_new("https://extensions.gnome.org/extension-query/");
  g_string_append_printf(URL, "?%s=%s", query, value);

  printf("URL = %s\n", URL->str);
  SoupMessage *msg = soup_message_new ("GET", URL->str);

  if (msg == NULL)
    return NULL;

  SoupSession *session = soup_session_sync_new();
  guint status = soup_session_send_message (session, msg);
  return msg;
}

extension_list *parse_json(const char *data)
{
  printf("parse_json()\n");
  JsonParser *parser = json_parser_new();
  json_parser_load_from_data(parser, data, -1, NULL);

  JsonReader *reader = json_reader_new(json_parser_get_root(parser));

  extension_list *list = g_new(extension_list, 1);


  json_reader_read_member(reader, "total");
  list->total = json_reader_get_int_value(reader);
  json_reader_end_element(reader);

  json_reader_read_member(reader, "numpages");
  list->total = json_reader_get_int_value(reader);
  json_reader_end_element(reader);


  json_reader_read_member (reader, "extensions");
  list->extensions = g_new(extension, list->total);

  int i;
  for (i=0; i < list->total; i++) {

    JsonReader *reader = json_reader_new(json_parser_get_root(parser));
    json_reader_read_member (reader, "extensions");
    json_reader_read_element (reader, i);

    json_reader_read_member (reader, "name");
    list->extensions[i].name = json_reader_get_string_value(reader);

    json_reader_end_element (reader);

    json_reader_read_member (reader, "description");
    list->extensions[i].description = json_reader_get_string_value(reader);

  }

  return list;
}

int main(int argc, char **argv)
{

  SoupMessage *msg = download_json("page", "2");
  if (msg == NULL) {
    printf("unable to download extension data\n");
  }

  extension_list *extension_list = parse_json(msg->response_body->data);

  gtk_init(&argc, &argv);

  builder = gtk_builder_new();
  gtk_builder_add_from_file(builder, "interface.glade", NULL);

  window = GTK_WIDGET(gtk_builder_get_object(builder, "window1"));
  gtk_builder_connect_signals(builder, NULL);

  GtkListBox *listbox = GTK_WIDGET(gtk_builder_get_object(builder, "listbox"));

  int i;
  for (i=0; i < 10; i++) {
    GtkLabel *label = gtk_label_new(NULL);
    gtk_widget_set_size_request(label, 50, 50);
    gtk_label_set_text(GTK_LABEL (label), extension_list->extensions[i].name);
     gtk_label_set_max_width_chars (GTK_LABEL (label), 30);
    gtk_list_box_insert(listbox, label, -1);
    gtk_widget_show(label);
  }

  g_object_unref(builder);
  gtk_widget_show(window);
  gtk_main();

  return 0;
}

void testi(GtkSearchEntry *entry, gpointer user_data)
{
  char* search_str = gtk_entry_get_text(entry);

  SoupMessage *msg = download_json("search", search_str);
  if (msg == NULL) {
    printf("unable to search extensions\n");
    return;
  }

  extension_list *list = parse_json(msg->response_body->data);

  int i;
  for (i = 0; i < list->total; i++)
    printf("[%d] %s\n", i, list->extensions[i].name);

  GtkListBox *listbox = GTK_WIDGET(gtk_builder_get_object(builder, "listbox"));
  GList *children = gtk_container_get_children(GTK_LIST_BOX(listbox));

 while (children->next != NULL) {
    printf("asd\n");
    children = children->next;
  }

}

void on_window_main_destroy()
{
  gtk_main_quit();
}
