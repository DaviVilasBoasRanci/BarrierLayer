// barrierlayer_gui.c
// Interface gráfica para executar arquivos .exe com BarrierLayer
// Requer GTK 3: sudo apt install libgtk-3-dev

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char exe_path[1024] = "";

static void on_file_set(GtkFileChooserButton *button, gpointer user_data) {
    const char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(button));
    strncpy(exe_path, filename, sizeof(exe_path)-1);
}

static void on_run_clicked(GtkButton *button, gpointer user_data) {
    if (strlen(exe_path) == 0) {
        GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Selecione um arquivo .exe primeiro!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    // Monta comando para rodar com BarrierLayer
    char command[2048];
    snprintf(command, sizeof(command), "ENABLE_BARRIERLAYER=1 ./scripts/run_with_barrierlayer.sh '%s'", exe_path);
    system(command);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "BarrierLayer GUI");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 150);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkWidget *file_chooser = gtk_file_chooser_button_new("Escolha um arquivo .exe", GTK_FILE_CHOOSER_ACTION_OPEN);
    gtk_box_pack_start(GTK_BOX(vbox), file_chooser, FALSE, FALSE, 0);
    g_signal_connect(file_chooser, "file-set", G_CALLBACK(on_file_set), NULL);

    GtkWidget *run_button = gtk_button_new_with_label("Executar com BarrierLayer");
    gtk_box_pack_start(GTK_BOX(vbox), run_button, FALSE, FALSE, 0);
    g_signal_connect(run_button, "clicked", G_CALLBACK(on_run_clicked), NULL);

    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}
