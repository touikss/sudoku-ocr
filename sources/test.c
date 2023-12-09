#include <gtk/gtk.h>
#include <pthread.h>
#include "../headers/image_2.h"
#include "../headers/canny.h"
#include "../headers/convert.h"
#include "../headers/image_manipulation.h"
#include "../headers/rotations.h"
#include "../headers/nn.h"
#include <string.h>
#include "../headers/solver_hexa.h"
#include "../headers/solver.h"
//#include <pango/pangofont.h>

void sudoku(GtkWidget *_, gpointer data);
void restart(GtkWidget *_, gpointer data);

#define IMAGE_SIZE 500
#define VERBOSE_MODE 0
#define VERBOSE_PATH NULL
#define ACCENT_COLOR    \
    {                   \
        13, 137, 224, 1 \
    }
#define TRANSPARENT \
    {               \
        0, 0, 0, 0  \
    }

SDL_Surface* globalImage;
SDL_Surface* globalRes;
GtkWidget *box;

int detected = 0;

double rotateScale = 0;
int hexa = 0;
char* path;
int mat[16][16];
int display[16][16];

GtkWidget *image;
GtkWidget *grid;
GtkWidget *cont_grid;

struct StepIndicators
{
    int current_step;
    GtkWidget *step_1_indicator;
    GtkWidget *step_2_indicator;
    GtkWidget *step_3_indicator;
    GtkWidget *step_4_indicator;
    GtkWidget *step_5_indicator;
    GtkWidget *step_6_indicator;
};
typedef struct StepIndicators StepIndicators;

struct Controls
{
    GtkButton *rotation_left;
    GtkButton *rotation_right;
    GtkButton *save_button;
    GtkButton *open_button;
    GtkButton *cancel_image_button;
    GtkButton *confirm_image_button;
    GtkButton *image_rotation_done_button;
    GtkButton *confirm_crop_button;
    GtkButton *correction_done_button;
    GtkButton *done_digit_detection;
    GtkButton *solve;
    GtkButton *restart_button;
    GtkFileChooserButton *file_chooser_button;
    GtkEventBox *crop_event_box;
};
typedef struct Controls Controls;

struct Page
{
    GtkViewport *container;
    GtkDrawingArea *image;
    GtkLabel *label;
};
typedef struct Page Page;

struct Pages
{
    char *current_page;
    Page *page1;
    Page *page2;
    Page *page3;
    Page *page4;
    Page *page5;
    Page *page6;
};
typedef struct Pages Pages;

struct Images
{
    Image2 *current_image;
    Image2 *clean;
    Image2 *mask;
    Image2 *image;
    Image2 *image_rotated;
    Image2 *image_rotated_clean;
    Image2 *image_rotated_cropped;
    Image2 *image_solved;
    Image2 *grid_square;
    gdouble current_rotation;
    gboolean is_rotated;
    gboolean is_loaded;
    gboolean is_displayed;
    gboolean is_grid_detected;
    gboolean is_perspective_corrected;
    gboolean is_solved;
    gint display_size;
    GtkDrawingArea *current_container;
    gboolean editing_digits;
    GtkLabel *current_label;
};
typedef struct Images Images;

struct MainWindow
{
    GtkWidget *window;
    GtkStack *stack;
    GtkGrid *sudoku_labels_grid;
    StepIndicators *step_indicators;
    Controls *controls;
    Pages *pages;
    Images *images;
    gboolean cropping;
    int **grid;
};
typedef struct MainWindow MainWindow;

void free_used_images(MainWindow *main_window)
{
    if (main_window->images->is_loaded)
    {
        free_Image2(main_window->images->mask);
        free_Image2(main_window->images->image);
        free_Image2(main_window->images->clean);
    }

    if (main_window->images->is_rotated && main_window->images->image_rotated)
    {
        free_Image2(main_window->images->image_rotated);
    }

    if (main_window->images->is_grid_detected)
    {
        free_Image2(main_window->images->image_rotated_clean);
    }

    if (main_window->images->is_perspective_corrected)
    {
        free_Image2(main_window->images->image_rotated_cropped);
    }

    if (main_window->images->is_solved)
    {
        free_Image2(main_window->images->image_solved);
    }
}

void draw_image(GtkDrawingArea *drawing_area, cairo_t *cr, gpointer data)
{
    MainWindow *main_window = (MainWindow *)data;

    GdkPixbuf *pixbuf = image_to_pixbuf(main_window->images->current_image);

    gint width = gtk_widget_get_allocated_width(GTK_WIDGET(drawing_area));
    gint height = gtk_widget_get_allocated_height(GTK_WIDGET(drawing_area));

    gint image_width = main_window->images->current_image->width;
    gint image_height = main_window->images->current_image->height;

    gdouble scale_x = (gdouble)width / image_width;
    gdouble scale_y = (gdouble)height / image_height;
    gdouble scale = MIN(scale_x, scale_y);

    gint scaled_width = image_width * scale;
    gint scaled_height = image_height * scale;
    gint scaled_x = (width - scaled_width) / 2;
    gint scaled_y = (height - scaled_height) / 2;

    pixbuf = gdk_pixbuf_scale_simple(
            pixbuf, scaled_width, scaled_height, GDK_INTERP_BILINEAR);

    cairo_surface_t *surface
            = gdk_cairo_surface_create_from_pixbuf(pixbuf, 1, NULL);

    cairo_set_source_surface(cr, surface, scaled_x, scaled_y);
    cairo_rectangle(cr, scaled_x, scaled_y, scaled_width, scaled_height);

    main_window->images->display_size
            = scaled_width > scaled_height ? scaled_height : scaled_width;

    cairo_fill(cr);

    cairo_surface_destroy(surface);
    g_object_unref(pixbuf);
}

