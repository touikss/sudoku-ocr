      #include <gtk/gtk.h>

      typedef struct
      {
        GtkBuilder *builder;
        gpointer user_data;
      } SGlobalData;

      void callback_about (GtkMenuItem *menuitem, gpointer user_data);

      int
      main(int argc, char *argv [])
      {
        GtkWidget *fenetre_principale = NULL;
        SGlobalData data;
        GError *error = NULL;
        gchar *filename = NULL;
        /* Initialisation de la bibliothèque Gtk. */
        gtk_init(&argc, &argv);

        /* Ouverture du fichier Glade de la fenêtre principale */
        data.builder = gtk_builder_new();

        /* Création du chemin complet pour accéder au fichier test.glade. */
        /* g_build_filename(); construit le chemin complet en fonction du système */
        /* d'exploitation. ( / pour Linux et \ pour Windows) */
        filename =  g_build_filename ("test.glade", NULL);

        /* Chargement du fichier test.glade. */
        gtk_builder_add_from_file (data.builder, filename, &error);
        g_free (filename);
        if (error)
        {
          gint code = error->code;
          g_printerr("%s\n", error->message);
          g_error_free (error);
          return code;
        }

        /* Affectation des signaux de l'interface aux différents CallBacks. */
        gtk_builder_connect_signals (data.builder, &data);

        /* Récupération du pointeur de la fenêtre principale */
        fenetre_principale = GTK_WIDGET(gtk_builder_get_object (data.builder, "MainWindow"));

        /* Affichage de la fenêtre principale. */
        gtk_widget_show_all (fenetre_principale);

        gtk_main();

        return 0;
      }

      void
      callback_about (GtkMenuItem *menuitem, gpointer user_data)
      {
        /* Transtypage du pointeur user_data pour récupérer nos données. */
        SGlobalData *data = (SGlobalData*) user_data;
        GtkWidget *dialog = NULL;

        /* Récupération de la fenêtre "AboutWindow". */
        dialog =  GTK_WIDGET (gtk_builder_get_object (data->builder, "AboutWindow"));
        gtk_dialog_run (GTK_DIALOG (dialog));

        /* On cache la fenêtre de dialogue. Si on la détruisait le prochain appel */
        /* à ce callback provoquerait un segdefault! */
        gtk_widget_hide (dialog);
      }