void display_images(
        GtkDrawingArea *image_container, Image2 *image, MainWindow *main_window)
{
    *main_window->images->current_container = *image_container;

    if (main_window->images->is_displayed)
        free_Image2(main_window->images->current_image);
    main_window->images->display_size = IMAGE_SIZE;
    main_window->images->is_displayed = TRUE;

    *main_window->images->current_image = *image;

    gtk_widget_queue_draw(GTK_WIDGET(image_container));
}

void set_button_to_load(GtkButton *button)
{
    gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);

    GtkSpinner *spinner = GTK_SPINNER(gtk_spinner_new());
    GtkWidget *child = gtk_bin_get_child(GTK_BIN(button));

    if (child)
        gtk_container_remove(GTK_CONTAINER(button), child);

    gtk_container_add(GTK_CONTAINER(button), GTK_WIDGET(spinner));

    gtk_widget_show(GTK_WIDGET(spinner));

    gtk_spinner_start(spinner);
}

void reset_button(GtkButton *button)
{
    gtk_widget_set_sensitive(GTK_WIDGET(button), TRUE);

    GtkSpinner *spinner = GTK_SPINNER(gtk_bin_get_child(GTK_BIN(button)));

    if (spinner)
    {
        gtk_spinner_stop(spinner);
        gtk_container_remove(GTK_CONTAINER(button), GTK_WIDGET(spinner));
    }

    gtk_button_set_label(button, "Process");
}


void set_page(MainWindow *main_window, gchar *page)
{
    main_window->pages->current_page = page;

    gtk_stack_set_visible_child_name(main_window->stack, page);
}

void set_step(StepIndicators *step_indicators, int step_number)
{
    step_indicators->current_step = step_number;

    gtk_widget_set_opacity(step_indicators->step_1_indicator, 0.5);
    gtk_widget_set_opacity(step_indicators->step_2_indicator, 0.5);
    gtk_widget_set_opacity(step_indicators->step_3_indicator, 0.5);
    gtk_widget_set_opacity(step_indicators->step_4_indicator, 0.5);
    gtk_widget_set_opacity(step_indicators->step_5_indicator, 0.5);
    gtk_widget_set_opacity(step_indicators->step_6_indicator, 0.5);

    switch (step_number)
    {
        case 1:
            gtk_widget_set_opacity(step_indicators->step_1_indicator, 1.0);
            break;
        case 2:
            gtk_widget_set_opacity(step_indicators->step_2_indicator, 1.0);
            break;
        case 3:
            gtk_widget_set_opacity(step_indicators->step_3_indicator, 1.0);
            break;
        case 4:
            gtk_widget_set_opacity(step_indicators->step_4_indicator, 1.0);
            break;
        case 5:
            gtk_widget_set_opacity(step_indicators->step_5_indicator, 1.0);
            break;
        case 6:
            gtk_widget_set_opacity(step_indicators->step_6_indicator, 1.0);
            break;
    }
}

void file_selected(GtkWidget *widget, gpointer data)
{
    MainWindow *main_window = (MainWindow *)data;

    GtkFileChooser *file_chooser;

    if (GTK_IS_FILE_CHOOSER(widget))
        file_chooser = GTK_FILE_CHOOSER(widget);
    else
        file_chooser
                = GTK_FILE_CHOOSER(main_window->controls->file_chooser_button);

    GFile *file = gtk_file_chooser_get_file(file_chooser);
    path = g_file_get_path(file);


    // Loading image from path
    SDL_Surface *image = IMG_Load(path);
    if (image == NULL)
        errx(1, "Error while loading image in InitSdlAndLoadImage()");

    // 32 bits conversion
    SDL_Surface* image32 = SDL_ConvertSurfaceFormat(image, SDL_PIXELFORMAT_RGBA8888, 0);
    SDL_FreeSurface(image);
    if (image32 == NULL)
        errx(1, "Error while converting image to 32 bit format in InitSdlAndLoadImage()");

    // Resize
    image32 = ResizeSurface(image32, 1200, 880);
    if (image32 == NULL)
        errx(1, "Error while resizing image in InitSdlAndLoadImage()");

    globalImage = image32;

    *main_window->images->image = SDL_Surface_to_Image2(load_image(path));
    *main_window->images->mask = SDL_Surface_to_Image2(load_image(path));
    *main_window->images->clean = SDL_Surface_to_Image2(load_image(path));
    main_window->images->is_loaded = TRUE;

    // Reset values
    main_window->images->current_rotation = 0;
    main_window->images->is_rotated = false;

    display_images(main_window->pages->page2->image, main_window->images->clean,
                  main_window);

    gchar label[100];
    g_snprintf(label, 100, "Image: %s", path);
    gtk_label_set_text(main_window->pages->page2->label, label);

    if (widget != NULL)
        set_page(main_window, "page2");
}

void cancel_image_selection(GtkWidget *_, gpointer data)
{
    MainWindow *main_window = (MainWindow *)data;

    set_page(main_window, "page1");

    if (_ == NULL)
        printf(" ");
}

gboolean grid_detection_finished(gpointer data)
{
    //printf("grid_detection_finished\n");
    MainWindow *main_window = (MainWindow *)data;

    main_window->images->is_grid_detected = TRUE;

    //display_image(main_window->pages->page4->image,
      //            main_window->images->image_rotated_clean, main_window);  

    //display_image(main_window->pages->page4->image,
      //            SDL_Surface_to_Image2(load_image("test.png")), main_window);

    set_step(main_window->step_indicators, 4);

    set_page(main_window, "page4");

    return FALSE;
}

void *grid_detection_handler(gpointer data)
{
    MainWindow *main_window = (MainWindow *)data;

    Image temp = Image_CreateFromSurface(globalImage, globalImage->w, globalImage->h);
    SDL_FreeSurface(globalImage);
    Image_Rotate(&temp, rotateScale);

    globalImage = Image_CreateSurface(&temp);
    SDL_Surface* surface = ApplyCannyFilter(globalImage);
    Image_Free(&temp);

    if (surface == NULL){
        detected = 0;
        set_page(main_window, "page1");
        set_step(main_window->step_indicators, 1);
        restart(NULL, data);
    }
    else{
        detected = 1;
        *main_window->images->current_image = SDL_Surface_to_Image2(surface);

        g_idle_add(grid_detection_finished, main_window);
    }

    return NULL;
}


void process_image(GtkWidget *_, gpointer data)
{
    MainWindow *main_window = (MainWindow *)data;

    set_button_to_load(main_window->controls->image_rotation_done_button);
    gtk_widget_hide(GTK_WIDGET(main_window->controls->rotation_right));
    gtk_widget_hide(GTK_WIDGET(main_window->controls->rotation_left));

    // If the image is not rotated, populate the image_rotated pointer
    if (!main_window->images->is_rotated)
        *main_window->images->image_rotated
                = clone_image(main_window->images->image);
    // set this to true so that image_rotated is freed
    main_window->images->is_rotated = TRUE;

    // Display elements in page 4
    /*gchar label[100];
    g_snprintf(label, 100,
               "<span weight=\"bold\" size=\"large\">Processing Image</span>");
    gtk_label_set_markup(main_window->pages->page4->label, label);*/
    gtk_widget_hide(GTK_WIDGET(main_window->controls->rotation_left));
    gtk_widget_hide(GTK_WIDGET(main_window->controls->rotation_right));
    set_step(main_window->step_indicators, 3);
    //display_image(main_window->pages->page4->image,
          //        main_window->images->image_rotated, main_window);

    g_thread_new("grid_detection_handler", (GThreadFunc)grid_detection_handler,
                 main_window);

    if (_ == NULL)
        printf(" ");
}

void manual_rotate_image(GtkWidget *widget, gpointer data)
{
    MainWindow *main_window = (MainWindow *)data;

    //g_thread_new("grid_extraction_handler",
      //           (GThreadFunc)grid_extraction_handler, main_window);

    //gtk_widget_hide(GTK_WIDGET(main_window->controls->rotation_left));
    //gtk_widget_hide(GTK_WIDGET(main_window->controls->rotation_right));
    //set_button_to_load(main_window->controls->confirm_image_button);
    //set_button_to_load(main_window->controls->image_rotation_done_button);

    display_images(main_window->pages->page3->image, main_window->images->image,
                  main_window);

    /*gchar label[100];
    g_snprintf(label, 100,
               "<span weight=\"bold\" size=\"large\">Rotating Image</span>");
    gtk_label_set_markup(main_window->pages->page3->label, label);*/

    set_step(main_window->step_indicators, 2);

    if (widget != NULL)
        set_page(main_window, "page3");
}

void rotation_gauche(GtkWidget *_, gpointer data)
{
    MainWindow *main_window = (MainWindow *)data;

    if (main_window == NULL)
        errx(EXIT_FAILURE, "NULL pointer in rotation_left()\n");

    Image temp = convert_im2_to_im(*main_window->images->image);
    Image_Rotate(&temp, M_PI_2);

    main_window->images->image_rotated = convert_im_to_im2(temp);

    main_window->images->image = main_window->images->image_rotated;

    rotateScale += M_PI_2;
    display_images(GTK_DRAWING_AREA(main_window->pages->page3->image),
                  main_window->images->image_rotated, main_window);

    if (_ == NULL)
        printf(" ");

}

void rotation_droite(GtkWidget *_, gpointer data)
{
     MainWindow *main_window = (MainWindow *)data;

    if (main_window == NULL)
        errx(EXIT_FAILURE, "NULL pointer in rotation_left()\n");

    Image temp = convert_im2_to_im(*main_window->images->image);
    Image_Rotate(&temp, -M_PI_2);

    main_window->images->image_rotated = convert_im_to_im2(temp);

    main_window->images->image = main_window->images->image_rotated;

    rotateScale -= M_PI_2;
    display_images(GTK_DRAWING_AREA(main_window->pages->page3->image),
                  main_window->images->image_rotated, main_window);

    if (_ == NULL)
        printf(" ");
}

void window_destroy(GtkWidget *_, gpointer data)
{
    /*MainWindow *main_window = (MainWindow *)data;

    free_used_images(main_window);

    if (main_window->images->is_displayed)
    {
        free_Image2(main_window->images->current_image);
    }

    for (int i = 0; i < 9; i++)
        free(main_window->grid[i]);
    free(main_window->grid);

    free(main_window->images->mask);
    free(main_window->images->image);
    free(main_window->images->clean);
    free(main_window->images->image_rotated);
    free(main_window->images->image_rotated_clean);
    free(main_window->images->image_rotated_cropped);
    free(main_window->images->image_solved);
    free(main_window->images->current_image);
    free(main_window->images->current_container);*/

    gtk_main_quit();

    if (data == NULL)
        printf(" ");

    if (_ == NULL)
        printf(" ");
}

void set_sudoku_values(GtkWidget *grid, int sudokuValues[16][16], int hexa) {
    int x = 9;
    int y = 9;
    if (hexa == 1){
        x = 16;
        y = 16;
    }
    
    for (int i = 0; i < x; i++) {
        for (int j = 0; j < y; j++) {
            // Obtenez le numéro actuel dans la grille
            int value = sudokuValues[i][j];

            // Obtenez le widget GtkEntry à la position (i, j)
            GtkWidget *entry = gtk_grid_get_child_at(GTK_GRID(grid), j, i);


            // Vérifiez si le widget existe et est de type GtkEntry
            if (GTK_IS_ENTRY(entry)) {
                // Convertissez la valeur en chaîne de caractères
                gchar *text = g_strdup_printf("%x", value);

                if (value == -1) {
                        text = g_strdup("");
                } else {
                        text = g_strdup_printf("%X", value);
                }
                // Définissez le texte de l'entrée
                gtk_entry_set_text(GTK_ENTRY(entry), text);

                // Libérez la mémoire allouée pour la chaîne
                g_free(text);
            }
        }
    }
}

typedef struct {
    int i;
    int j;
} EntryIndices;

void on_entry_changed(GtkWidget *entry, gpointer user_data) {
    const gchar *text = gtk_entry_get_text(GTK_ENTRY(entry));

    // Récupérer la structure EntryIndices depuis l'entrée
    EntryIndices *indices = g_object_get_data(G_OBJECT(entry), "entry_indices");
    if (indices != NULL) {
        //g_print("Label de la case : %s, %d, %d\n", text, indices->i, indices->j);
        int val = 0;
        sscanf(text, "%X", &val);
        mat[indices->i][indices->j] = val;
    }
    if (entry == NULL || user_data)
        printf(" ");
}

void clear_grid(GtkWidget *grid) {
    GList *children, *iter;
    children = gtk_container_get_children(GTK_CONTAINER(grid));
    for (iter = children; iter != NULL; iter = g_list_next(iter)) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);
}

void create_sudoku_grid(GtkWidget *grid, int hexa) {
    //clear_grid(grid);
    
    // Activer l'homogénéité pour que les lignes et les colonnes aient la même taille
    gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);


    int x = 9;
    int y = 9;
    if (hexa == 1){
        x = 16;
        y = 16;
    }

    //int max_dimension = MAX(x, y);
    gint grid_width = gtk_widget_get_allocated_width(grid);
    gint grid_height = gtk_widget_get_allocated_height(grid);
    gint cell_size = MIN(grid_width / x, grid_height / y);


    for (int i = 0; i < x; i++) {
        //gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
            
        for (int j = 0; j < y; j++) {
            //gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
            
            GtkWidget *entry = gtk_entry_new();
            gtk_entry_set_width_chars(GTK_ENTRY(entry), 1);
            gtk_entry_set_max_length(GTK_ENTRY(entry), 1);

            const gchar *css = "entry { font: 18px Arial; }";  // 12 est la taille, Arial est la police
            GtkCssProvider *provider = gtk_css_provider_new();
            gtk_css_provider_load_from_data(GTK_CSS_PROVIDER(provider), css, -1, NULL);
            gtk_style_context_add_provider(gtk_widget_get_style_context(entry),
                                            GTK_STYLE_PROVIDER(provider),
                                            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
            g_object_unref(provider);


            gtk_entry_set_alignment(GTK_ENTRY(entry), 0.5);
    
            gtk_widget_set_size_request(entry, cell_size, cell_size);

            EntryIndices *indices = g_malloc(sizeof(EntryIndices));
            indices->i = i;
            indices->j = j;

            g_object_set_data(G_OBJECT(entry), "entry_indices", indices);

            g_signal_connect(entry, "changed", G_CALLBACK(on_entry_changed), NULL);

            gtk_grid_attach(GTK_GRID(grid), entry, j, i, 1, 1);
        }
    }
}


void ocr(GtkWidget *_, gpointer data)
{
    MainWindow *main_window = (MainWindow *)data;

    hexa = verifieEtSupprime("./tmp/hexa.txt");
    //printf("HEXA OCR = %d\n", hexa);

    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            mat[i][j] = -1;
        }
    }

    perform_nn(hexa, mat);

    *main_window->images->current_image = SDL_Surface_to_Image2(IMG_Load("../tmp/croped"));

    set_step(main_window->step_indicators, 5);

    set_page(main_window, "page5");

    GtkViewport *viewport = GTK_VIEWPORT(main_window->pages->page5->container);
    GList *child = gtk_container_get_children(GTK_CONTAINER(viewport));

    GtkWidget *box = GTK_WIDGET(child->data);

    // Obtenez la liste des enfants du conteneur (box)
    GList *children = gtk_container_get_children(GTK_CONTAINER(box));

        // Vérifiez si la liste n'est pas vide
    cont_grid = GTK_WIDGET(children->data);

    g_list_free(child);
    g_list_free(children);

    grid = gtk_grid_new();
     // Ajoutez la nouvelle grille à la fenêtre principale
    gtk_container_add(GTK_CONTAINER(cont_grid), grid);

    create_sudoku_grid(grid, hexa);

    set_sudoku_values(grid, mat, hexa);

    gtk_widget_show_all(main_window->window);

    if (_ == NULL)
        printf(" ");
}

int** createSudokuFromHexa(int mat[16][16]){
    int** sud = malloc(9 * sizeof(int*)); // Allocate memory for rows
    for (int i = 0; i < 9; i++) {
        sud[i] = malloc(9 * sizeof(int)); // Allocate memory for columns
    }
    for (int i = 0; i < 9; i ++){
        for (int j = 0; j < 9; j ++){
            sud[i][j] = mat[i][j];
            if (sud[i][j] < 0)
                sud[i][j] = 0;
        }
    }
    return sud;
}

//this function returns 0 if there is multiple times the same digit in a row, column or square
int gridIsValid(int** grid){
    int* tab = malloc(9 * sizeof(int));
    for (int i = 0; i < 9; i ++){
        for (int j = 0; j < 9; j ++){
            tab[j] = 0;
        }
        for (int j = 0; j < 9; j ++){
            if (grid[i][j] != 0){
                if (tab[grid[i][j] - 1] == 1){
                    return 0;
                }
                else{
                    tab[grid[i][j] - 1] = 1;
                }
            }
        }
    }
    for (int i = 0; i < 9; i ++){
        for (int j = 0; j < 9; j ++){
            tab[j] = 0;
        }
        for (int j = 0; j < 9; j ++){
            if (grid[j][i] != 0){
                if (tab[grid[j][i] - 1] == 1){
                    return 0;
                }
                else{
                    tab[grid[j][i] - 1] = 1;
                }
            }
        }
    }
    for (int i = 0; i < 9; i += 3){
        for (int j = 0; j < 9; j += 3){
            for (int k = 0; k < 9; k ++){
                tab[k] = 0;
            }
            for (int k = 0; k < 9; k ++){
                if (grid[i + k / 3][j + k % 3] != 0){
                    if (tab[grid[i + k / 3][j + k % 3] - 1] == 1){
                        return 0;
                    }
                    else{
                        tab[grid[i + k / 3][j + k % 3] - 1] = 1;
                    }
                }
            }
        }
    }
    return 1;
}

char** createCharMatrixFromIntMatrix(int mat[][16]){
    char** sud = malloc(16 * sizeof(char*)); // Allocate memory for rows
    for (int i = 0; i < 16; i++) {
        sud[i] = malloc(16 * sizeof(char)); // Allocate memory for columns
    }
    for (int i = 0; i < 16; i ++){
        for (int j = 0; j < 16; j ++){
            if (mat[i][j] < 10){
                sud[i][j] = (char)(mat[i][j] + '0');
            }
            else{
                sud[i][j] = (mat[i][j] - 10 + 'A');
            }
            //sud[i][j] = mat[i][j];
            if (mat[i][j] < 0)
                sud[i][j] = 'X';
        }
    }
    return sud;
}

int gridIsValidHexa(char** grid){
    int row, col, i, j;

    // Vérifier les lignes
    for (row = 0; row < 16; row++) {
        int used[16] = {0};
        for (col = 0; col < 16; col++) {
            char current = grid[row][col];
            if (current != 'X') {
                int index = (current >= 'A') ? (current - 'A' + 10) : (current - '0');
                if (used[index]) {
                    return 0;  // Le chiffre est déjà utilisé dans la ligne
                } else {
                    used[index] = 1;
                }
            }
        }
    }

    // Vérifier les colonnes
    for (col = 0; col < 16; col++) {
        int used[16] = {0};
        for (row = 0; row < 16; row++) {
            char current = grid[row][col];
            if (current != 'X') {
                int index = (current >= 'A') ? (current - 'A' + 10) : (current - '0');
                if (used[index]) {
                    return 0;  // Le chiffre est déjà utilisé dans la colonne
                } else {
                    used[index] = 1;
                }
            }
        }
    }

    // Vérifier les blocs 4x4
    for (i = 0; i < 16; i += 4) {
        for (j = 0; j < 16; j += 4) {
            int used[16] = {0};
            for (row = 0; row < 4; row++) {
                for (col = 0; col < 4; col++) {
                    char current = grid[i + row][j + col];
                    if (current != 'X') {
                        int index = (current >= 'A') ? (current - 'A' + 10) : (current - '0');
                        if (used[index]) {
                            return 0;  // Le chiffre est déjà utilisé dans le bloc
                        } else {
                            used[index] = 1;
                        }
                    }
                }
            }
        }
    }

    return 1;  // La grille est valide
}

void sudoku(GtkWidget *_, gpointer data){

    MainWindow *main_window = (MainWindow *)data;

    if (chdir("..") == 0) {}    

    char buffer[4096];  // Vous pouvez ajuster la taille du tampon selon vos besoins

    if (getcwd(buffer, sizeof(buffer)) != NULL) {
        //printf("Le répertoire de travail actuel est : %s\n", buffer);
    } else {
        perror("getcwd() a échoué");
    }

    strcat(buffer, "SudokuSolver");
    int solved = 0;
    if (hexa == 0){
        int** sud = createSudokuFromHexa(mat);
        if(gridIsValid(sud)) {
            print_grid(sud);
            solved = solve_sudoku(sud, 0, 0);
            print_grid(sud);
            for (int i = 0; i < 9; i ++){
                for (size_t j = 0; j < 9; j++)
                {
                    display[i][j] = sud[i][j];
                }
            }
        }
        else{
            printf("Invalid grid or non solvable sudoku\n");
        }
    }
    else{
        char** sud = createCharMatrixFromIntMatrix(mat);
        dispBoard(sud);
        if(gridIsValidHexa(sud)) {
            solved = !suducul(sud);
            dispBoard(sud);
            for (int i = 0; i < 16; i ++){
                for (size_t j = 0; j < 16; j++)
                {
                    if (sud[i][j] > '9')
                        display[i][j] = sud[i][j] - 'A';
                    else
                        display[i][j] = sud[i][j] - '0';
                }
            }
        }
        else{
            printf("Invalid grid or non solvable hexadoku\n");
        }

    }

    set_step(main_window->step_indicators, 6);

    set_page(main_window, "page6");
    
    Image black = Image_CreateBlack(400, 400);
    globalRes = Image_CreateSurface(&black);
    globalRes = draw_sudoku_grid(globalRes, display, hexa == 0? 9 : 16);
    //*main_window->images->current_image = SDL_Surface_to_Image2(surface);
    IMG_SavePNG(globalRes, "./tmp/res");
    GtkViewport *viewport = GTK_VIEWPORT(main_window->pages->page6->container);
    GList *child = gtk_container_get_children(GTK_CONTAINER(viewport));

    box = GTK_WIDGET(child->data);

    // Obtenez la liste des enfants du conteneur (box)
    GList *children = gtk_container_get_children(GTK_CONTAINER(box));

   // GtkWidget *cont = GTK_WIDGET(children->data);

    g_list_free(child);
    g_list_free(children);

    if (solved == 1)
        image = gtk_image_new_from_file("./tmp/res");
    else 
        image = gtk_image_new_from_file("./res/not_found.png");

    // Ajouter l'image à la boîte
    gtk_box_pack_start(GTK_BOX(box), image, TRUE, TRUE, 0);

    gtk_widget_show_all(main_window->window);

    //printf("SOLVED : %d\n", solved);

    if (_ == NULL || data == NULL)
        printf(" ");    
}

void save(GtkWidget *_, gpointer data){

    IMG_SavePNG(globalRes, "OUTPUT");

    if (_ == NULL || data == NULL)
        printf(" ");
}

void restart(GtkWidget *_, gpointer data)
{
    MainWindow *main_window = (MainWindow *)data;

    reset_button(main_window->controls->image_rotation_done_button);

    set_step(main_window->step_indicators, 1);

    set_page(main_window, "page1");

    if (detected == 1){
        gtk_container_remove(GTK_CONTAINER(box), image);
        gtk_container_remove(GTK_CONTAINER(cont_grid), grid);
    }
    if (_ == NULL || data == NULL)
        printf(" ");
}

int main(int argc, char *argv[]) {
    // Initialisation de GTK
    gtk_init(&argc, &argv);

    // Charger le fichier Glade
    GtkBuilder *builder = gtk_builder_new();
    GError *error = NULL;


    if (!gtk_builder_add_from_file(builder, "glade/main.glade", &error)) {
        g_critical("Impossible de charger le fichier Glade : %s", error->message);
        g_error_free(error);
        return 1;
    }

    // Création de la fenêtre principale
    GtkWindow *window
            = GTK_WINDOW(gtk_builder_get_object(builder, "sudokuocr"));

    GtkButton *save_button
            = GTK_BUTTON(gtk_builder_get_object(builder, "save_button"));
    GtkButton *open_button
            = GTK_BUTTON(gtk_builder_get_object(builder, "openbutton"));
    GtkButton *confirm_image_button
            = GTK_BUTTON(gtk_builder_get_object(builder, "confirmimagebutton"));
    GtkButton *cancel_image_button
            = GTK_BUTTON(gtk_builder_get_object(builder, "cancelimagebutton"));
    GtkButton *rotation_done_button
            = GTK_BUTTON(gtk_builder_get_object(builder, "rotationdonebutton"));
    GtkButton *confirm_crop_button
            = GTK_BUTTON(gtk_builder_get_object(builder, "confirmcropbutton"));
    GtkButton *correction_done_button
            = GTK_BUTTON(gtk_builder_get_object(builder, "solve"));

    GtkButton *restart_button
            = GTK_BUTTON(gtk_builder_get_object(builder, "restart_button"));

    GtkEventBox *crop_event_box
            = GTK_EVENT_BOX(gtk_builder_get_object(builder, "cropeventbox"));

    GtkFileChooserButton *file_chooser_button = GTK_FILE_CHOOSER_BUTTON(
            gtk_builder_get_object(builder, "fileselector"));

    GtkButton *rotation_left
            = GTK_BUTTON(gtk_builder_get_object(builder, "rotationLeft"));
    GtkButton *rotation_right
            = GTK_BUTTON(gtk_builder_get_object(builder, "rotationRight"));

    GtkGrid *sudoku_labels_grid
            = GTK_GRID(gtk_builder_get_object(builder, "sudokulabelsgrid"));

    GtkWidget *step_1_indicator
            = GTK_WIDGET(gtk_builder_get_object(builder, "step1indicator"));
    GtkWidget *step_2_indicator
            = GTK_WIDGET(gtk_builder_get_object(builder, "step2indicator"));
    GtkWidget *step_3_indicator
            = GTK_WIDGET(gtk_builder_get_object(builder, "step3indicator"));
    GtkWidget *step_4_indicator
            = GTK_WIDGET(gtk_builder_get_object(builder, "step4indicator"));
    GtkWidget *step_5_indicator
            = GTK_WIDGET(gtk_builder_get_object(builder, "step5indicator"));
    GtkWidget *step_6_indicator
            = GTK_WIDGET(gtk_builder_get_object(builder, "step6indicator"));

    GtkStack *stack = GTK_STACK(gtk_builder_get_object(builder, "stack1"));

    GtkViewport *page1_container
            = GTK_VIEWPORT(gtk_builder_get_object(builder, "page1"));
    GtkViewport *page2_container
            = GTK_VIEWPORT(gtk_builder_get_object(builder, "page2"));
    GtkViewport *page3_container
            = GTK_VIEWPORT(gtk_builder_get_object(builder, "page3"));
    GtkViewport *page4_container
            = GTK_VIEWPORT(gtk_builder_get_object(builder, "page4"));
    GtkViewport *page5_container
            = GTK_VIEWPORT(gtk_builder_get_object(builder, "page5"));
    GtkViewport *page6_container
            = GTK_VIEWPORT(gtk_builder_get_object(builder, "page6"));

    GtkDrawingArea *page2_image
            = GTK_DRAWING_AREA(gtk_builder_get_object(builder, "page2image"));
    GtkDrawingArea *page3_image
            = GTK_DRAWING_AREA(gtk_builder_get_object(builder, "page3image"));
    GtkDrawingArea *page4_image
            = GTK_DRAWING_AREA(gtk_builder_get_object(builder, "page4image"));
    GtkDrawingArea *page5_image
            = GTK_DRAWING_AREA(gtk_builder_get_object(builder, "page5image"));
    GtkDrawingArea *page6_image
            = GTK_DRAWING_AREA(gtk_builder_get_object(builder, "page6image1"));
  
    GtkLabel *page2_label
            = GTK_LABEL(gtk_builder_get_object(builder, "page2label"));
    GtkLabel *page3_label
            = GTK_LABEL(gtk_builder_get_object(builder, "page3label"));
    GtkLabel *page4_label
            = GTK_LABEL(gtk_builder_get_object(builder, "page4label"));
    GtkLabel *page5_label
            = GTK_LABEL(gtk_builder_get_object(builder, "page5label"));
    GtkLabel *page6_label
            = GTK_LABEL(gtk_builder_get_object(builder, "page6label"));



    StepIndicators step_indicators = {
            .current_step = 1,
            .step_1_indicator = step_1_indicator,
            .step_2_indicator = step_2_indicator,
            .step_3_indicator = step_3_indicator,
            .step_4_indicator = step_4_indicator,
            .step_5_indicator = step_5_indicator,
            .step_6_indicator = step_6_indicator,
    };

    Controls controls = {
            .save_button = save_button,
            .open_button = open_button,
            .file_chooser_button = file_chooser_button,
            .confirm_image_button = confirm_image_button,
            .cancel_image_button = cancel_image_button,
            .rotation_left = rotation_left,
            .rotation_right = rotation_right,
            .image_rotation_done_button = rotation_done_button,
            .confirm_crop_button = confirm_crop_button,
            .correction_done_button = correction_done_button,
            .restart_button = restart_button,
            .crop_event_box = crop_event_box,
    };

    Page page1 = {
            .container = page1_container,
            .image = NULL,
            .label = NULL,
    };
    Page page2 = {
            .container = page2_container,
            .image = page2_image,
            .label = page2_label,
    };
    Page page3 = {
            .container = page3_container,
            .image = page3_image,
            .label = page3_label,
    };
    Page page4 = {
            .container = page4_container,
            .image = page4_image,
            .label = page4_label,
    };
    Page page5 = {
            .container = page5_container,
            .image = page5_image,
            .label = page5_label,
    };
    Page page6 = {
            .container = page6_container,
            .image = page6_image,
            .label = page6_label,
    };

    Pages pages = {
            .current_page = "page1",
            .page1 = &page1,
            .page2 = &page2,
            .page3 = &page3,
            .page4 = &page4,
            .page5 = &page5,
            .page6 = &page6,
    };

    Images images = {
            .is_rotated = FALSE,
            .is_loaded = FALSE,
            .is_displayed = FALSE,
            .is_grid_detected = FALSE,
            .is_perspective_corrected = FALSE,
            .is_solved = FALSE,
            .current_rotation = 0,
            .mask = malloc(sizeof(Image2)),
            .image = malloc(sizeof(Image2)),
            .clean = malloc(sizeof(Image2)),
            .image_rotated = malloc(sizeof(Image2)),
            .image_rotated_clean = malloc(sizeof(Image2)),
            .image_rotated_cropped = malloc(sizeof(Image2)),
            .image_solved = malloc(sizeof(Image2)),
            .display_size = 0,
            .current_container = malloc(sizeof(GtkDrawingArea)),
            .current_image = malloc(sizeof(Image2)),
            .current_label = NULL,
            .editing_digits = FALSE,
    };

    MainWindow main_window = {
            .window = GTK_WIDGET(window),
            .stack = stack,
            .sudoku_labels_grid = sudoku_labels_grid,
            .step_indicators = &step_indicators,
            .controls = &controls,
            .pages = &pages,
            .images = &images,
            .cropping = FALSE,
    };


    // Connect signals
    g_signal_connect(
            window, "destroy", G_CALLBACK(window_destroy), &main_window);

    g_signal_connect(confirm_image_button, "clicked",
                     G_CALLBACK(manual_rotate_image), &main_window);
    
    g_signal_connect(confirm_crop_button, "clicked",
                G_CALLBACK(ocr), &main_window);

    // File selection
    g_signal_connect(file_chooser_button, "selection-changed",
                     G_CALLBACK(file_selected), &main_window);

    // Cancel image
    g_signal_connect(cancel_image_button, "clicked",
                     G_CALLBACK(cancel_image_selection), &main_window);


    // Rotation done
    g_signal_connect(rotation_done_button, "clicked",
                     G_CALLBACK(process_image), &main_window);

    // Rotation Scale
    g_signal_connect(rotation_left, "clicked",
                     G_CALLBACK(rotation_gauche), &main_window);
    g_signal_connect(rotation_right, "clicked",
                   G_CALLBACK(rotation_droite), &main_window);
     g_signal_connect(save_button, "clicked",
                   G_CALLBACK(save), &main_window);

    g_signal_connect(restart_button, "clicked",
                     G_CALLBACK(restart), &main_window);


    // Images Drawing Area
    g_signal_connect(
            page2_image, "draw", G_CALLBACK(draw_image), &main_window);
    g_signal_connect(
            page3_image, "draw", G_CALLBACK(draw_image), &main_window);
    g_signal_connect(
            page4_image, "draw", G_CALLBACK(draw_image), &main_window);
    g_signal_connect(
        page5_image, "draw", G_CALLBACK(draw_image), &main_window); 
    g_signal_connect(
        page6_image, "draw", G_CALLBACK(draw_image), &main_window);


     g_signal_connect(G_OBJECT(correction_done_button), "clicked",
        G_CALLBACK(sudoku), &main_window);        

    // Affichage de la fenêtre principale
    gtk_widget_show_all(GTK_WIDGET(window));

    // Lancement de la boucle principale GTK
    gtk_main();

    // Libération des ressources
    g_object_unref(builder);

    return 0;
}

