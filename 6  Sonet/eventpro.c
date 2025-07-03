#include <gtk/gtk.h>
#include <stdio.h>
#define _GNU_SOURCE
#include <string.h>
#include <ctype.h>
#define _XOPEN_SOURCE
#include <time.h>


#define MAX_USERNAME_LENGTH 50
#define MAX_PASSWORD_LENGTH 50
#define MAX_EMAIL_LENGTH 100
#define PROFILE_FILE "user_profile.txt"

// Global variable to store the notebook and sidebar buttons
GtkWidget *global_workspace = NULL;

// Global variables for notification system
static GList *notifications_list = NULL;
static GtkWidget *global_notification_button = NULL;  // New global variable for button reference

typedef struct {
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    char email[MAX_EMAIL_LENGTH];
    char birthday[20];
} UserProfile;

// Notification structure
typedef struct {
    char *message;
    char *timestamp;
    gboolean is_read;
} Notification;

// Global widget and timer declarations
typedef struct {
    GtkWidget *window;
    GtkWidget *progress_bar;
    GtkWidget *loading_label;
    guint progress_timer_id;
    guint hint_timer_id;
} LoadingScreenWidgets;

// Structure to hold event details
typedef struct {
    long event_id;
    char name[256];
    char date[256];
    char location[256];
    char category[256];
    char status[256];
    int attendees;
    double budget;
} Event;

// Structure to hold dashboard statistics
typedef struct {
    int total_events;
    int active_events;
    int team_members;
    int upcoming_events;
} DashboardStats;


// Function prototypes (keep existing ones)
static gboolean update_progress(gpointer data);
static gboolean update_loading_text(gpointer data);
static void create_main_window(GtkWidget *loading_window);
static void show_loading_screen(void);
static void setup_window_styling(GtkWidget *window, const char *bg_color);
void show_credits_dialog(GtkWidget *widget, gpointer data);
void show_settings_dialog(GtkWidget *widget, gpointer data);
void show_reports_window(GtkWidget *widget, gpointer data);
void generate_report(GtkWidget *widget, gpointer data);
Notification* create_notification(const char *message);
void free_notification(Notification *notif);
void add_notification(const char *message);
void setup_notifications_button(GtkWidget *button, GtkWidget *parent_window);
void trigger_notification(const char *message);
static void show_notifications_dialog(GtkWidget *button, gpointer user_data);


// Helper function to export the report (you'll need to declare this in your header file)
void export_report(GtkWidget *widget, gpointer data) {
    GtkTextView *text_view = GTK_TEXT_VIEW(data);
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(text_view);
    
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    char *report_text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
    
    // Get current time for filename
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char filename[100];
    snprintf(filename, sizeof(filename), "EventPro_Report_%04d%02d%02d_%02d%02d%02d.txt",
        t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
        t->tm_hour, t->tm_min, t->tm_sec);
    
    // Create file chooser dialog
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Save Report",
        NULL,
        GTK_FILE_CHOOSER_ACTION_SAVE,
        "Cancel", GTK_RESPONSE_CANCEL,
        "Save", GTK_RESPONSE_ACCEPT,
        NULL);
    
    // Set suggested filename
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), filename);
    
    // Show the dialog and wait for response
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        FILE *f = fopen(filename, "w");
        if (f) {
            fprintf(f, "%s", report_text);
            fclose(f);
            
            // Show success message
            GtkWidget *message = gtk_message_dialog_new(NULL,
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_INFO,
                GTK_BUTTONS_OK,
                "Report successfully exported to:\n%s", filename);
            gtk_dialog_run(GTK_DIALOG(message));
            gtk_widget_destroy(message);
        }
        g_free(filename);
    }
    
    // Clean up
    gtk_widget_destroy(dialog);
    g_free(report_text);
}


// Function to save user profile to a file
int save_user_profile(UserProfile *profile) {
    FILE *file = fopen(PROFILE_FILE, "w");
    if (file == NULL) {
        g_print("Error: Could not open file for writing.\n");
        return 0;
    }

    fprintf(file, "%s\n%s\n%s\n%s\n", 
        profile->username, 
        profile->password, 
        profile->email, 
        profile->birthday);
    
    fclose(file);
    return 1;
}

// Function to load user profile from file
int load_user_profile(UserProfile *profile) {
    FILE *file = fopen(PROFILE_FILE, "r");
    if (file == NULL) {
        g_print("No existing profile found.\n");
        return 0;
    }

    if (fscanf(file, "%49s\n%49s\n%99s\n%19s", 
        profile->username, 
        profile->password, 
        profile->email, 
        profile->birthday) != 4) {
        g_print("Error reading profile data.\n");
        fclose(file);
        return 0;
    }
    
    fclose(file);
    return 1;
}

// User Registration Dialog
static void show_registration_dialog(GtkWidget *parent_window) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "User Registration", 
        GTK_WINDOW(parent_window), 
        GTK_DIALOG_MODAL, 
        "Register", GTK_RESPONSE_OK, 
        "Cancel", GTK_RESPONSE_CANCEL, 
        NULL
    );

    // Set dialog size and make it look more modern
    gtk_window_set_default_size(GTK_WINDOW(dialog), 500, 650);
    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_set_border_width(GTK_CONTAINER(content_area), 20);

    // Main vertical box for content
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_container_add(GTK_CONTAINER(content_area), main_box);

    // Registration header
    GtkWidget *header_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(header_label), 
        "<span font='24' weight='bold'>Create Your Profile</span>");
    gtk_box_pack_start(GTK_BOX(main_box), header_label, FALSE, FALSE, 10);

    // Subtitle
    GtkWidget *subtitle_label = gtk_label_new("Fill in your details to get started");
    gtk_box_pack_start(GTK_BOX(main_box), subtitle_label, FALSE, FALSE, 5);

    // Create a grid for input fields
    GtkWidget *input_grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(input_grid), 10);
    gtk_grid_set_row_spacing(GTK_GRID(input_grid), 10);

    // Input fields with labels
    const char *labels[] = {
        "Username", "Email", "Password", "Confirm Password", "Birthday"
    };

    GtkWidget *entries[5];
    GtkWidget *icons[5];

    // Predefined icons (you might want to use actual icon files or GTK icons)
    const char *icon_chars[] = {"👤", "✉️", "🔐", "🔐", "📅"};
    
    for (int i = 0; i < 5; i++) {
        // Icon label
        icons[i] = gtk_label_new(icon_chars[i]);
        gtk_grid_attach(GTK_GRID(input_grid), icons[i], 0, i, 1, 1);

        // Create entry
        entries[i] = gtk_entry_new();
        gtk_entry_set_placeholder_text(GTK_ENTRY(entries[i]), labels[i]);
        
        // Special handling for password fields
        if (i == 2 || i == 3) {
            gtk_entry_set_visibility(GTK_ENTRY(entries[i]), FALSE);
        }
        
        // Special handling for birthday
        if (i == 4) {
            gtk_entry_set_placeholder_text(GTK_ENTRY(entries[i]), "YYYY-MM-DD");
        }

        gtk_grid_attach(GTK_GRID(input_grid), entries[i], 1, i, 1, 1);
    }

    // Add input grid to main box
    gtk_box_pack_start(GTK_BOX(main_box), input_grid, FALSE, FALSE, 10);

    // Terms and Conditions checkbox
    GtkWidget *terms_check = gtk_check_button_new_with_label(
        "I agree to the Terms and Conditions"
    );
    gtk_box_pack_start(GTK_BOX(main_box), terms_check, FALSE, FALSE, 5);

    // CSS Styling
    GtkCssProvider *css_provider = gtk_css_provider_new();
    const gchar *css_data = 
    "entry {"
    "    background-color: #f5f5f5;"
    "    border: 1px solid #ddd;"
    "    border-radius: 5px;"
    "    padding: 10px;"
    "    font-size: 16px;"
    "    transition: all 0.3s ease;"
    "}"
    "button {"  
    "    transition: all 0.4s ease;"
    "}"
;

    gtk_css_provider_load_from_data(css_provider, css_data, -1, NULL);
    
    GdkScreen *screen = gdk_screen_get_default();
    gtk_style_context_add_provider_for_screen(screen, 
        GTK_STYLE_PROVIDER(css_provider), 
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    
    g_object_unref(css_provider);

    // Show all widgets
    gtk_widget_show_all(content_area);

    // Run dialog
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));

    if (result == GTK_RESPONSE_OK) {
        UserProfile new_profile;

        // Copy input values to profile struct
        strncpy(new_profile.username, 
            gtk_entry_get_text(GTK_ENTRY(entries[0])), 
            MAX_USERNAME_LENGTH - 1);
        
        strncpy(new_profile.email, 
            gtk_entry_get_text(GTK_ENTRY(entries[1])), 
            MAX_EMAIL_LENGTH - 1);
        
        strncpy(new_profile.password, 
            gtk_entry_get_text(GTK_ENTRY(entries[2])), 
            MAX_PASSWORD_LENGTH - 1);
        
        strncpy(new_profile.birthday, 
            gtk_entry_get_text(GTK_ENTRY(entries[4])), 
            sizeof(new_profile.birthday) - 1);

        // Basic validation (you might want to add more robust validation)
        if (strlen(new_profile.username) > 0 && 
            strlen(new_profile.email) > 0 && 
            strlen(new_profile.password) > 0) {
            // Compare password fields
            const gchar *password = gtk_entry_get_text(GTK_ENTRY(entries[2]));
            const gchar *confirm_password = gtk_entry_get_text(GTK_ENTRY(entries[3]));
            
            if (strcmp(password, confirm_password) == 0) {
                // Save profile
                if (save_user_profile(&new_profile)) {
                    g_print("Profile saved successfully!\n");
                    trigger_notification("New Profile Updated!");
                }
            } else {
                // Show error if passwords don't match
                GtkWidget *error_dialog = gtk_message_dialog_new(
                    GTK_WINDOW(dialog),
                    GTK_DIALOG_MODAL,
                    GTK_MESSAGE_ERROR,
                    GTK_BUTTONS_OK,
                    "Passwords do not match!"
                );
                gtk_dialog_run(GTK_DIALOG(error_dialog));
                gtk_widget_destroy(error_dialog);
            }
        }
    }

    // Destroy dialog
    gtk_widget_destroy(dialog);
}

// User Profile View/Edit Dialog
static void show_profile_dialog(GtkWidget *parent_window) {
    UserProfile current_profile;
    
    if (!load_user_profile(&current_profile)) {
        show_registration_dialog(parent_window);
        return;
    }

    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "User Profile", 
        GTK_WINDOW(parent_window),
        GTK_DIALOG_MODAL | GTK_DIALOG_USE_HEADER_BAR,
        "Edit Profile", GTK_RESPONSE_ACCEPT,
        "Close", GTK_RESPONSE_CLOSE,
        NULL
    );

    // Slightly reduced window size since we removed the icon
    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 450);
    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_set_border_width(GTK_CONTAINER(content_area), 24);

    // Main container
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_name(main_box, "profile-container");
    gtk_container_add(GTK_CONTAINER(content_area), main_box);

    // Username section with larger font
    GtkWidget *username_label = gtk_label_new(NULL);
    gchar *username_markup = g_markup_printf_escaped(
        "<span font='24' weight='bold' color='#2D3748'>%s</span>", 
        current_profile.username
    );
    gtk_label_set_markup(GTK_LABEL(username_label), username_markup);
    gtk_widget_set_margin_top(username_label, 10);
    gtk_widget_set_margin_bottom(username_label, 5);
    gtk_box_pack_start(GTK_BOX(main_box), username_label, FALSE, FALSE, 0);
    g_free(username_markup);

    // Separator with increased margins
    GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_set_margin_top(separator, 10);
    gtk_widget_set_margin_bottom(separator, 15);
    gtk_box_pack_start(GTK_BOX(main_box), separator, FALSE, FALSE, 0);

    // Profile details section with improved spacing and fonts
    GtkWidget *details_grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(details_grid), 20);  // Increased spacing
    gtk_grid_set_row_spacing(GTK_GRID(details_grid), 18);    // Increased spacing
    gtk_widget_set_margin_start(details_grid, 15);
    gtk_widget_set_margin_end(details_grid, 15);

    const char *labels[] = {"📧 Email:", "🎂 Birthday:", "📅 Member Since:"};
    const char *values[] = {
        current_profile.email,
        current_profile.birthday,
        "January 1, 2024"
    };

    for (int i = 0; i < 3; i++) {
        // Label with larger font
        GtkWidget *label = gtk_label_new(NULL);
        gchar *label_markup = g_markup_printf_escaped(
            "<span font='12' weight='bold' color='#4A5568'>%s</span>",
            labels[i]
        );
        gtk_label_set_markup(GTK_LABEL(label), label_markup);
        gtk_widget_set_halign(label, GTK_ALIGN_START);
        g_free(label_markup);

        // Value with larger font
        GtkWidget *value_label = gtk_label_new(NULL);
        gchar *value_markup = g_markup_printf_escaped(
            "<span font='12' color='#4A5568'>%s</span>",
            values[i]
        );
        gtk_label_set_markup(GTK_LABEL(value_label), value_markup);
        gtk_widget_set_halign(value_label, GTK_ALIGN_START);
        g_free(value_markup);

        gtk_grid_attach(GTK_GRID(details_grid), label, 0, i, 1, 1);
        gtk_grid_attach(GTK_GRID(details_grid), value_label, 1, i, 1, 1);
    }

    gtk_box_pack_start(GTK_BOX(main_box), details_grid, FALSE, FALSE, 5);

    // Action buttons with improved styling
    GtkWidget *action_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);
    gtk_widget_set_halign(action_box, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top(action_box, 25);  // Increased margin
    gtk_widget_set_margin_bottom(action_box, 10);

    const char *action_buttons[] = {"🔐 Change Password", "📷 Change Avatar"};
    for (int i = 0; i < 2; i++) {
        GtkWidget *button = gtk_button_new_with_label(action_buttons[i]);
        gtk_widget_set_name(button, "profile-action-button");
        gtk_box_pack_start(GTK_BOX(action_box), button, FALSE, FALSE, 0);
    }

    gtk_box_pack_start(GTK_BOX(main_box), action_box, FALSE, FALSE, 5);

    // Enhanced CSS styling
    GtkCssProvider *css_provider = gtk_css_provider_new();
    const gchar *css_data = 
        "box#profile-container {"
        "    background-color: white;"
        "    border-radius: 8px;"
        "    padding: 20px;"
        "    margin: 0;"
        "}"
        "button#profile-action-button {"
        "    background: linear-gradient(to bottom, #f8f9fa, #e9ecef);"
        "    color: #2D3748;"  // Darker text for better readability
        "    border: 1px solid #dee2e6;"
        "    border-radius: 6px;"
        "    padding: 10px 20px;"  // Increased padding
        "    font-size: 13px;"     // Increased font size
        "    font-weight: 500;"    // Slightly bolder
        "    box-shadow: 0 2px 4px rgba(0,0,0,0.05);"
        "    transition: all 0.2s ease;"
        "}"
        "button#profile-action-button:hover {"
        "    background: linear-gradient(to bottom, #e9ecef, #dee2e6);"
        "    border-color: #ced4da;"
        "    box-shadow: 0 4px 8px rgba(0,0,0,0.1);"
        "}";

    gtk_css_provider_load_from_data(css_provider, css_data, -1, NULL);
    
    GdkScreen *screen = gdk_screen_get_default();
    gtk_style_context_add_provider_for_screen(
        screen,
        GTK_STYLE_PROVIDER(css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    
    g_object_unref(css_provider);

    gtk_widget_show_all(dialog);
    
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (result == GTK_RESPONSE_ACCEPT) {
        show_registration_dialog(dialog);
    }

    gtk_widget_destroy(dialog);
}

// Callback function to switch to a specific notebook page
void switch_to_notebook_page(GtkWidget *widget, gpointer data) {
    if (!global_workspace) return;
    
    int page_index = GPOINTER_TO_INT(data);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(global_workspace), page_index);
}

// Improved date validation function
gboolean validate_date(const gchar *date_str) {
    // Check if date_str is NULL or empty
    if (date_str == NULL || strlen(date_str) == 0) return FALSE;
    
    // Check exact length
    if (strlen(date_str) != 10) return FALSE;
    
    // Check format
    if (date_str[4] != '-' || date_str[7] != '-') return FALSE;
    
    // Check year, month, day are numeric
    for (int i = 0; i < 10; i++) {
        if (i == 4 || i == 7) continue;
        if (date_str[i] < '0' || date_str[i] > '9') return FALSE;
    }
    
    // Parse components
    int year = atoi(date_str);
    int month = atoi(date_str + 5);
    int day = atoi(date_str + 8);
    
    // Enhanced validation checks
    if (year < 1900 || year > 2100) return FALSE;
    if (month < 1 || month > 12) return FALSE;
    
    // Days in each month
    int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    
    // Leap year check (more robust)
    if (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) {
        days_in_month[1] = 29;
    }
    
    if (day < 1 || day > days_in_month[month-1]) return FALSE;
    
    // Optional: Check if date is not in the past
    time_t now = time(NULL);
    struct tm *current_time = localtime(&now);
    
    struct tm event_time = {0};
    event_time.tm_year = year - 1900;
    event_time.tm_mon = month - 1;
    event_time.tm_mday = day;
    
    time_t event_timestamp = mktime(&event_time);
    
    // Optional: Uncomment if you want to prevent past dates
    // if (event_timestamp < now) return FALSE;
    
    return TRUE;
}


// Enhanced file saving function with error handling
void save_event_to_file(const gchar *event_name, const gchar *event_date, 
                         const gchar *location, const gchar *category, 
                         const gchar *status, gint attendees, gdouble budget) {
    // Check for NULL or empty inputs
    if (!event_name || strlen(event_name) == 0) {
        g_print("Error: Event name cannot be empty.\n");
        return;
    }

    FILE *file = fopen("events.txt", "a");
    if (file == NULL) {
        g_print("Error: Unable to open events.txt for writing!\n");
        return;
    }

    // Generate unique event ID (timestamp-based)
    time_t now = time(NULL);
    
    // Use fprintf with error checking
    int result = 0;
    result += fprintf(file, "Event ID: %ld\n", now);
    result += fprintf(file, "Name: %s\n", event_name);
    result += fprintf(file, "Date: %s\n", event_date);
    result += fprintf(file, "Location: %s\n", location ? location : "N/A");
    result += fprintf(file, "Category: %s\n", category);
    result += fprintf(file, "Status: %s\n", status);
    result += fprintf(file, "Attendees: %d\n", attendees);
    result += fprintf(file, "Budget: %.2f\n", budget);
    result += fprintf(file, "---\n");  // Separator between events

    // Check if all writes were successful
    if (result < 0) {
        g_print("Error: Failed to write to events.txt\n");
    }

    fclose(file);
}

static void on_add_event_button_clicked(GtkWidget *widget, gpointer data) {
    // Create a dialog for adding an event
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Add Event",
        GTK_WINDOW(data), // Parent window
        GTK_DIALOG_MODAL,
        "Add", GTK_RESPONSE_OK,
        "Cancel", GTK_RESPONSE_CANCEL,
        NULL
    );

    // Set dialog size
    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 500);
    
    // Create a grid for input fields
    GtkWidget *input_grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(input_grid), 10);
    gtk_grid_set_row_spacing(GTK_GRID(input_grid), 10);
    
    // Create input fields for event details
    GtkWidget *event_name_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(event_name_entry), "Event Name");
    
    GtkWidget *event_date_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(event_date_entry), "Event Date (YYYY-MM-DD)");
    
    GtkWidget *location_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(location_entry), "Event Location");
    
    // Category Combo Box
    GtkWidget *category_combo = gtk_combo_box_text_new();
    const char *categories[] = {"Conference", "Workshop", "Seminar", "Social", "Other"};
    for (int i = 0; i < G_N_ELEMENTS(categories); i++) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(category_combo), categories[i]);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(category_combo), 0);
    
    // Status Combo Box
    GtkWidget *status_combo = gtk_combo_box_text_new();
    const char *statuses[] = {"Planned", "Ongoing", "Completed", "Cancelled"};
    for (int i = 0; i < G_N_ELEMENTS(statuses); i++) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(status_combo), statuses[i]);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(status_combo), 0);
    
    // Attendees Spin Button
    GtkWidget *attendees_spin = gtk_spin_button_new_with_range(0, 1000, 1);
    
    // Budget Spin Button
    GtkWidget *budget_spin = gtk_spin_button_new_with_range(0, 1000000, 100);
    
    // Add input fields to the grid
    gtk_grid_attach(GTK_GRID(input_grid), gtk_label_new("Event Name:"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(input_grid), event_name_entry, 1, 0, 1, 1);
    
    gtk_grid_attach(GTK_GRID(input_grid), gtk_label_new("Event Date:"), 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(input_grid), event_date_entry, 1, 1, 1, 1);
    
    gtk_grid_attach(GTK_GRID(input_grid), gtk_label_new("Location:"), 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(input_grid), location_entry, 1, 2, 1, 1);
    
    gtk_grid_attach(GTK_GRID(input_grid), gtk_label_new("Category:"), 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(input_grid), category_combo, 1, 3, 1, 1);
    
    gtk_grid_attach(GTK_GRID(input_grid), gtk_label_new("Status:"), 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(input_grid), status_combo, 1, 4, 1, 1);
    
    gtk_grid_attach(GTK_GRID(input_grid), gtk_label_new("Attendees:"), 0, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(input_grid), attendees_spin, 1, 5, 1, 1);
    
    gtk_grid_attach(GTK_GRID(input_grid), gtk_label_new("Budget (₹):"), 0, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(input_grid), budget_spin, 1, 6, 1, 1);
    
    // Add grid to dialog
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), input_grid, TRUE, TRUE, 10);
    
    // Show all widgets in the dialog
    gtk_widget_show_all(dialog);
    
    // Run the dialog and handle the response
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_OK) {
        const gchar *event_name = gtk_entry_get_text(GTK_ENTRY(event_name_entry));
        const gchar *event_date = gtk_entry_get_text(GTK_ENTRY(event_date_entry));
        const gchar *location = gtk_entry_get_text(GTK_ENTRY(location_entry));
        gchar *category = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(category_combo));
        gchar *status = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(status_combo));
        gint attendees = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(attendees_spin));
        gdouble budget = gtk_spin_button_get_value(GTK_SPIN_BUTTON(budget_spin));
        
        // Validate inputs
        if (strlen(event_name) == 0) {
            GtkWidget *error_dialog = gtk_message_dialog_new(
                GTK_WINDOW(dialog),
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_ERROR,
                GTK_BUTTONS_OK,
                "Event name cannot be empty!"
            );
            gtk_dialog_run(GTK_DIALOG(error_dialog));
            gtk_widget_destroy(error_dialog);
            
            // Free dynamically allocated strings
            g_free(category);
            g_free(status);
            
            // Continue dialog
            gtk_widget_destroy(dialog);
            return;
        }
        
        if (!validate_date(event_date)) {
            GtkWidget *error_dialog = gtk_message_dialog_new(
                GTK_WINDOW(dialog),
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_ERROR,
                GTK_BUTTONS_OK,
                "Invalid date format! Use YYYY-MM-DD"
            );
            gtk_dialog_run(GTK_DIALOG(error_dialog));
            gtk_widget_destroy(error_dialog);
            
            // Free dynamically allocated strings
            g_free(category);
            g_free(status);
            
            // Continue dialog
            gtk_widget_destroy(dialog);
            return;
        }
        
        // Save event to file
        save_event_to_file(event_name, event_date, location, 
                            category, status, attendees, budget);
        
        // Print confirmation
        g_print("Event Added: %s on %s\n", event_name, event_date);
        trigger_notification("A New Event is Added!");
        
        // Free dynamically allocated strings
        g_free(category);
        g_free(status);
    }
    
    // Destroy the dialog
    gtk_widget_destroy(dialog);
}

// Function to read events from file
Event* read_events_from_file(int* num_events) {
    FILE *file = fopen("events.txt", "r");
    if (file == NULL) {
        g_print("Error: Unable to open events.txt for reading!\n");
        *num_events = 0;
        return NULL;
    }

    // Allocate initial space for events
    int max_events = 100;  // Initial allocation
    Event* events = malloc(max_events * sizeof(Event));
    *num_events = 0;

    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        // Trim newline
        line[strcspn(line, "\n")] = 0;

        if (strncmp(line, "Event ID: ", 10) == 0) {
            // Ensure we have space for a new event
            if (*num_events >= max_events) {
                max_events *= 2;
                events = realloc(events, max_events * sizeof(Event));
            }

            // Parse Event ID
            events[*num_events].event_id = atol(line + 10);

            // Read next 7 lines for other event details
            fgets(line, sizeof(line), file);  // Name
            sscanf(line, "Name: %[^\n]", events[*num_events].name);

            fgets(line, sizeof(line), file);  // Date
            sscanf(line, "Date: %[^\n]", events[*num_events].date);

            fgets(line, sizeof(line), file);  // Location
            sscanf(line, "Location: %[^\n]", events[*num_events].location);

            fgets(line, sizeof(line), file);  // Category
            sscanf(line, "Category: %[^\n]", events[*num_events].category);

            fgets(line, sizeof(line), file);  // Status
            sscanf(line, "Status: %[^\n]", events[*num_events].status);

            fgets(line, sizeof(line), file);  // Attendees
            sscanf(line, "Attendees: %d", &events[*num_events].attendees);

            fgets(line, sizeof(line), file);  // Budget
            sscanf(line, "Budget: %lf", &events[*num_events].budget);

            // Skip separator line
            fgets(line, sizeof(line), file);

            (*num_events)++;
        }
    }

    fclose(file);
    return events;
}

// Function to overwrite events file with updated events
void overwrite_events_file(Event* events, int num_events) {
    FILE *file = fopen("events.txt", "w");
    if (file == NULL) {
        g_print("Error: Unable to open events.txt for writing!\n");
        return;
    }

    for (int i = 0; i < num_events; i++) {
        fprintf(file, "Event ID: %ld\n", events[i].event_id);
        fprintf(file, "Name: %s\n", events[i].name);
        fprintf(file, "Date: %s\n", events[i].date);
        fprintf(file, "Location: %s\n", events[i].location);
        fprintf(file, "Category: %s\n", events[i].category);
        fprintf(file, "Status: %s\n", events[i].status);
        fprintf(file, "Attendees: %d\n", events[i].attendees);
        fprintf(file, "Budget: %.2f\n", events[i].budget);
        fprintf(file, "---\n");
    }

    fclose(file);
}

// Callback for edit event button
static void on_edit_event_button_clicked(GtkWidget *widget, gpointer data) {
    GtkWindow *parent_window = GTK_WINDOW(data);
    int num_events = 0;
    Event* events = read_events_from_file(&num_events);

    if (num_events == 0) {
        GtkWidget *error_dialog = gtk_message_dialog_new(
            parent_window,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_INFO,
            GTK_BUTTONS_OK,
            "No events found to edit!"
        );
        gtk_dialog_run(GTK_DIALOG(error_dialog));
        gtk_widget_destroy(error_dialog);
        free(events);
        return;
    }

    // Create event selection dialog with improved styling
    GtkWidget *select_dialog = gtk_dialog_new_with_buttons(
        "Select Event to Edit",
        parent_window,
        GTK_DIALOG_MODAL,
        "Edit", GTK_RESPONSE_OK,
        "Cancel", GTK_RESPONSE_CANCEL,
        NULL
    );
    
    // Increase dialog size for better readability
    gtk_window_set_default_size(GTK_WINDOW(select_dialog), 600, 400);

    // Create a scrolled window for events list
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), 
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    // Create a list store for events
    GtkListStore *event_store = gtk_list_store_new(4, 
        G_TYPE_LONG,    // Event ID
        G_TYPE_STRING,  // Event Name
        G_TYPE_STRING,  // Event Date
        G_TYPE_STRING   // Event Category
    );

    // Populate the list store
    GtkTreeIter iter;
    for (int i = 0; i < num_events; i++) {
        gtk_list_store_append(event_store, &iter);
        gtk_list_store_set(event_store, &iter, 
            0, events[i].event_id,
            1, events[i].name,
            2, events[i].date,
            3, events[i].category,
            -1
        );
    }

    // Create a tree view
    GtkWidget *event_tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(event_store));
    g_object_unref(event_store);

    // Add CSS styling for the tree view
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
        "treeview {"
        "   grid-line-color: #E0E0E0;"
        "}"
        "treeview:selected {"
        "   background-color: #E0E0E0;"
        "   color: #000000;"
        "}", -1, NULL);

    GdkScreen *screen = gdk_screen_get_default();
    gtk_style_context_add_provider_for_screen(screen, 
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);

    // Configure tree view
    gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(event_tree_view), 
                                 GTK_TREE_VIEW_GRID_LINES_VERTICAL);

    // Create columns
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    // Event ID Column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("ID", renderer, 
                                                      "text", 0, 
                                                      NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(event_tree_view), column);

    // Event Name Column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Event Name", renderer, 
                                                      "text", 1, 
                                                      NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(event_tree_view), column);

    // Event Date Column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Date", renderer, 
                                                      "text", 2, 
                                                      NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(event_tree_view), column);

    // Event Category Column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Category", renderer, 
                                                      "text", 3, 
                                                      NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(event_tree_view), column);

    // Enable selection
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(event_tree_view));
    gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);

    // Add tree view to scrolled window
    gtk_container_add(GTK_CONTAINER(scrolled_window), event_tree_view);

    // Add scrolled window to dialog
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(select_dialog));
    gtk_box_pack_start(GTK_BOX(content_area), scrolled_window, TRUE, TRUE, 10);

    // Show all widgets
    gtk_widget_show_all(select_dialog);

    // Run dialog and get selected event
    gint result = gtk_dialog_run(GTK_DIALOG(select_dialog));
    
    // Get the selected event
    GtkTreeModel *model;
    long selected_event_id = -1;
    if (result == GTK_RESPONSE_OK) {
        GtkTreeIter selected_iter;
        if (gtk_tree_selection_get_selected(selection, &model, &selected_iter)) {
            gtk_tree_model_get(model, &selected_iter, 0, &selected_event_id, -1);
        }
    }

    // Destroy dialog
    gtk_widget_destroy(select_dialog);

    // Find the index of the selected event
    int selected_index = -1;
    for (int i = 0; i < num_events; i++) {
        if (events[i].event_id == selected_event_id) {
            selected_index = i;
            break;
        }
    }

    // If no event selected or cancel pressed, return
    if (selected_index == -1) {
        free(events);
        return;
    }

    // Create edit dialog similar to add event dialog
    GtkWidget *edit_dialog = gtk_dialog_new_with_buttons(
        "Edit Event",
        parent_window,
        GTK_DIALOG_MODAL,
        "Save", GTK_RESPONSE_OK,
        "Cancel", GTK_RESPONSE_CANCEL,
        NULL
    );
    gtk_window_set_default_size(GTK_WINDOW(edit_dialog), 400, 500);

    // Create input grid
    GtkWidget *input_grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(input_grid), 10);
    gtk_grid_set_row_spacing(GTK_GRID(input_grid), 10);

    // Create input fields with current event data
    Event *current_event = &events[selected_index];

    GtkWidget *event_name_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(event_name_entry), current_event->name);

    GtkWidget *event_date_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(event_date_entry), current_event->date);

    GtkWidget *location_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(location_entry), current_event->location);

    // Category Combo Box
    GtkWidget *category_combo = gtk_combo_box_text_new();
    const char *categories[] = {"Conference", "Workshop", "Seminar", "Social", "Other"};
    for (int i = 0; i < G_N_ELEMENTS(categories); i++) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(category_combo), categories[i]);
        if (strcmp(current_event->category, categories[i]) == 0) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(category_combo), i);
        }
    }

    // Status Combo Box
    GtkWidget *status_combo = gtk_combo_box_text_new();
    const char *statuses[] = {"Planned", "Ongoing", "Completed", "Cancelled"};
    for (int i = 0; i < G_N_ELEMENTS(statuses); i++) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(status_combo), statuses[i]);
        if (strcmp(current_event->status, statuses[i]) == 0) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(status_combo), i);
        }
    }

    // Attendees Spin Button
    GtkWidget *attendees_spin = gtk_spin_button_new_with_range(0, 1000, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(attendees_spin), current_event->attendees);

    // Budget Spin Button
    GtkWidget *budget_spin = gtk_spin_button_new_with_range(0, 1000000, 100);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(budget_spin), current_event->budget);

    // Attach widgets to grid (similar to add event dialog)
    gtk_grid_attach(GTK_GRID(input_grid), gtk_label_new("Event Name:"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(input_grid), event_name_entry, 1, 0, 1, 1);
    
    gtk_grid_attach(GTK_GRID(input_grid), gtk_label_new("Event Date:"), 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(input_grid), event_date_entry, 1, 1, 1, 1);
    
    gtk_grid_attach(GTK_GRID(input_grid), gtk_label_new("Location:"), 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(input_grid), location_entry, 1, 2, 1, 1);
    
    gtk_grid_attach(GTK_GRID(input_grid), gtk_label_new("Category:"), 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(input_grid), category_combo, 1, 3, 1, 1);
    
    gtk_grid_attach(GTK_GRID(input_grid), gtk_label_new("Status:"), 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(input_grid), status_combo, 1, 4, 1, 1);
    
    gtk_grid_attach(GTK_GRID(input_grid), gtk_label_new("Attendees:"), 0, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(input_grid), attendees_spin, 1, 5, 1, 1);
    
    gtk_grid_attach(GTK_GRID(input_grid), gtk_label_new("Budget (₹):"), 0, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(input_grid), budget_spin, 1, 6, 1, 1);
    
    // Add grid to dialog
    GtkWidget *content_area_edit = gtk_dialog_get_content_area(GTK_DIALOG(edit_dialog));
    gtk_box_pack_start(GTK_BOX(content_area_edit), input_grid, TRUE, TRUE, 10);
    
    // Show all widgets in the dialog
    gtk_widget_show_all(edit_dialog);
    
    // Run the dialog and handle the response
    result = gtk_dialog_run(GTK_DIALOG(edit_dialog));
    if (result == GTK_RESPONSE_OK) {
        const gchar *event_name = gtk_entry_get_text(GTK_ENTRY(event_name_entry));
        const gchar *event_date = gtk_entry_get_text(GTK_ENTRY(event_date_entry));
        const gchar *location = gtk_entry_get_text(GTK_ENTRY(location_entry));
        gchar *category = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(category_combo));
        gchar *status = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(status_combo));
        gint attendees = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(attendees_spin));
        gdouble budget = gtk_spin_button_get_value(GTK_SPIN_BUTTON(budget_spin));
        
        // Validate inputs
        if (strlen(event_name) == 0) {
            GtkWidget *error_dialog = gtk_message_dialog_new(
                GTK_WINDOW(edit_dialog),
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_ERROR,
                GTK_BUTTONS_OK,
                "Event name cannot be empty!"
            );
            gtk_dialog_run(GTK_DIALOG(error_dialog));
            gtk_widget_destroy(error_dialog);
            
            // Free dynamically allocated strings
            g_free(category);
            g_free(status);
            
            // Cleanup
            gtk_widget_destroy(edit_dialog);
            free(events);
            return;
        }
        
        if (!validate_date(event_date)) {
            GtkWidget *error_dialog = gtk_message_dialog_new(
                GTK_WINDOW(edit_dialog),
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_ERROR,
                GTK_BUTTONS_OK,
                "Invalid date format! Use YYYY-MM-DD"
            );
            gtk_dialog_run(GTK_DIALOG(error_dialog));
            gtk_widget_destroy(error_dialog);
            
            // Free dynamically allocated strings
            g_free(category);
            g_free(status);
            
            // Cleanup
            gtk_widget_destroy(edit_dialog);
            free(events);
            return;
        }
        
        // Update the event in memory
        strcpy(events[selected_index].name, event_name);
        strcpy(events[selected_index].date, event_date);
        strcpy(events[selected_index].location, location);
        strcpy(events[selected_index].category, category);
        strcpy(events[selected_index].status, status);
        events[selected_index].attendees = attendees;
        events[selected_index].budget = budget;
        
        // Overwrite the events file with updated events
        overwrite_events_file(events, num_events);
        
        // Print confirmation
        g_print("Event Updated: %s on %s\n", event_name, event_date);
        trigger_notification("A Event was edited successfully!");
	// Free dynamically allocated strings
        g_free(category);
        g_free(status);
    }
    
    // Cleanup
    gtk_widget_destroy(edit_dialog);
    free(events);
}

//Callback for Delete event button
static void on_delete_event_button_clicked(GtkWidget *widget, gpointer data) {
    GtkWindow *parent_window = GTK_WINDOW(data);
    int num_events = 0;
    Event* events = read_events_from_file(&num_events);

    if (num_events == 0) {
        GtkWidget *error_dialog = gtk_message_dialog_new(
            parent_window,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_INFO,
            GTK_BUTTONS_OK,
            "No events found to delete!"
        );
        gtk_dialog_run(GTK_DIALOG(error_dialog));
        gtk_widget_destroy(error_dialog);
        free(events);
        return;
    }

    // Create option dialog for deletion type
    GtkWidget *option_dialog = gtk_dialog_new_with_buttons(
        "Delete Events",
        parent_window,
        GTK_DIALOG_MODAL,
        "Delete Single Event", GTK_RESPONSE_YES,
        "Delete All Events", GTK_RESPONSE_APPLY,
        "Cancel", GTK_RESPONSE_CANCEL,
        NULL
    );

    // Add descriptive label
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(option_dialog));
    GtkWidget *label = gtk_label_new("Please select a deletion option:");
    gtk_box_pack_start(GTK_BOX(content_area), label, TRUE, TRUE, 10);
    gtk_widget_show_all(option_dialog);

    // Run option dialog
    gint option_result = gtk_dialog_run(GTK_DIALOG(option_dialog));
    gtk_widget_destroy(option_dialog);

    if (option_result == GTK_RESPONSE_YES) {
        // Single event deletion logic
        GtkWidget *select_dialog = gtk_dialog_new_with_buttons(
            "Select Event to Delete",
            parent_window,
            GTK_DIALOG_MODAL,
            "Delete", GTK_RESPONSE_OK,
            "Cancel", GTK_RESPONSE_CANCEL,
            NULL
        );
        
        gtk_window_set_default_size(GTK_WINDOW(select_dialog), 600, 400);

        // Create and configure scrolled window
        GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), 
                                     GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

        // Create and populate list store
        GtkListStore *event_store = gtk_list_store_new(4, 
            G_TYPE_LONG,    // Event ID
            G_TYPE_STRING,  // Event Name
            G_TYPE_STRING,  // Event Date
            G_TYPE_STRING   // Event Category
        );

        GtkTreeIter iter;
        for (int i = 0; i < num_events; i++) {
            gtk_list_store_append(event_store, &iter);
            gtk_list_store_set(event_store, &iter, 
                0, events[i].event_id,
                1, events[i].name,
                2, events[i].date,
                3, events[i].category,
                -1
            );
        }

        // Create and configure tree view
        GtkWidget *event_tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(event_store));
        g_object_unref(event_store);

        // Apply CSS styling
        GtkCssProvider *provider = gtk_css_provider_new();
        gtk_css_provider_load_from_data(provider,
            "treeview {"
            "   grid-line-color: #E0E0E0;"
            "}"
            "treeview:selected {"
            "   background-color: #FFCCCB;"
            "   color: #000000;"
            "}", -1, NULL);

        GdkScreen *screen = gdk_screen_get_default();
        gtk_style_context_add_provider_for_screen(screen, 
            GTK_STYLE_PROVIDER(provider),
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        g_object_unref(provider);

        // Configure columns
        const char *column_titles[] = {"ID", "Event Name", "Date", "Category"};
        for (int i = 0; i < 4; i++) {
            GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
            GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(
                column_titles[i], renderer, "text", i, NULL);
            gtk_tree_view_append_column(GTK_TREE_VIEW(event_tree_view), column);
        }

        // Configure selection
        GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(event_tree_view));
        gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);

        // Add tree view to dialog
        gtk_container_add(GTK_CONTAINER(scrolled_window), event_tree_view);
        gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(select_dialog))), 
                          scrolled_window, TRUE, TRUE, 10);
        gtk_widget_show_all(select_dialog);

        // Handle single event deletion
        if (gtk_dialog_run(GTK_DIALOG(select_dialog)) == GTK_RESPONSE_OK) {
            GtkTreeIter selected_iter;
            GtkTreeModel *model;
            long selected_event_id = -1;

            if (gtk_tree_selection_get_selected(selection, &model, &selected_iter)) {
                gtk_tree_model_get(model, &selected_iter, 0, &selected_event_id, -1);
                
                // Find selected event
                int selected_index = -1;
                for (int i = 0; i < num_events; i++) {
                    if (events[i].event_id == selected_event_id) {
                        selected_index = i;
                        break;
                    }
                }

                if (selected_index != -1) {
                    // Confirmation dialog
                    GtkWidget *confirm_dialog = gtk_message_dialog_new(
                        parent_window,
                        GTK_DIALOG_MODAL,
                        GTK_MESSAGE_QUESTION,
                        GTK_BUTTONS_YES_NO,
                        "Are you sure you want to delete:\n\n"
                        "Event: %s\n"
                        "Date: %s\n"
                        "Category: %s",
                        events[selected_index].name,
                        events[selected_index].date,
                        events[selected_index].category
                    );

                    if (gtk_dialog_run(GTK_DIALOG(confirm_dialog)) == GTK_RESPONSE_YES) {
                        // Delete single event
                        Event *updated_events = malloc((num_events - 1) * sizeof(Event));
                        int j = 0;
                        for (int i = 0; i < num_events; i++) {
                            if (i != selected_index) {
                                updated_events[j++] = events[i];
                            }
                        }
                        overwrite_events_file(updated_events, num_events - 1);
                        trigger_notification("Event deleted successfully!");
                        free(updated_events);
                    }
                    gtk_widget_destroy(confirm_dialog);
                }
            }
        }
        gtk_widget_destroy(select_dialog);
    }
    else if (option_result == GTK_RESPONSE_APPLY) {
        // Delete all events confirmation
        GtkWidget *confirm_all_dialog = gtk_message_dialog_new(
            parent_window,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_WARNING,
            GTK_BUTTONS_YES_NO,
            "Warning: This will delete ALL events!\n\n"
            "Total events to be deleted: %d\n\n"
            "Are you sure you want to continue?",
            num_events
        );

        if (gtk_dialog_run(GTK_DIALOG(confirm_all_dialog)) == GTK_RESPONSE_YES) {
            // Delete all events
            overwrite_events_file(NULL, 0);
            trigger_notification("All events have been deleted!");
        }
        gtk_widget_destroy(confirm_all_dialog);
    }

    // Cleanup
    free(events);
}

//Callback for View Events Details Button
static void on_view_event_button_clicked(GtkWidget *widget, gpointer data) {
    GtkWindow *parent_window = GTK_WINDOW(data);
    int num_events = 0;
    Event* events = read_events_from_file(&num_events);

    if (num_events == 0) {
        GtkWidget *error_dialog = gtk_message_dialog_new(
            parent_window,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_INFO,
            GTK_BUTTONS_OK,
            "No events found to view!"
        );
        gtk_dialog_run(GTK_DIALOG(error_dialog));
        gtk_widget_destroy(error_dialog);
        free(events);
        return;
    }

    // Create event selection dialog for viewing details
    GtkWidget *select_dialog = gtk_dialog_new_with_buttons(
        "Select Event to View Details",
        parent_window,
        GTK_DIALOG_MODAL,
        "View Details", GTK_RESPONSE_OK,
        "Cancel", GTK_RESPONSE_CANCEL,
        NULL
    );
    
    // Increase dialog size for better readability
    gtk_window_set_default_size(GTK_WINDOW(select_dialog), 600, 400);

    // Create a scrolled window for events list
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), 
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    // Create a list store for events
    GtkListStore *event_store = gtk_list_store_new(4, 
        G_TYPE_LONG,    // Event ID
        G_TYPE_STRING,  // Event Name
        G_TYPE_STRING,  // Event Date
        G_TYPE_STRING   // Event Category
    );

    // Populate the list store
    GtkTreeIter iter;
    for (int i = 0; i < num_events; i++) {
        gtk_list_store_append(event_store, &iter);
        gtk_list_store_set(event_store, &iter, 
            0, events[i].event_id,
            1, events[i].name,
            2, events[i].date,
            3, events[i].category,
            -1
        );
    }

    // Create a tree view
    GtkWidget *event_tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(event_store));
    g_object_unref(event_store);

    // Add CSS styling for the tree view
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
        "treeview {"
        "   grid-line-color: #E0E0E0;"
        "}"
        "treeview:selected {"
        "   background-color: #E0F8FF;"  // Light blue for selection
        "   color: #000000;"
        "}", -1, NULL);

    GdkScreen *screen = gdk_screen_get_default();
    gtk_style_context_add_provider_for_screen(screen, 
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);

    // Configure tree view
    gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(event_tree_view), 
                                 GTK_TREE_VIEW_GRID_LINES_VERTICAL);

    // Create columns
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    // Event ID Column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("ID", renderer, 
                                                      "text", 0, 
                                                      NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(event_tree_view), column);

    // Event Name Column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Event Name", renderer, 
                                                      "text", 1, 
                                                      NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(event_tree_view), column);

    // Event Date Column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Date", renderer, 
                                                      "text", 2, 
                                                      NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(event_tree_view), column);

    // Event Category Column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Category", renderer, 
                                                      "text", 3, 
                                                      NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(event_tree_view), column);

    // Enable selection
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(event_tree_view));
    gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);

    // Add tree view to scrolled window
    gtk_container_add(GTK_CONTAINER(scrolled_window), event_tree_view);

    // Add scrolled window to dialog
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(select_dialog));
    gtk_box_pack_start(GTK_BOX(content_area), scrolled_window, TRUE, TRUE, 10);

    // Show all widgets
    gtk_widget_show_all(select_dialog);

    // Run dialog and get selected event
    gint result = gtk_dialog_run(GTK_DIALOG(select_dialog));
    
    // Get the selected event
    GtkTreeModel *model;
    long selected_event_id = -1;
    if (result == GTK_RESPONSE_OK) {
        GtkTreeIter selected_iter;
        if (gtk_tree_selection_get_selected(selection, &model, &selected_iter)) {
            gtk_tree_model_get(model, &selected_iter, 0, &selected_event_id, -1);
        }
    }

    // Destroy selection dialog
    gtk_widget_destroy(select_dialog);

    // Find the selected event
    Event *selected_event = NULL;
    for (int i = 0; i < num_events; i++) {
        if (events[i].event_id == selected_event_id) {
            selected_event = &events[i];
            break;
        }
    }

    // If no event selected or cancel pressed, return
    if (selected_event == NULL) {
        free(events);
        return;
    }

    // Create details dialog
    GtkWidget *details_dialog = gtk_dialog_new_with_buttons(
        "Event Details",
        parent_window,
        GTK_DIALOG_MODAL,
        "Close", GTK_RESPONSE_CLOSE,
        NULL
    );
    gtk_window_set_default_size(GTK_WINDOW(details_dialog), 400, 500);

    // Create a grid for details
    GtkWidget *details_grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(details_grid), 10);
    gtk_grid_set_row_spacing(GTK_GRID(details_grid), 10);

    // Create labels for each detail with a bold header style
    PangoAttrList *bold_attrs = pango_attr_list_new();
    PangoAttribute *bold = pango_attr_weight_new(PANGO_WEIGHT_BOLD);
    pango_attr_list_insert(bold_attrs, bold);

    // Create and style detail labels
    GtkWidget *detail_labels[] = {
        gtk_label_new("Event ID:"),
        gtk_label_new("Event Name:"),
        gtk_label_new("Date:"),
        gtk_label_new("Location:"),
        gtk_label_new("Category:"),
        gtk_label_new("Status:"),
        gtk_label_new("Attendees:"),
        gtk_label_new("Budget:")
    };

    // Set bold style for header labels
    for (int i = 0; i < G_N_ELEMENTS(detail_labels); i++) {
        gtk_label_set_attributes(GTK_LABEL(detail_labels[i]), bold_attrs);
        gtk_label_set_xalign(GTK_LABEL(detail_labels[i]), 1.0);  // Right align
    }

    // Create value labels
    char id_str[20], attendees_str[20], budget_str[20];
    sprintf(id_str, "%ld", selected_event->event_id);
    sprintf(attendees_str, "%d", selected_event->attendees);
    sprintf(budget_str, "₹%.2f", selected_event->budget);

    GtkWidget *value_labels[] = {
        gtk_label_new(id_str),
        gtk_label_new(selected_event->name),
        gtk_label_new(selected_event->date),
        gtk_label_new(selected_event->location),
        gtk_label_new(selected_event->category),
        gtk_label_new(selected_event->status),
        gtk_label_new(attendees_str),
        gtk_label_new(budget_str)
    };

    // Set left alignment for value labels
    for (int i = 0; i < G_N_ELEMENTS(value_labels); i++) {
        gtk_label_set_xalign(GTK_LABEL(value_labels[i]), 0.0);  // Left align
    }

    // Attach labels to grid
    for (int i = 0; i < G_N_ELEMENTS(detail_labels); i++) {
        gtk_grid_attach(GTK_GRID(details_grid), detail_labels[i], 0, i, 1, 1);
        gtk_grid_attach(GTK_GRID(details_grid), value_labels[i], 1, i, 1, 1);
    }

    // Add grid to dialog
    GtkWidget *content_area_details = gtk_dialog_get_content_area(GTK_DIALOG(details_dialog));
    gtk_box_pack_start(GTK_BOX(content_area_details), details_grid, TRUE, TRUE, 10);

    // Free bold attributes
    pango_attr_list_unref(bold_attrs);

    // Show all widgets
    gtk_widget_show_all(details_dialog);

    // Run the dialog
    gtk_dialog_run(GTK_DIALOG(details_dialog));

    // Cleanup
    gtk_widget_destroy(details_dialog);
    free(events);
}

// Helper function for case-insensitive string search
static char* stristr(const char* haystack, const char* needle) {
    if (!haystack || !needle) return NULL;

    char* haystack_lower = strdup(haystack);
    char* needle_lower = strdup(needle);
    
    for (int i = 0; haystack_lower[i]; i++) {
        haystack_lower[i] = tolower(haystack_lower[i]);
    }
    for (int i = 0; needle_lower[i]; i++) {
        needle_lower[i] = tolower(needle_lower[i]);
    }

    char* result = strstr(haystack_lower, needle_lower);
    char* final_result = result ? (char*)haystack + (result - haystack_lower) : NULL;
    
    free(haystack_lower);
    free(needle_lower);
    
    return final_result;
}


// Function to search events based on search criteria
static void on_search_event_button_clicked(GtkWidget *widget, gpointer data) {
    GtkWindow *parent_window = GTK_WINDOW(data);
    int num_events = 0;
    Event* events = read_events_from_file(&num_events);

    if (num_events == 0) {
        GtkWidget *error_dialog = gtk_message_dialog_new(
            parent_window,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_INFO,
            GTK_BUTTONS_OK,
            "No events found to search!"
        );
        gtk_dialog_run(GTK_DIALOG(error_dialog));
        gtk_widget_destroy(error_dialog);
        free(events);
        return;
    }

    // Create search dialog with improved styling
    GtkWidget *search_dialog = gtk_dialog_new_with_buttons(
        "Search Events",
        parent_window,
        GTK_DIALOG_MODAL,
        "Search", GTK_RESPONSE_OK,
        "Cancel", GTK_RESPONSE_CANCEL,
        NULL
    );
    gtk_window_set_default_size(GTK_WINDOW(search_dialog), 400, 250);

    // Add some padding to the dialog
    GtkWidget *search_content = gtk_dialog_get_content_area(GTK_DIALOG(search_dialog));
    gtk_container_set_border_width(GTK_CONTAINER(search_content), 20);

    // Create search grid with improved spacing
    GtkWidget *search_grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(search_grid), 15);
    gtk_grid_set_row_spacing(GTK_GRID(search_grid), 15);

    // Create search fields with improved styling
    GtkWidget *name_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(name_entry), "Search by name...");
    gtk_widget_set_size_request(name_entry, 200, -1);

    GtkWidget *location_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(location_entry), "Search by location...");
    gtk_widget_set_size_request(location_entry, 200, -1);

    GtkWidget *date_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(date_entry), "YYYY-MM-DD");
    gtk_widget_set_size_request(date_entry, 200, -1);

    // Category Combo Box
    GtkWidget *category_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(category_combo), "Any");
    const char *categories[] = {"Conference", "Workshop", "Seminar", "Social", "Other"};
    for (int i = 0; i < G_N_ELEMENTS(categories); i++) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(category_combo), categories[i]);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(category_combo), 0);

    // Status Combo Box
    GtkWidget *status_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(status_combo), "Any");
    const char *statuses[] = {"Planned", "Ongoing", "Completed", "Cancelled"};
    for (int i = 0; i < G_N_ELEMENTS(statuses); i++) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(status_combo), statuses[i]);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(status_combo), 0);

    // Create labels with better alignment
    GtkWidget *name_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(name_label), "<b>Name:</b>");
    gtk_widget_set_halign(name_label, GTK_ALIGN_START);

    GtkWidget *location_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(location_label), "<b>Location:</b>");
    gtk_widget_set_halign(location_label, GTK_ALIGN_START);

    GtkWidget *date_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(date_label), "<b>Date:</b>");
    gtk_widget_set_halign(date_label, GTK_ALIGN_START);

    GtkWidget *category_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(category_label), "<b>Category:</b>");
    gtk_widget_set_halign(category_label, GTK_ALIGN_START);

    GtkWidget *status_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(status_label), "<b>Status:</b>");
    gtk_widget_set_halign(status_label, GTK_ALIGN_START);

    // Attach widgets to grid
    gtk_grid_attach(GTK_GRID(search_grid), name_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(search_grid), name_entry, 1, 0, 1, 1);

    gtk_grid_attach(GTK_GRID(search_grid), location_label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(search_grid), location_entry, 1, 1, 1, 1);

    gtk_grid_attach(GTK_GRID(search_grid), date_label, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(search_grid), date_entry, 1, 2, 1, 1);

    gtk_grid_attach(GTK_GRID(search_grid), category_label, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(search_grid), category_combo, 1, 3, 1, 1);

    gtk_grid_attach(GTK_GRID(search_grid), status_label, 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(search_grid), status_combo, 1, 4, 1, 1);

    // Add grid to dialog
    gtk_box_pack_start(GTK_BOX(search_content), search_grid, TRUE, TRUE, 0);

    // Show all widgets
    gtk_widget_show_all(search_dialog);

    // Run dialog and handle response
    gint result = gtk_dialog_run(GTK_DIALOG(search_dialog));
    if (result == GTK_RESPONSE_OK) {
        const gchar *search_name = gtk_entry_get_text(GTK_ENTRY(name_entry));
        const gchar *search_location = gtk_entry_get_text(GTK_ENTRY(location_entry));
        const gchar *search_date = gtk_entry_get_text(GTK_ENTRY(date_entry));
        gchar *search_category = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(category_combo));
        gchar *search_status = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(status_combo));

        // Create results window with improved styling
        GtkWidget *results_dialog = gtk_dialog_new_with_buttons(
            "Search Results",
            parent_window,
            GTK_DIALOG_MODAL,
            "Close", GTK_RESPONSE_CLOSE,
            NULL
        );
        gtk_window_set_default_size(GTK_WINDOW(results_dialog), 600, 400);

        // Create scrolled window with improved styling
        GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                     GTK_POLICY_AUTOMATIC,
                                     GTK_POLICY_AUTOMATIC);
        gtk_container_set_border_width(GTK_CONTAINER(scrolled_window), 10);

        // Create text view with improved styling
        GtkWidget *text_view = gtk_text_view_new();
        gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
        gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
        gtk_text_view_set_left_margin(GTK_TEXT_VIEW(text_view), 10);
        gtk_text_view_set_right_margin(GTK_TEXT_VIEW(text_view), 10);

        // Add CSS styling
        GtkCssProvider *provider = gtk_css_provider_new();
        gtk_css_provider_load_from_data(provider,
            "textview {"
            "   font-family: 'Sans';"
            "   font-size: 11pt;"
            "   background-color: #ffffff;"
            "}"
            "textview text {"
            "   color: #2c3e50;"
            "}", -1, NULL);
        
        GtkStyleContext *context = gtk_widget_get_style_context(text_view);
        gtk_style_context_add_provider(context,
            GTK_STYLE_PROVIDER(provider),
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        g_object_unref(provider);

        gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);

        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
        GtkTextIter iter;
        gtk_text_buffer_get_start_iter(buffer, &iter);

        // Add header with search criteria
        gchar *header = g_strdup_printf(
            "Search Results\n"
            "----------------------------------------\n"
            "Filters applied: %s%s%s%s%s\n\n",
            strlen(search_name) > 0 ? g_strdup_printf("Name=\"%s\" ", search_name) : "",
            strlen(search_location) > 0 ? g_strdup_printf("Location=\"%s\" ", search_location) : "",
            strlen(search_date) > 0 ? g_strdup_printf("Date=\"%s\" ", search_date) : "",
            strcmp(search_category, "Any") != 0 ? g_strdup_printf("Category=\"%s\" ", search_category) : "",
            strcmp(search_status, "Any") != 0 ? g_strdup_printf("Status=\"%s\" ", search_status) : ""
        );
        gtk_text_buffer_insert(buffer, &iter, header, -1);
        g_free(header);

        int matches_found = 0;

        // Search through events
        for (int i = 0; i < num_events; i++) {
            gboolean match = TRUE;

            if (strlen(search_name) > 0) {
                if (stristr(events[i].name, search_name) == NULL) {
                    match = FALSE;
                }
            }

            if (strlen(search_location) > 0) {
                if (stristr(events[i].location, search_location) == NULL) {
                    match = FALSE;
                }
            }

            if (strlen(search_date) > 0) {
                if (strcmp(events[i].date, search_date) != 0) {
                    match = FALSE;
                }
            }

            if (strcmp(search_category, "Any") != 0) {
                if (strcmp(events[i].category, search_category) != 0) {
                    match = FALSE;
                }
            }

            if (strcmp(search_status, "Any") != 0) {
                if (strcmp(events[i].status, search_status) != 0) {
                    match = FALSE;
                }
            }

            if (match) {
                matches_found++;
                // Format each event with better spacing and organization
                gchar *event_text = g_strdup_printf(
                    "Event #%d:\n"
                    "----------------------------------------\n"
                    "  • ID: %ld\n"
                    "  • Name: %s\n"
                    "  • Date: %s\n"
                    "  • Location: %s\n"
                    "  • Category: %s\n"
                    "  • Status: %s\n"
                    "  • Attendees: %d\n"
                    "  • Budget: ₹%.2f\n"
                    "----------------------------------------\n\n",
                    matches_found,
                    events[i].event_id,
                    events[i].name,
                    events[i].date,
                    events[i].location,
                    events[i].category,
                    events[i].status,
                    events[i].attendees,
                    events[i].budget
                );
                gtk_text_buffer_insert(buffer, &iter, event_text, -1);
                g_free(event_text);
            }
        }

        // Show summary at the end
        gchar *summary = g_strdup_printf(
            "Total matches found: %d\n",
            matches_found
        );
        gtk_text_buffer_insert(buffer, &iter, summary, -1);
        g_free(summary);

        // Add scrolled window to dialog
        GtkWidget *results_content_area = gtk_dialog_get_content_area(GTK_DIALOG(results_dialog));
        gtk_box_pack_start(GTK_BOX(results_content_area), scrolled_window, TRUE, TRUE, 0);

        // Show all widgets and run dialog
        gtk_widget_show_all(results_dialog);
        gtk_dialog_run(GTK_DIALOG(results_dialog));
        gtk_widget_destroy(results_dialog);

        // Free allocated strings
        g_free(search_category);
        g_free(search_status);
    }

    // Cleanup
    gtk_widget_destroy(search_dialog);
    free(events);
}

//Callback for Sort Events Button
static void on_sort_event_button_clicked(GtkWidget *widget, gpointer data) {
    GtkWindow *parent_window = GTK_WINDOW(data);
    int num_events = 0;
    Event* events = read_events_from_file(&num_events);

    if (num_events == 0) {
        GtkWidget *error_dialog = gtk_message_dialog_new(
            parent_window,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_INFO,
            GTK_BUTTONS_OK,
            "No events found to sort!"
        );
        gtk_dialog_run(GTK_DIALOG(error_dialog));
        gtk_widget_destroy(error_dialog);
        free(events);
        return;
    }

    // Create sort selection dialog with improved styling
    GtkWidget *sort_dialog = gtk_dialog_new_with_buttons(
        "Sort Events",
        parent_window,
        GTK_DIALOG_MODAL,
        "Sort", GTK_RESPONSE_OK,
        "Cancel", GTK_RESPONSE_CANCEL,
        NULL
    );
    gtk_window_set_default_size(GTK_WINDOW(sort_dialog), 350, 250);

    // Create a styled container
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(sort_dialog));
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_container_set_border_width(GTK_CONTAINER(main_box), 20);

    // Create a frame for sort options
    GtkWidget *options_frame = gtk_frame_new("Sort Options");
    GtkWidget *frame_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(frame_box), 15);

    // Sort criteria combo box with label
    GtkWidget *sort_criteria_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *sort_criteria_label = gtk_label_new("Sort By:");
    GtkWidget *sort_criteria_combo = gtk_combo_box_text_new();
    const char *criteria[] = {
        "Event Date", 
        "Event Name", 
        "Category", 
        "Budget", 
        "Attendees"
    };
    for (int i = 0; i < G_N_ELEMENTS(criteria); i++) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(sort_criteria_combo), criteria[i]);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(sort_criteria_combo), 0);  // Default to Event Date
    gtk_box_pack_start(GTK_BOX(sort_criteria_box), sort_criteria_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(sort_criteria_box), sort_criteria_combo, TRUE, TRUE, 0);

    // Sort order combo box with label
    GtkWidget *sort_order_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *sort_order_label = gtk_label_new("Order:");
    GtkWidget *sort_order_combo = gtk_combo_box_text_new();
    const char *orders[] = {"Ascending", "Descending"};
    for (int i = 0; i < G_N_ELEMENTS(orders); i++) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(sort_order_combo), orders[i]);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(sort_order_combo), 0);  // Default to Ascending
    gtk_box_pack_start(GTK_BOX(sort_order_box), sort_order_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(sort_order_box), sort_order_combo, TRUE, TRUE, 0);

    // Add to frame
    gtk_box_pack_start(GTK_BOX(frame_box), sort_criteria_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(frame_box), sort_order_box, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(options_frame), frame_box);

    // Add frame to main box
    gtk_box_pack_start(GTK_BOX(main_box), options_frame, FALSE, FALSE, 0);

    // Add main box to content area
    gtk_box_pack_start(GTK_BOX(content_area), main_box, TRUE, TRUE, 0);

    // Show all widgets
    gtk_widget_show_all(sort_dialog);

    // Comparison functions (same as previous implementation)
    int compare_events_by_date(const void *a, const void *b) {
        Event *event1 = (Event *)a;
        Event *event2 = (Event *)b;
        return strcmp(event1->date, event2->date);
    }

    int compare_events_by_name(const void *a, const void *b) {
        Event *event1 = (Event *)a;
        Event *event2 = (Event *)b;
        return strcmp(event1->name, event2->name);
    }

    int compare_events_by_category(const void *a, const void *b) {
        Event *event1 = (Event *)a;
        Event *event2 = (Event *)b;
        return strcmp(event1->category, event2->category);
    }

    int compare_events_by_budget(const void *a, const void *b) {
        Event *event1 = (Event *)a;
        Event *event2 = (Event *)b;
        return (event1->budget > event2->budget) - (event1->budget < event2->budget);
    }

    int compare_events_by_attendees(const void *a, const void *b) {
        Event *event1 = (Event *)a;
        Event *event2 = (Event *)b;
        return event1->attendees - event2->attendees;
    }

    // Run the dialog
    gint result = gtk_dialog_run(GTK_DIALOG(sort_dialog));
    
    if (result == GTK_RESPONSE_OK) {
        // Safely get selected values
        gchar *sort_criteria = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(sort_criteria_combo));
        gchar *sort_order = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(sort_order_combo));
        
        if (!sort_criteria || !sort_order) {
            // Fallback to default if something goes wrong
            sort_criteria = g_strdup("Event Date");
            sort_order = g_strdup("Ascending");
        }

        // Determine comparison function and order
        int (*compare_func)(const void *, const void *) = compare_events_by_date;
        int order_multiplier = 1;

        if (g_strcmp0(sort_criteria, "Event Name") == 0) {
            compare_func = compare_events_by_name;
        } else if (g_strcmp0(sort_criteria, "Category") == 0) {
            compare_func = compare_events_by_category;
        } else if (g_strcmp0(sort_criteria, "Budget") == 0) {
            compare_func = compare_events_by_budget;
        } else if (g_strcmp0(sort_criteria, "Attendees") == 0) {
            compare_func = compare_events_by_attendees;
        }

        // Adjust order
        if (g_strcmp0(sort_order, "Descending") == 0) {
            order_multiplier = -1;
        }

        // Custom comparison wrapper that respects order
        int compare_wrapper(const void *a, const void *b) {
            return order_multiplier * compare_func(a, b);
        }

        // Sort the events
        qsort(events, num_events, sizeof(Event), compare_wrapper);

        // Overwrite the events file with sorted events
        overwrite_events_file(events, num_events);

        // Create sorted events dialog
        GtkWidget *sorted_dialog = gtk_dialog_new_with_buttons(
            "Sorted Events",
            parent_window,
            GTK_DIALOG_MODAL,
            "Close", GTK_RESPONSE_CLOSE,
            NULL
        );
        gtk_window_set_default_size(GTK_WINDOW(sorted_dialog), 600, 400);
        trigger_notification("Events Sorted Successfully!");

        // Create a scrolled window for events list
        GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), 
                                       GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

        // Create a list store for events
        GtkListStore *event_store = gtk_list_store_new(4, 
            G_TYPE_LONG,    // Event ID
            G_TYPE_STRING,  // Event Name
            G_TYPE_STRING,  // Event Date
            G_TYPE_STRING   // Event Category
        );

        // Populate the list store with sorted events
        GtkTreeIter iter;
        for (int i = 0; i < num_events; i++) {
            gtk_list_store_append(event_store, &iter);
            gtk_list_store_set(event_store, &iter, 
                0, events[i].event_id,
                1, events[i].name,
                2, events[i].date,
                3, events[i].category,
                -1
            );
        }

        // Create a tree view
        GtkWidget *event_tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(event_store));
        g_object_unref(event_store);

        // Add CSS styling for the tree view
        GtkCssProvider *provider = gtk_css_provider_new();
        gtk_css_provider_load_from_data(provider,
            "treeview {"
            "   grid-line-color: #E0E0E0;"
            "}"
            "treeview:selected {"
            "   background-color: #E0F8E0;"  // Light green for selection
            "   color: #000000;"  // Black text for readability
            "}", -1, NULL);

        GdkScreen *screen = gdk_screen_get_default();
        gtk_style_context_add_provider_for_screen(screen, 
            GTK_STYLE_PROVIDER(provider),
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        g_object_unref(provider);

        // Create columns (similar to previous implementation)
        GtkCellRenderer *renderer;
        GtkTreeViewColumn *column;

        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("ID", renderer, "text", 0, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(event_tree_view), column);

        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Event Name", renderer, "text", 1, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(event_tree_view), column);

        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Date", renderer, "text", 2, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(event_tree_view), column);

        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Category", renderer, "text", 3, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(event_tree_view), column);

        // Add tree view to scrolled window
        gtk_container_add(GTK_CONTAINER(scrolled_window), event_tree_view);

        // Add a label to show sorting details
        GtkWidget *content_area_sorted = gtk_dialog_get_content_area(GTK_DIALOG(sorted_dialog));
        char sort_label_text[100];
        snprintf(sort_label_text, sizeof(sort_label_text), 
                 "Sorted by %s (%s order)", sort_criteria, sort_order);
        GtkWidget *sort_label = gtk_label_new(sort_label_text);
        
        // Create a vertical box to hold label and scrolled window
        GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
        gtk_box_pack_start(GTK_BOX(vbox), sort_label, FALSE, FALSE, 10);
        gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);
        
        // Add to dialog content area
        gtk_box_pack_start(GTK_BOX(content_area_sorted), vbox, TRUE, TRUE, 0);

        // Show all widgets
        gtk_widget_show_all(sorted_dialog);

        // Run the sorted events dialog
        gtk_dialog_run(GTK_DIALOG(sorted_dialog));

        // Cleanup
        gtk_widget_destroy(sorted_dialog);
        g_free(sort_criteria);
        g_free(sort_order);
    }

    // Cleanup
    gtk_widget_destroy(sort_dialog);
    free(events);
}

//Callback for Filter Events Button

static void on_filter_event_button_clicked(GtkWidget *widget, gpointer data) {
    GtkWindow *parent_window = GTK_WINDOW(data);
    int num_events = 0;
    Event* events = read_events_from_file(&num_events);

    if (num_events == 0) {
        GtkWidget *error_dialog = gtk_message_dialog_new(
            parent_window,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_INFO,
            GTK_BUTTONS_OK,
            "No events found to filter!"
        );
        gtk_dialog_run(GTK_DIALOG(error_dialog));
        gtk_widget_destroy(error_dialog);
        free(events);
        return;
    }

    // Create filter selection dialog
    GtkWidget *filter_dialog = gtk_dialog_new_with_buttons(
        "Filter Events",
        parent_window,
        GTK_DIALOG_MODAL,
        "Filter", GTK_RESPONSE_OK,
        "Cancel", GTK_RESPONSE_CANCEL,
        NULL
    );
    gtk_window_set_default_size(GTK_WINDOW(filter_dialog), 400, 300);

    // Main container
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(filter_dialog));
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_container_set_border_width(GTK_CONTAINER(main_box), 20);

    // Filter criteria frames
    GtkWidget *category_frame = gtk_frame_new("Category Filter");
    GtkWidget *status_frame = gtk_frame_new("Status Filter");
    GtkWidget *date_frame = gtk_frame_new("Date Range Filter");

    // Category filter
    GtkWidget *category_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(category_box), 10);
    const char *categories[] = {"All", "Conference", "Workshop", "Seminar", "Social", "Other"};
    GSList *category_group = NULL;
    GtkWidget *category_radios[G_N_ELEMENTS(categories)];

    for (int i = 0; i < G_N_ELEMENTS(categories); i++) {
        category_radios[i] = gtk_radio_button_new_with_label(category_group, categories[i]);
        category_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(category_radios[i]));
        gtk_box_pack_start(GTK_BOX(category_box), category_radios[i], FALSE, FALSE, 0);
    }
    gtk_container_add(GTK_CONTAINER(category_frame), category_box);

    // Status filter
    GtkWidget *status_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(status_box), 10);
    const char *statuses[] = {"All", "Planned", "Ongoing", "Completed", "Cancelled"};
    GSList *status_group = NULL;
    GtkWidget *status_radios[G_N_ELEMENTS(statuses)];

    for (int i = 0; i < G_N_ELEMENTS(statuses); i++) {
        status_radios[i] = gtk_radio_button_new_with_label(status_group, statuses[i]);
        status_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(status_radios[i]));
        gtk_box_pack_start(GTK_BOX(status_box), status_radios[i], FALSE, FALSE, 0);
    }
    gtk_container_add(GTK_CONTAINER(status_frame), status_box);

    // Date range filter
    GtkWidget *date_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(date_box), 10);

    GtkWidget *start_date_label = gtk_label_new("Start Date (YYYY-MM-DD):");
    GtkWidget *start_date_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(start_date_entry), "Optional");

    GtkWidget *end_date_label = gtk_label_new("End Date (YYYY-MM-DD):");
    GtkWidget *end_date_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(end_date_entry), "Optional");

    gtk_box_pack_start(GTK_BOX(date_box), start_date_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(date_box), start_date_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(date_box), end_date_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(date_box), end_date_entry, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(date_frame), date_box);

    // Add frames to main box
    gtk_box_pack_start(GTK_BOX(main_box), category_frame, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), status_frame, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), date_frame, FALSE, FALSE, 0);

    // Add main box to content area
    gtk_box_pack_start(GTK_BOX(content_area), main_box, TRUE, TRUE, 0);

    // Show all widgets
    gtk_widget_show_all(filter_dialog);

    // Run the dialog
    gint result = gtk_dialog_run(GTK_DIALOG(filter_dialog));
    
    if (result == GTK_RESPONSE_OK) {
        // Determine selected filters
        char *selected_category = NULL;
        char *selected_status = NULL;
        char *start_date = NULL;
        char *end_date = NULL;

        // Get category
        for (int i = 0; i < G_N_ELEMENTS(categories); i++) {
            if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(category_radios[i]))) {
                selected_category = (char*)categories[i];
                break;
            }
        }

        // Get status
        for (int i = 0; i < G_N_ELEMENTS(statuses); i++) {
            if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(status_radios[i]))) {
                selected_status = (char*)statuses[i];
                break;
            }
        }

        // Get date range
        start_date = (char*)gtk_entry_get_text(GTK_ENTRY(start_date_entry));
        end_date = (char*)gtk_entry_get_text(GTK_ENTRY(end_date_entry));

        // Validate date inputs if provided
        if ((strlen(start_date) > 0 && !validate_date(start_date)) || 
            (strlen(end_date) > 0 && !validate_date(end_date))) {
            GtkWidget *error_dialog = gtk_message_dialog_new(
                GTK_WINDOW(filter_dialog),
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_ERROR,
                GTK_BUTTONS_OK,
                "Invalid date format! Use YYYY-MM-DD"
            );
            gtk_dialog_run(GTK_DIALOG(error_dialog));
            gtk_widget_destroy(error_dialog);
            gtk_widget_destroy(filter_dialog);
            free(events);
            return;
        }

        // Filter events
        Event* filtered_events = malloc(num_events * sizeof(Event));
        int filtered_count = 0;

        for (int i = 0; i < num_events; i++) {
            // Category filter
            if (strcmp(selected_category, "All") != 0 && 
                strcmp(events[i].category, selected_category) != 0) {
                continue;
            }

            // Status filter
            if (strcmp(selected_status, "All") != 0 && 
                strcmp(events[i].status, selected_status) != 0) {
                continue;
            }

            // Date range filter
            if (strlen(start_date) > 0 && strcmp(events[i].date, start_date) < 0) {
                continue;
            }
            if (strlen(end_date) > 0 && strcmp(events[i].date, end_date) > 0) {
                continue;
            }

            // If event passes all filters, add to filtered list
            filtered_events[filtered_count++] = events[i];
        }

        // Create filtered events dialog
        GtkWidget *filtered_dialog = gtk_dialog_new_with_buttons(
            "Filtered Events",
            parent_window,
            GTK_DIALOG_MODAL,
            "Close", GTK_RESPONSE_CLOSE,
            NULL
        );
        gtk_window_set_default_size(GTK_WINDOW(filtered_dialog), 600, 400);

        // Create a scrolled window for events list
        GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), 
                                       GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

        // Create a list store for filtered events
        GtkListStore *event_store = gtk_list_store_new(4, 
            G_TYPE_LONG,    // Event ID
            G_TYPE_STRING,  // Event Name
            G_TYPE_STRING,  // Event Date
            G_TYPE_STRING   // Event Category
        );

        // Populate the list store with filtered events
        GtkTreeIter iter;
        for (int i = 0; i < filtered_count; i++) {
            gtk_list_store_append(event_store, &iter);
            gtk_list_store_set(event_store, &iter, 
                0, filtered_events[i].event_id,
                1, filtered_events[i].name,
                2, filtered_events[i].date,
                3, filtered_events[i].category,
                -1
            );
        }

        // Create a tree view
        GtkWidget *event_tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(event_store));
        g_object_unref(event_store);

        // Add CSS styling for the tree view
        GtkCssProvider *provider = gtk_css_provider_new();
        gtk_css_provider_load_from_data(provider,
            "treeview {"
            "   grid-line-color: #E0E0E0;"
            "}"
            "treeview:selected {"
            "   background-color: #E0F8E0;"  // Light green for selection
            "   color: #000000;"  // Black text for readability
            "}", -1, NULL);

        GdkScreen *screen = gdk_screen_get_default();
        gtk_style_context_add_provider_for_screen(screen, 
            GTK_STYLE_PROVIDER(provider),
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        g_object_unref(provider);

        // Create columns
        GtkCellRenderer *renderer;
        GtkTreeViewColumn *column;

        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("ID", renderer, "text", 0, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(event_tree_view), column);

        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Event Name", renderer, "text", 1, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(event_tree_view), column);

        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Date", renderer, "text", 2, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(event_tree_view), column);

        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Category", renderer, "text", 3, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(event_tree_view), column);

        // Add tree view to scrolled window
        gtk_container_add(GTK_CONTAINER(scrolled_window), event_tree_view);

        // Create filter summary label
        char filter_summary[300];
        snprintf(filter_summary, sizeof(filter_summary), 
                 "Filtered Results: %d events\nCategory: %s, Status: %s, Date Range: %s to %s", 
                 filtered_count, 
                 selected_category, 
                 selected_status,
                 strlen(start_date) > 0 ? start_date : "Any",
                 strlen(end_date) > 0 ? end_date : "Any");

        GtkWidget *summary_label = gtk_label_new(filter_summary);

        // Create a vertical box to hold label and scrolled window
        GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
        gtk_box_pack_start(GTK_BOX(vbox), summary_label, FALSE, FALSE, 10);
        gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);

        // Add to dialog content area
        GtkWidget *content_area_filtered = gtk_dialog_get_content_area(GTK_DIALOG(filtered_dialog));
        gtk_box_pack_start(GTK_BOX(content_area_filtered), vbox, TRUE, TRUE, 0);

        // Show all widgets
        gtk_widget_show_all(filtered_dialog);

        // Run the filtered events dialog
        gtk_dialog_run(GTK_DIALOG(filtered_dialog));

        // Cleanup
        gtk_widget_destroy(filtered_dialog);
        free(filtered_events);
    }

    // Cleanup
    gtk_widget_destroy(filter_dialog);
    free(events);
}

//Callback for Export Events Button
static void on_export_event_button_clicked(GtkWidget *widget, gpointer data) {
    GtkWindow *parent_window = GTK_WINDOW(data);
    int num_events = 0;
    Event* events = read_events_from_file(&num_events);

    if (num_events == 0) {
        GtkWidget *error_dialog = gtk_message_dialog_new(
            parent_window,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_INFO,
            GTK_BUTTONS_OK,
            "No events found to export!"
        );
        gtk_dialog_run(GTK_DIALOG(error_dialog));
        gtk_widget_destroy(error_dialog);
        free(events);
        return;
    }

    // Create export selection dialog
    GtkWidget *export_dialog = gtk_dialog_new_with_buttons(
        "Export Events",
        parent_window,
        GTK_DIALOG_MODAL,
        "Export", GTK_RESPONSE_OK,
        "Cancel", GTK_RESPONSE_CANCEL,
        NULL
    );
    gtk_window_set_default_size(GTK_WINDOW(export_dialog), 350, 250);

    // Create a styled container
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(export_dialog));
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_container_set_border_width(GTK_CONTAINER(main_box), 20);

    // Create a frame for export options
    GtkWidget *options_frame = gtk_frame_new("Export Options");
    GtkWidget *frame_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(frame_box), 15);

    // Export format combo box with label
    GtkWidget *export_format_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *export_format_label = gtk_label_new("Export Format:");
    GtkWidget *export_format_combo = gtk_combo_box_text_new();
    const char *formats[] = {
        "CSV (Spreadsheet)", 
        "PDF (Portable Document)"
    };
    for (int i = 0; i < G_N_ELEMENTS(formats); i++) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(export_format_combo), formats[i]);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(export_format_combo), 0);  // Default to CSV
    gtk_box_pack_start(GTK_BOX(export_format_box), export_format_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(export_format_box), export_format_combo, TRUE, TRUE, 0);

    // File name input
    GtkWidget *filename_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *filename_label = gtk_label_new("File Name:");
    GtkWidget *filename_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(filename_entry), "events_export");
    gtk_box_pack_start(GTK_BOX(filename_box), filename_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(filename_box), filename_entry, TRUE, TRUE, 0);

    // Add to frame
    gtk_box_pack_start(GTK_BOX(frame_box), export_format_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(frame_box), filename_box, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(options_frame), frame_box);

    // Add frame to main box
    gtk_box_pack_start(GTK_BOX(main_box), options_frame, FALSE, FALSE, 0);

    // Add main box to content area
    gtk_box_pack_start(GTK_BOX(content_area), main_box, TRUE, TRUE, 0);

    // Show all widgets
    gtk_widget_show_all(export_dialog);

    // Run the dialog
    gint result = gtk_dialog_run(GTK_DIALOG(export_dialog));
    
    if (result == GTK_RESPONSE_OK) {
        // Get selected format and filename
        gchar *export_format = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(export_format_combo));
        const gchar *filename_input = gtk_entry_get_text(GTK_ENTRY(filename_entry));
        
        // Generate filename with timestamp if not provided
        gchar *filename;
        if (filename_input && strlen(filename_input) > 0) {
            filename = g_strdup(filename_input);
        } else {
            time_t now = time(NULL);
            struct tm *t = localtime(&now);
            filename = g_malloc(100);
            strftime(filename, 100, "events_export_%Y%m%d_%H%M%S", t);
        }

        // Prepare full filepath
        gchar *filepath;
        if (g_strcmp0(export_format, "CSV (Spreadsheet)") == 0) {
            // Export to CSV
            filepath = g_strdup_printf("%s.csv", filename);
            FILE *csv_file = fopen(filepath, "w");
            
            if (csv_file) {
                // Write CSV header
                fprintf(csv_file, "Event ID,Name,Date,Location,Category,Status,Attendees,Budget\n");
                
                // Write event data
                for (int i = 0; i < num_events; i++) {
                    fprintf(csv_file, "%ld,\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%d,%.2f\n",
                            events[i].event_id, 
                            events[i].name, 
                            events[i].date,  // Ensure this is in a format Excel recognizes
                            events[i].location, 
                            events[i].category, 
                            events[i].status, 
                            events[i].attendees, 
                            events[i].budget);
                }
                
                fclose(csv_file);
                
                // Success dialog
                GtkWidget *success_dialog = gtk_message_dialog_new(
                    parent_window,
                    GTK_DIALOG_MODAL,
                    GTK_MESSAGE_INFO,
                    GTK_BUTTONS_OK,
                    "Events exported successfully to %s!", filepath
                );
                gtk_dialog_run(GTK_DIALOG(success_dialog));
                gtk_widget_destroy(success_dialog);
                trigger_notification("Events Exported Successfully!");
            } else {
                // Error dialog if file can't be created
                GtkWidget *error_dialog = gtk_message_dialog_new(
                    parent_window,
                    GTK_DIALOG_MODAL,
                    GTK_MESSAGE_ERROR,
                    GTK_BUTTONS_OK,
                    "Failed to create export file!"
                );
                gtk_dialog_run(GTK_DIALOG(error_dialog));
                gtk_widget_destroy(error_dialog);
            }
        } else if (g_strcmp0(export_format, "PDF (Portable Document)") == 0) {
            // Export to PDF (using basic text-based PDF generation)
            filepath = g_strdup_printf("%s.pdf", filename);
            FILE *pdf_file = fopen(filepath, "w");
            
            if (pdf_file) {
                // Simple PDF header and metadata
                fprintf(pdf_file, "%%PDF-1.7\n");
                fprintf(pdf_file, "1 0 obj\n");
                fprintf(pdf_file, "<<\n");
                fprintf(pdf_file, "/Type /Catalog\n");
                fprintf(pdf_file, "/Pages 2 0 R\n");
                fprintf(pdf_file, ">>\n");
                fprintf(pdf_file, "endobj\n");
                
                // Page object
                fprintf(pdf_file, "2 0 obj\n");
                fprintf(pdf_file, "<<\n");
                fprintf(pdf_file, "/Type /Page\n");
                fprintf(pdf_file, "/Parent 3 0 R\n");
                fprintf(pdf_file, "/Resources <<>>\n");
                fprintf(pdf_file, "/MediaBox [0 0 595 842]\n");
                fprintf(pdf_file, "/Contents 4 0 R\n");
                fprintf(pdf_file, ">>\n");
                fprintf(pdf_file, "endobj\n");
                
                // Content stream
                fprintf(pdf_file, "4 0 obj\n");
                fprintf(pdf_file, "<<\n");
                fprintf(pdf_file, "/Length 1000\n");
                fprintf(pdf_file, ">>\n");
                fprintf(pdf_file, "stream\n");
                fprintf(pdf_file, "BT\n");
                fprintf(pdf_file, "/F1 12 Tf\n");
                fprintf(pdf_file, "50 750 Td\n");
                
                // Write event data
                fprintf(pdf_file, "(Event Export Report) Tj\n");
                fprintf(pdf_file, "0 -20 Td\n");
                for (int i = 0; i < num_events; i++) {
                    fprintf(pdf_file, "(Event ID: %ld) Tj\n", events[i].event_id);
                    fprintf(pdf_file, "0 -15 Td\n");
                    fprintf(pdf_file, "(Name: %s) Tj\n", events[i].name);
                    fprintf(pdf_file, "0 -15 Td\n");
                    fprintf(pdf_file, "(Date: %s) Tj\n", events[i].date);
                    fprintf(pdf_file, "0 -15 Td\n");
                    fprintf(pdf_file, "---) Tj\n");
                    fprintf(pdf_file, "0 -15 Td\n");
                }
                
                fprintf(pdf_file, "ET\n");
                fprintf(pdf_file, "endstream\n");
                fprintf(pdf_file, "endobj\n");
                
                // Finish PDF
                fprintf(pdf_file, "3 0 obj\n");
                fprintf(pdf_file, "<<\n");
                fprintf(pdf_file, "/Type /Pages\n");
                fprintf(pdf_file, "/Kids [2 0 R]\n");
                fprintf(pdf_file, "/Count 1\n");
                fprintf(pdf_file, ">>\n");
                fprintf(pdf_file, "endobj\n");
                fprintf(pdf_file, "xref\n");
                fprintf(pdf_file, "0 5\n");
                fprintf(pdf_file, "0000000000 65535 f\n");
                fprintf(pdf_file, "0000000010 00000 n\n");
                fprintf(pdf_file, "0000000100 00000 n\n");
                fprintf(pdf_file, "0000000200 00000 n\n");
                fprintf(pdf_file, "0000000300 00000 n\n");
                fprintf(pdf_file, "trailer\n");
                fprintf(pdf_file, "<<\n");
                fprintf(pdf_file, "/Size 5\n");
                fprintf(pdf_file, "/Root 1 0 R\n");
                fprintf(pdf_file, ">>\n");
                fprintf(pdf_file, "startxref\n");
                fprintf(pdf_file, "500\n");
                fprintf(pdf_file, "%%EOF\n");
                
                fclose(pdf_file);
                
                // Success dialog
                GtkWidget *success_dialog = gtk_message_dialog_new(
                    parent_window,
                    GTK_DIALOG_MODAL,
                    GTK_MESSAGE_INFO,
                    GTK_BUTTONS_OK,
                    "Events exported successfully to %s!", filepath
                );
                trigger_notification("Exported Events Successfully!");
                gtk_dialog_run(GTK_DIALOG(success_dialog));
                gtk_widget_destroy(success_dialog);
            } else {
                // Error dialog if file can't be created
                GtkWidget *error_dialog = gtk_message_dialog_new(
                    parent_window,
                    GTK_DIALOG_MODAL,
                    GTK_MESSAGE_ERROR,
                    GTK_BUTTONS_OK,
                    "Failed to create export file!"
                );
                gtk_dialog_run(GTK_DIALOG(error_dialog));
                gtk_widget_destroy(error_dialog);
            }
        }

        // Cleanup
        g_free(filename);
        g_free(filepath);
        g_free(export_format);
    }

    // Final cleanup
    gtk_widget_destroy(export_dialog);
    free(events);
}

//Import Events Button
static void load_events_after_import(GtkWidget *events_list) {
    // Clear existing list
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(events_list)));
    gtk_list_store_clear(store);

    // Read events from file and repopulate the list
    int num_events = 0;
    Event *events = read_events_from_file(&num_events);

    for (int i = 0; i < num_events; i++) {
        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
            0, events[i].name,
            1, events[i].date,
            2, events[i].status,
            3, events[i].event_id,
            4, events[i].location,
            5, events[i].category,
            6, events[i].attendees,
            7, events[i].budget,
            -1);
    }

    // Free the events array
    if (events) {
        free(events);
    }
}


static void on_import_event_button_clicked(GtkWidget *widget, gpointer data) {
    GtkWindow *parent_window = GTK_WINDOW(data);

    // Create file chooser dialog
    GtkWidget *file_chooser = gtk_file_chooser_dialog_new(
        "Import Events",
        parent_window,
        GTK_FILE_CHOOSER_ACTION_OPEN,
        "Cancel", GTK_RESPONSE_CANCEL,
        "Import", GTK_RESPONSE_OK,
        NULL
    );

    // Add file filters to help user select text files
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filter, "*.txt");
    gtk_file_filter_set_name(filter, "Text Files");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(file_chooser), filter);

    // Run the dialog
    gint result = gtk_dialog_run(GTK_DIALOG(file_chooser));
    
    if (result == GTK_RESPONSE_OK) {
        // Get the selected filename
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_chooser));
        
        // Open the file
        FILE *import_file = fopen(filename, "r");
        
        if (import_file) {
            // Open events file for appending
            FILE *events_file = fopen("Events.txt", "a");
            
            if (events_file) {
                char line[1024];
                Event new_event;
                int field_count = 0;
                
                // Reset event fields
                memset(&new_event, 0, sizeof(Event));
                
                // Read file line by line
                while (fgets(line, sizeof(line), import_file)) {
                    // Trim newline
                    line[strcspn(line, "\n")] = 0;
                    
                    // Skip empty lines
                    if (strlen(line) == 0) continue;
                    
                    // Check for event separator
                    if (strcmp(line, "---") == 0) {
                        // If we have a complete event, write it to Events.txt
                        if (field_count == 8) {
                            fprintf(events_file, "Event ID: %ld\n", new_event.event_id);
                            fprintf(events_file, "Name: %s\n", new_event.name);
                            fprintf(events_file, "Date: %s\n", new_event.date);
                            fprintf(events_file, "Location: %s\n", new_event.location);
                            fprintf(events_file, "Category: %s\n", new_event.category);
                            fprintf(events_file, "Status: %s\n", new_event.status);
                            fprintf(events_file, "Attendees: %d\n", new_event.attendees);
                            fprintf(events_file, "Budget: %.2f\n", new_event.budget);
                            fprintf(events_file, "---\n");
                        }
                        
                        // Reset for next event
                        memset(&new_event, 0, sizeof(Event));
                        field_count = 0;
                        continue;
                    }
                    
                    // Parse event fields
                    if (strncmp(line, "Event ID: ", 10) == 0) {
                        new_event.event_id = atol(line + 10);
                        field_count++;
                    }
                    else if (strncmp(line, "Name: ", 6) == 0) {
                        strncpy(new_event.name, line + 6, sizeof(new_event.name) - 1);
                        field_count++;
                    }
                    else if (strncmp(line, "Date: ", 6) == 0) {
                        strncpy(new_event.date, line + 6, sizeof(new_event.date) - 1);
                        field_count++;
                    }
                    else if (strncmp(line, "Location: ", 10) == 0) {
                        strncpy(new_event.location, line + 10, sizeof(new_event.location) - 1);
                        field_count++;
                    }
                    else if (strncmp(line, "Category: ", 10) == 0) {
                        strncpy(new_event.category, line + 10, sizeof(new_event.category) - 1);
                        field_count++;
                    }
                    else if (strncmp(line, "Status: ", 8) == 0) {
                        strncpy(new_event.status, line + 8, sizeof(new_event.status) - 1);
                        field_count++;
                    }
                    else if (strncmp(line, "Attendees: ", 11) == 0) {
                        new_event.attendees = atoi(line + 11);
                        field_count++;
                    }
                    else if (strncmp(line, "Budget: ", 8) == 0) {
                        new_event.budget = atof(line + 8);
                        field_count++;
                    }
                }
                
                // Close files
                fclose(import_file);
                fclose(events_file);
                
                // Success dialog
                GtkWidget *success_dialog = gtk_message_dialog_new(
                    parent_window,
                    GTK_DIALOG_MODAL,
                    GTK_MESSAGE_INFO,
                    GTK_BUTTONS_OK,
                    "Events imported successfully!"
                );
                trigger_notification("New Items Successfully Imported!");
                gtk_dialog_run(GTK_DIALOG(success_dialog));
                gtk_widget_destroy(success_dialog);
                
                // Find and update the events list in the notebook
                if (global_workspace) {
                    // Assuming events list is on the second page (index 1)
                    GtkWidget *events_page = gtk_notebook_get_nth_page(GTK_NOTEBOOK(global_workspace), 1);
                    GtkWidget *events_list = NULL;
                    
                    // Search for the tree view in the events page
                    GList *children = gtk_container_get_children(GTK_CONTAINER(events_page));
                    while (children) {
                        if (GTK_IS_TREE_VIEW(children->data)) {
                            events_list = GTK_WIDGET(children->data);
                            break;
                        }
                        children = children->next;
                    }
                    
                    // Update events list if found
                    if (events_list) {
                        load_events_after_import(events_list);
                    }
                }
            } else {
                // Error opening events file
                GtkWidget *error_dialog = gtk_message_dialog_new(
                    parent_window,
                    GTK_DIALOG_MODAL,
                    GTK_MESSAGE_ERROR,
                    GTK_BUTTONS_OK,
                    "Failed to open Events.txt for writing!"
                );
                gtk_dialog_run(GTK_DIALOG(error_dialog));
                gtk_widget_destroy(error_dialog);
            }
            
            g_free(filename);
        } else {
            // Error opening import file
            GtkWidget *error_dialog = gtk_message_dialog_new(
                parent_window,
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_ERROR,
                GTK_BUTTONS_OK,
                "Failed to open selected file!"
            );
            gtk_dialog_run(GTK_DIALOG(error_dialog));
            gtk_widget_destroy(error_dialog);
        }
    }
    
    // Destroy file chooser
    gtk_widget_destroy(file_chooser);
}

// Progress bar update function
static gboolean update_progress(gpointer data) {
    LoadingScreenWidgets *widgets = (LoadingScreenWidgets *)data;
    static double progress = 0.0;

    progress += 0.01;
    if (progress > 1.0) {
        progress = 1.0;
        return G_SOURCE_REMOVE;
    }

    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(widgets->progress_bar), progress);
    return G_SOURCE_CONTINUE;
}

// Function to count lines in a file
int count_file_lines(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return 0;
    
    int lines = 0;
    char buffer[256];
    
    while (fgets(buffer, sizeof(buffer), file)) {
        lines++;
    }
    
    fclose(file);
    return lines / 8; // Since each event takes 8 lines including separator
}

// Function to calculate dashboard statistics - Fixed counting logic
// Function to calculate dashboard statistics - Fixed counting logic
DashboardStats calculate_dashboard_stats() {
    DashboardStats stats = {0};
    FILE *file = fopen("events.txt", "r");
    if (!file) return stats;

    char line[256];
    char status[50];
    char date[11];
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    char current_date[11];
    strftime(current_date, sizeof(current_date), "%Y-%m-%d", tm_now);

    // Reset counters
    stats.total_events = 0;
    stats.active_events = 0;
    stats.upcoming_events = 0;

    // Variables to track current event
    int reading_event = 0;
    char current_status[50] = "";
    char current_event_date[11] = "";

    // Process file line by line
    while (fgets(line, sizeof(line), file)) {
        // Remove newline if present
        line[strcspn(line, "\n")] = 0;
        
        if (strstr(line, "Name:") != NULL) {
            // Start of new event
            reading_event = 1;
            current_status[0] = '\0';
            current_event_date[0] = '\0';
        }
        else if (strstr(line, "Date:") != NULL && reading_event) {
            sscanf(line, "Date: %10s", current_event_date);
        }
        else if (strstr(line, "Status:") != NULL && reading_event) {
            sscanf(line, "Status: %49[^\n]", current_status);
        }
        else if (strstr(line, "---") != NULL && reading_event) {
            // End of event - process the collected data
            stats.total_events++;
            
            if (strcmp(current_status, "In Progress") == 0) {
                stats.active_events++;
            }
            
            // Compare dates properly
            if (strcmp(current_event_date, current_date) > 0) {
                stats.upcoming_events++;
            }
            
            reading_event = 0;
        }
    }
    
    fclose(file);

    // Count team members (each profile takes 4 lines)
    FILE *profile_file = fopen("user_profile.txt", "r");
    if (profile_file) {
        char profile_line[256];
        int profile_lines = 0;
        while (fgets(profile_line, sizeof(profile_line), profile_file)) {
            profile_lines++;
        }
        stats.team_members = profile_lines / 4;
        fclose(profile_file);
    }

    return stats;
}

// Function to update welcome message with user's name
void update_welcome_message(GtkWidget *welcome_label) {
    UserProfile profile;
    char welcome_text[256];
    
    if (load_user_profile(&profile)) {
        snprintf(welcome_text, sizeof(welcome_text), 
                "<span font='24'>Welcome, %s</span>\n<span font='16'>Today is your day to shine!</span>",
                profile.username);
    } else {
        snprintf(welcome_text, sizeof(welcome_text),
                "<span font='24'>Welcome, Team Manager</span>\n<span font='16'>Today is your day to shine!</span>");
    }
    
    gtk_label_set_markup(GTK_LABEL(welcome_label), welcome_text);
}

// Function to update dashboard statistics
void update_dashboard_stats(GtkWidget *stats_grid) {
    DashboardStats stats = calculate_dashboard_stats();
    
    // Update each stat in the grid
    int values[4] = {stats.total_events, stats.active_events, stats.team_members, stats.upcoming_events};
    
    // Get all stat boxes from the grid
    for (int i = 0; i < 4; i++) {
        // Get the box widget directly
        GtkWidget *child = gtk_grid_get_child_at(GTK_GRID(stats_grid), i % 2, i / 2);
        if (child && GTK_IS_CONTAINER(child)) {
            // Get the list of children
            GList *children = gtk_container_get_children(GTK_CONTAINER(child));
            GList *iter = children;
            
            // Look for the text_box (second child)
            if (iter) {
                iter = iter->next;  // Skip icon box
                if (iter && GTK_IS_BOX(iter->data)) {
                    GtkWidget *text_box = GTK_WIDGET(iter->data);
                    
                    // Get children of text box
                    GList *text_children = gtk_container_get_children(GTK_CONTAINER(text_box));
                    if (text_children && GTK_IS_LABEL(text_children->data)) {
                        GtkWidget *value_label = GTK_WIDGET(text_children->data);
                        
                        char value_str[32];
                        snprintf(value_str, sizeof(value_str), "%d", values[i]);
                        gtk_label_set_text(GTK_LABEL(value_label), value_str);
                        
                        g_list_free(text_children);
                    }
                }
            }
            
            g_list_free(children);
        }
    }
}

// Function to load recent events into the dashboard list
void load_recent_events(GtkWidget *events_list) {
    // First, clear the existing model
    GtkTreeModel *old_model = gtk_tree_view_get_model(GTK_TREE_VIEW(events_list));
    if (old_model) {
        g_object_unref(old_model);
    }
    
    // Create new list store
    GtkListStore *store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    
    FILE *file = fopen("events.txt", "r");
    if (!file) {
        gtk_tree_view_set_model(GTK_TREE_VIEW(events_list), GTK_TREE_MODEL(store));
        g_object_unref(store);
        return;
    }
    
    // Store events in a temporary array to show most recent first
    typedef struct {
        char name[100];
        char date[11];
        char status[50];
    } EventEntry;
    
    EventEntry events[100];  // Assuming max 100 events
    int event_count = 0;
    
    char line[256];
    while (fgets(line, sizeof(line), file) && event_count < 100) {
        if (strstr(line, "Name:") != NULL) {
            sscanf(line, "Name: %99[^\n]", events[event_count].name);
            
            // Get date
            fgets(line, sizeof(line), file);
            sscanf(line, "Date: %10s", events[event_count].date);
            
            // Skip location and category
            fgets(line, sizeof(line), file);
            fgets(line, sizeof(line), file);
            
            // Get status
            fgets(line, sizeof(line), file);
            sscanf(line, "Status: %49[^\n]", events[event_count].status);
            
            // Skip remaining event fields
            fgets(line, sizeof(line), file);
            fgets(line, sizeof(line), file);
            
            event_count++;
        }
    }
    fclose(file);
    
    // Add the 5 most recent events to the store
    for (int i = event_count - 1; i >= MAX(0, event_count - 5); i--) {
        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                          0, events[i].name,
                          1, events[i].date,
                          2, events[i].status,
                          -1);
    }
    
    gtk_tree_view_set_model(GTK_TREE_VIEW(events_list), GTK_TREE_MODEL(store));
    g_object_unref(store);

    // Set up the selection style
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(events_list));
    gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
    
    // Apply custom CSS for the tree view
    GtkCssProvider *provider = gtk_css_provider_new();
    const char *css =
        "treeview:selected {"
        "    background-color: #3584e4;"  // Nice blue color
        "    color: #ffffff;"             // White text
        "}"
        "treeview {"
        "    color: #000000;"            // Black text for unselected
        "    background-color: #ffffff;"  // White background
        "}"
        "treeview:selected:focus {"
        "    background-color: #3584e4;"  // Keep same blue when focused
        "    color: #ffffff;"
        "}";
        
    gtk_css_provider_load_from_data(provider, css, -1, NULL);
    GtkStyleContext *context = gtk_widget_get_style_context(events_list);
    gtk_style_context_add_provider(context,
                                 GTK_STYLE_PROVIDER(provider),
                                 GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}

// Function to refresh all dashboard components
gboolean refresh_dashboard(gpointer user_data) {
    gpointer *widgets = (gpointer *)user_data;
    GtkWidget *welcome_label = GTK_WIDGET(widgets[0]);
    GtkWidget *stats_grid = GTK_WIDGET(widgets[1]);
    GtkWidget *events_list = GTK_WIDGET(widgets[2]);
    
    update_welcome_message(welcome_label);
    update_dashboard_stats(stats_grid);
    load_recent_events(events_list);
    
    return G_SOURCE_CONTINUE;  // Keep the timer running
}

// Structure to store widget pointers for refresh callback
typedef struct {
    GtkWidget *welcome_label;
    GtkWidget *stats_grid;
    GtkWidget *events_list;
} DashboardWidgets;

// Modified version of the setup_auto_refresh part
void setup_dashboard_auto_refresh(GtkWidget *welcome_label, GtkWidget *stats_grid, GtkWidget *events_list) {
    // Allocate memory for the widgets structure
    DashboardWidgets *widgets = g_new(DashboardWidgets, 1);
    widgets->welcome_label = welcome_label;
    widgets->stats_grid = stats_grid;
    widgets->events_list = events_list;
    
    // Set up the auto-refresh timer
    g_timeout_add_seconds_full(G_PRIORITY_DEFAULT,
                             10,  // 30 second refresh interval
                             refresh_dashboard,
                             widgets,
                             g_free);  // Free the widgets structure when the timer is destroyed
}

// Dynamic loading text update function
static gboolean update_loading_text(gpointer data) {
    LoadingScreenWidgets *widgets = (LoadingScreenWidgets *)data;
    static int step = 0;
    const char *messages[] = {
        "Events become simple",
        "Organize with ease", 
        "Empower your team",
        "Stay organized with EventPro",
        "Wishhhhhhhh",
        "Loading EventPro...",
    };

    gtk_label_set_text(GTK_LABEL(widgets->loading_label), messages[step % 6]);
    step++;
    return G_SOURCE_CONTINUE;
}

// Apply CSS styling to window
static void setup_window_styling(GtkWidget *window, const char *bg_color) {
    GtkCssProvider *css_provider = gtk_css_provider_new();
    char css_data[256];
    
    snprintf(css_data, sizeof(css_data), 
        "window { background-color: %s; }"
        "button { background-color: #4a4a4a; color: white; }"
        "label { font-size: 16px; color: black; }", 
        bg_color);

    gtk_css_provider_load_from_data(css_provider, css_data, -1, NULL);
    
    GtkStyleContext *context = gtk_widget_get_style_context(window);
    gtk_style_context_add_provider(context, 
        GTK_STYLE_PROVIDER(css_provider), 
        GTK_STYLE_PROVIDER_PRIORITY_USER);
    
    g_object_unref(css_provider);
}

void show_credits_dialog(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Credits & Acknowledgments", 
        GTK_WINDOW(data), 
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, 
        "Close", GTK_RESPONSE_CLOSE, 
        NULL
    );

    // Set dialog size and styling
    gtk_window_set_default_size(GTK_WINDOW(dialog), 500, 400);
    gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

    // Get content area
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_widget_set_margin_start(content_area, 20);
    gtk_widget_set_margin_end(content_area, 20);

    // Create main vertical box
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_container_add(GTK_CONTAINER(content_area), main_box);

    // Add logo/header
    GtkWidget *header_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(header_label), 
        "<span font='20' weight='bold'>EventPro</span>\n"
        "<span font='12' style='italic'>Professional Event Management</span>");
    gtk_widget_set_margin_top(header_label, 10);
    gtk_box_pack_start(GTK_BOX(main_box), header_label, FALSE, FALSE, 0);

    // Add separator
    GtkWidget *separator1 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(main_box), separator1, FALSE, FALSE, 0);

    // Team section
    GtkWidget *team_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(team_label),
        "<span font='14' weight='bold'>Development Team</span>\n\n"
        "<span font='12'>Project Lead:</span> <span font='12' weight='bold'>Ajay</span>\n"
        "<span font='12'>Core Team:</span> <span font='12' weight='bold'>Nakul, Nainshu</span>");
    gtk_box_pack_start(GTK_BOX(main_box), team_label, FALSE, FALSE, 0);

    // Add separator
    GtkWidget *separator2 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(main_box), separator2, FALSE, FALSE, 0);

    // Technical details
    GtkWidget *tech_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(tech_label),
        "<span font='14' weight='bold'>Technical Details</span>\n\n"
        "<span font='12'>Framework:</span> GTK+ 3.0\n"
        "<span font='12'>Language:</span> C Programming\n"
        "<span font='12'>Development Time:</span> 3 Days\n"
        "<span font='12'>Development Type:</span> Hackathon Project");
    gtk_box_pack_start(GTK_BOX(main_box), tech_label, FALSE, FALSE, 0);

    // Add separator
    GtkWidget *separator3 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(main_box), separator3, FALSE, FALSE, 0);

    // Footer
    GtkWidget *footer_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(footer_label),
        "<span font='11' style='italic'>Created with ❤️ during the Hackathon\n"
        "Copyright © 2024 EventPro Team</span>");
    gtk_box_pack_start(GTK_BOX(main_box), footer_label, FALSE, FALSE, 0);

    // Apply custom CSS
    GtkCssProvider *provider = gtk_css_provider_new();
    const char *css =
        "dialog { background-color: #ffffff; }"
        "dialog * { color: #333333; }"
        "separator { background-color: #e0e0e0; margin: 10px 0; }";
    gtk_css_provider_load_from_data(provider, css, -1, NULL);
    GtkStyleContext *context = gtk_widget_get_style_context(dialog);
    gtk_style_context_add_provider(context,
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);

    // Show dialog and handle response
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}


void show_settings_dialog(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Settings", 
        GTK_WINDOW(data), 
        GTK_DIALOG_MODAL, 
        "Save", GTK_RESPONSE_OK,
        "Cancel", GTK_RESPONSE_CANCEL,
        NULL
    );
    
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_set_border_width(GTK_CONTAINER(content_area), 20);
    
    // Basic settings layout
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    
    // Auto-refresh Dashboard Setting
    GtkWidget *refresh_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *refresh_label = gtk_label_new("Auto-refresh Dashboard:");
    GtkWidget *refresh_switch = gtk_switch_new();
    gtk_box_pack_start(GTK_BOX(refresh_box), refresh_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(refresh_box), refresh_switch, FALSE, FALSE, 0);
    
    // Notification Settings
    GtkWidget *notif_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *notif_label = gtk_label_new("Enable Notifications:");
    GtkWidget *notif_switch = gtk_switch_new();
    gtk_box_pack_start(GTK_BOX(notif_box), notif_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(notif_box), notif_switch, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(main_box), refresh_box, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(main_box), notif_box, FALSE, FALSE, 10);
    
    gtk_container_add(GTK_CONTAINER(content_area), main_box);
    gtk_widget_show_all(dialog);
    
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (response == GTK_RESPONSE_OK) {
        // Save settings
        gboolean auto_refresh = gtk_switch_get_active(GTK_SWITCH(refresh_switch));
        gboolean notifications = gtk_switch_get_active(GTK_SWITCH(notif_switch));
        
        // Here you would implement the actual settings saving logic
        g_print("Settings saved - Auto-refresh: %d, Notifications: %d\n", 
                auto_refresh, notifications);
    }
    
    gtk_widget_destroy(dialog);
}

//Reports Window
void show_reports_window(GtkWidget *widget, gpointer data) {
    GtkWidget *reports_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(reports_window), "EventPro Analytics & Reports");
    gtk_window_set_default_size(GTK_WINDOW(reports_window), 800, 600);
    gtk_window_set_position(GTK_WINDOW(reports_window), GTK_WIN_POS_CENTER);
    gtk_container_set_border_width(GTK_CONTAINER(reports_window), 20);

    // Main vertical box
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_container_add(GTK_CONTAINER(reports_window), main_box);

    // Header
    GtkWidget *header_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(header_label),
        "<span font='18' weight='bold'>EventPro Analytics Dashboard</span>");
    gtk_box_pack_start(GTK_BOX(main_box), header_label, FALSE, FALSE, 0);

    // Add separator
    GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(main_box), separator, FALSE, FALSE, 10);

    // Create controls box
    GtkWidget *controls_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(main_box), controls_box, FALSE, FALSE, 0);

    // Add refresh button
    GtkWidget *refresh_btn = gtk_button_new_with_label("🔄 Refresh Report");
    gtk_widget_set_size_request(refresh_btn, 150, 40);
    gtk_box_pack_start(GTK_BOX(controls_box), refresh_btn, FALSE, FALSE, 0);

    // Add export button
    GtkWidget *export_btn = gtk_button_new_with_label("📥 Export Report");
    gtk_widget_set_size_request(export_btn, 150, 40);
    gtk_box_pack_start(GTK_BOX(controls_box), export_btn, FALSE, FALSE, 0);

    // Create scrolled window for text view
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(main_box), scroll, TRUE, TRUE, 0);

    // Create text view with monospace font
    GtkWidget *text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD_CHAR);
    gtk_container_add(GTK_CONTAINER(scroll), text_view);

    // Connect signals
    g_signal_connect(refresh_btn, "clicked", G_CALLBACK(generate_report), text_view);
    g_signal_connect(export_btn, "clicked", G_CALLBACK(export_report), text_view);

    // Apply custom CSS
    GtkCssProvider *provider = gtk_css_provider_new();
    const char *css =
        "window { background-color: #ffffff; }"
        "button { background-image: none; background-color: #f0f0f0; "
        "        border: 1px solid #d0d0d0; border-radius: 5px; "
        "        padding: 8px 15px; }"
        "button:hover { background-color: #e0e0e0; }"
        "textview { font-family: 'Monaco', monospace; font-size: 12px; "
        "           padding: 10px; background-color: #f8f9fa; }"
        "textview text { margin: 15px; }";
    gtk_css_provider_load_from_data(provider, css, -1, NULL);
    GtkStyleContext *context = gtk_widget_get_style_context(reports_window);
    gtk_style_context_add_provider(context,
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);

    // Generate initial report
    generate_report(NULL, text_view);

    // Show window
    gtk_widget_show_all(reports_window);
}

void generate_report(GtkWidget *widget, gpointer data) {
    // Define a reasonable maximum number of events to prevent potential buffer overruns
    #define MAX_SAFE_EVENTS 1000

    GtkTextView *text_view = GTK_TEXT_VIEW(data);
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(text_view);
    
    // Validate text view
    if (!text_view || !buffer) {
        g_warning("Invalid text view or buffer");
        return;
    }
    
    // Read events using existing function
    int num_events;
    Event* events = read_events_from_file(&num_events);
    
    if (!events || num_events == 0) {
        gtk_text_buffer_set_text(buffer, "No events found or error reading events file.", -1);
        return;
    }
    
    // Limit number of events to prevent potential issues
    num_events = (num_events > MAX_SAFE_EVENTS) ? MAX_SAFE_EVENTS : num_events;
    
    // Initialize counters and totals with safety
    int planned_events = 0, ongoing_events = 0;
    int completed_events = 0, cancelled_events = 0;
    double total_budget = 0.0;
    int total_attendees = 0;
    
    // Process events with bounds checking
    for (int i = 0; i < num_events; i++) {
        // Validate individual event before processing
        if (!events[i].name || !events[i].status) continue;
        
        total_budget += events[i].budget;
        total_attendees += events[i].attendees;
        
        // Use safer string comparison
        if (strncmp(events[i].status, "Planned", 7) == 0) planned_events++;
        else if (strncmp(events[i].status, "Ongoing", 7) == 0) ongoing_events++;
        else if (strncmp(events[i].status, "Completed", 9) == 0) completed_events++;
        else if (strncmp(events[i].status, "Cancelled", 9) == 0) cancelled_events++;
    }
    
    // Create report text buffer
    GString *report = g_string_new("");
    
    // Get current datetime with error handling
    time_t now;
    struct tm *tmp;
    char datetime[64] = "Unknown Date";
    
    time(&now);
    tmp = localtime(&now);
    if (tmp) {
        strftime(datetime, sizeof(datetime), "%Y-%m-%d %H:%M:%S", tmp);
    }
    
    // Stylized Report Header
    g_string_append_printf(report, 
        "==================================================\n"
        "           EventPro Analytics Report\n"
        "==================================================\n"
        "Generated: %s\n\n", datetime);
    
    // Event Status Overview with visual separation
    g_string_append_printf(report, 
        "EVENT STATUS OVERVIEW\n"
        "--------------------\n");
    g_string_append_printf(report, "Total Events:     %d\n", num_events);
    g_string_append_printf(report, "Planned:          %d\n", planned_events);
    g_string_append_printf(report, "Ongoing:          %d\n", ongoing_events);
    g_string_append_printf(report, "Completed:        %d\n", completed_events);
    g_string_append_printf(report, "Cancelled:        %d\n\n", cancelled_events);
    
    // Financial Insights
    g_string_append_printf(report, 
        "FINANCIAL INSIGHTS\n"
        "------------------\n");
    g_string_append_printf(report, "Total Budget:     ₹%.2f\n", total_budget);
    if (num_events > 0) {
        g_string_append_printf(report, "Avg Budget/Event: ₹%.2f\n\n", 
            total_budget / num_events);
    }
    
    // Attendance Metrics
    g_string_append_printf(report, 
        "ATTENDANCE METRICS\n"
        "-------------------\n");
    g_string_append_printf(report, "Total Attendees:  %d\n", total_attendees);
    if (num_events > 0) {
        g_string_append_printf(report, "Avg Attendees:    %d\n\n", 
            total_attendees / num_events);
    }
    
    // Recent Events
    g_string_append_printf(report, 
        "RECENT EVENTS\n"
        "-------------\n");
    
    // Show last 5 events or all if less than 5
    int start_idx = num_events > 5 ? num_events - 5 : 0;
    for (int i = start_idx; i < num_events; i++) {
        g_string_append_printf(report, 
            "• %s\n  Status: %s\n  Date: %s\n  Budget: ₹%.2f\n\n", 
            events[i].name, 
            events[i].status, 
            events[i].date, 
            events[i].budget);
    }
    
    // Set the report text with appropriate font
    gtk_text_buffer_set_text(buffer, report->str, -1);
    
    // Apply monospace font and some styling
    GtkTextTag *tag = gtk_text_buffer_create_tag(buffer, NULL, 
        "family", "Monospace", 
        "scale", 1.0,  // Normal size
        "foreground", "#333333",  // Dark gray text
        NULL);
    
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    gtk_text_buffer_apply_tag(buffer, tag, &start, &end);
    
    // Free resources
    g_string_free(report, TRUE);
    free(events);

    // Remove the macro after use
    #undef MAX_SAFE_EVENTS
}

// Update the CSS in show_reports_window function
const char *css =
    "window { "
    "    background-color: #f5f5f7; "
    "} "
    "button { "
    "    background-color: #4A90E2; "
    "    color: white; "
    "    border: none; "
    "    border-radius: 4px; "
    "    padding: 8px 15px; "
    "    margin: 5px; "
    "    font-weight: bold; "
    "} "
    "button:hover { "
    "    background-color: #357ABD; "
    "} "
    "textview { "
    "    background-color: white; "
    "    border-radius: 6px; "
    "    padding: 15px; "
    "    box-shadow: 0 2px 4px rgba(0,0,0,0.1); "
    "}";


// Function to create a new notification
Notification* create_notification(const char *message) {
    Notification *notif = g_new(Notification, 1);
    notif->message = g_strdup(message);
    
    // Get current timestamp
    GDateTime *now = g_date_time_new_now_local();
    notif->timestamp = g_date_time_format(now, "%Y-%m-%d %H:%M:%S");
    g_date_time_unref(now);
    
    notif->is_read = FALSE;
    return notif;
}

// Function to free notification
// Function to free notification
void free_notification(Notification *notif) {
    if (notif) {
        g_free(notif->message);
        g_free(notif->timestamp);
        g_free(notif);
    }
}

// Function to count unread notifications
static int count_unread_notifications() {
    int unread = 0;
    GList *iter = notifications_list;
    while (iter) {
        Notification *notif = iter->data;
        if (!notif->is_read) unread++;
        iter = iter->next;
    }
    return unread;
}

// Function to update notification button label
static void update_notification_button_label() {
    if (global_notification_button) {
        int unread = count_unread_notifications();
        char *label = g_strdup_printf("🔔 Notifications (%d)", unread);
        gtk_button_set_label(GTK_BUTTON(global_notification_button), label);
        g_free(label);
    }
}

// Function to add a new notification
void add_notification(const char *message) {
    Notification *notif = create_notification(message);
    notifications_list = g_list_append(notifications_list, notif);
    update_notification_button_label();
}

// Function to create notification list window
static void show_notifications_dialog(GtkWidget *button, gpointer user_data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Notifications",
        GTK_WINDOW(user_data),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "Mark All Read", GTK_RESPONSE_APPLY,
        "Close", GTK_RESPONSE_CLOSE,
        NULL
    );
    
    // Set dialog size and make it resizable
    gtk_window_set_default_size(GTK_WINDOW(dialog), 500, 600);
    gtk_window_set_resizable(GTK_WINDOW(dialog), TRUE);
    
    // Get the content area of the dialog
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_widget_set_margin_start(content_area, 20);
    gtk_widget_set_margin_end(content_area, 20);
    gtk_widget_set_margin_top(content_area, 20);
    gtk_widget_set_margin_bottom(content_area, 20);
    
    // Create scrolled window for notifications
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                 GTK_POLICY_NEVER,
                                 GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand(scrolled, TRUE);
    
    // Create box to hold notifications
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(box), 10);
    
    // Add "No notifications" message if list is empty
    if (notifications_list == NULL) {
        GtkWidget *no_notif_label = gtk_label_new("No notifications");
        gtk_widget_set_name(no_notif_label, "no-notifications-label");
        gtk_box_pack_start(GTK_BOX(box), no_notif_label, TRUE, TRUE, 0);
    } else {
        // Add notifications to box
        GList *iter = notifications_list;
        while (iter) {
            Notification *notif = iter->data;
            
            // Create notification card
            GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
            gtk_widget_set_name(card, "notification-card");
            
            // Add message with proper wrapping
            GtkWidget *msg_label = gtk_label_new(notif->message);
            gtk_label_set_line_wrap(GTK_LABEL(msg_label), TRUE);
            gtk_label_set_line_wrap_mode(GTK_LABEL(msg_label), PANGO_WRAP_WORD_CHAR);
            gtk_widget_set_halign(msg_label, GTK_ALIGN_START);
            gtk_widget_set_hexpand(msg_label, TRUE);
            
            // Add timestamp with styling
            GtkWidget *time_label = gtk_label_new(notif->timestamp);
            gtk_widget_set_name(time_label, "timestamp-label");
            gtk_widget_set_halign(time_label, GTK_ALIGN_START);
            
            // Add unread indicator if needed
            if (!notif->is_read) {
                gtk_widget_set_name(card, "notification-card-unread");
            }
            
            gtk_box_pack_start(GTK_BOX(card), msg_label, FALSE, FALSE, 0);
            gtk_box_pack_start(GTK_BOX(card), time_label, FALSE, FALSE, 0);
            
            // Add separator after each card except the last one
            gtk_box_pack_start(GTK_BOX(box), card, FALSE, FALSE, 0);
            if (iter->next != NULL) {
                GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
                gtk_box_pack_start(GTK_BOX(box), separator, FALSE, FALSE, 5);
            }
            
            // Mark as read
            notif->is_read = TRUE;
            
            iter = iter->next;
        }
    }
    
    // Add CSS styling
    GtkCssProvider *css_provider = gtk_css_provider_new();
    const char *css =
        "window { background-color: #ffffff; }"
        
        "#notification-card,"
        "#notification-card-unread {"
        "    background-color: #f8f9fa;"
        "    border: 1px solid #e0e0e0;"
        "    border-radius: 8px;"
        "    padding: 15px;"
        "    margin: 5px 0;"
        "}"
        
        "#notification-card-unread {"
        "    border-left: 4px solid #3498db;"
        "}"
        
        "#timestamp-label {"
        "    color: #666666;"
        "    font-size: 12px;"
        "    margin-top: 5px;"
        "}"
        
        "#no-notifications-label {"
        "    color: #666666;"
        "    font-size: 14px;"
        "    font-style: italic;"
        "    padding: 20px;"
        "}";
    
    gtk_css_provider_load_from_data(css_provider, css, -1, NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    g_object_unref(css_provider);
    
    // Add scrolled window to dialog
    gtk_container_add(GTK_CONTAINER(scrolled), box);
    gtk_container_add(GTK_CONTAINER(content_area), scrolled);
    
    // Update notification count on button
    update_notification_button_label();
    
    // Show dialog and handle response
    gtk_widget_show_all(dialog);
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (response == GTK_RESPONSE_APPLY) {
        // Mark all notifications as read
        GList *iter = notifications_list;
        while (iter) {
            Notification *notif = iter->data;
            notif->is_read = TRUE;
            iter = iter->next;
        }
        update_notification_button_label();
    }
    
    gtk_widget_destroy(dialog);
}


// Function to setup notifications button
void setup_notifications_button(GtkWidget *button, GtkWidget *parent_window) {
    // Store button reference in global variable
    global_notification_button = button;
    
    // Connect click handler
    g_signal_connect(button, "clicked", G_CALLBACK(show_notifications_dialog), parent_window);
    
    // Set initial label
    gtk_button_set_label(GTK_BUTTON(button), "🔔 Notifications (0)");
}

// Example function to trigger notifications from other parts of the application
void trigger_notification(const char *message) {
    add_notification(message);
}


// Create main application window
// Enhanced main window creation with improved UI
static void create_main_window(GtkWidget *loading_window) {
    // Remove loading screen timers
    LoadingScreenWidgets *loading_widgets = g_object_get_data(G_OBJECT(loading_window), "loading_widgets");
    if (loading_widgets) {
        if (loading_widgets->progress_timer_id) 
            g_source_remove(loading_widgets->progress_timer_id);
        if (loading_widgets->hint_timer_id)
            g_source_remove(loading_widgets->hint_timer_id);
    }

    // Destroy loading window
    gtk_widget_destroy(loading_window);

    // Create main window
    GtkWidget *main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(main_window), "EventPro - Professional Event Management");
    gtk_window_set_default_size(GTK_WINDOW(main_window), 1800, 1000);
    gtk_window_maximize(GTK_WINDOW(main_window));

    // Apply more professional styling with a lighter background
    setup_window_styling(main_window, "#ffffff");

    // Main vertical box
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(main_window), main_box);

    // Original header styling
    GtkWidget *header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);
    gtk_box_set_homogeneous(GTK_BOX(header), FALSE);
    gtk_box_pack_start(GTK_BOX(main_box), header, FALSE, FALSE, 15);

    // Logo with original styling
    GtkWidget *logo = gtk_image_new_from_file("eventpro_logo.png");
    gtk_box_pack_start(GTK_BOX(header), logo, FALSE, FALSE, 30);

    // Spacer
    GtkWidget *header_spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(header), header_spacer, TRUE, TRUE, 0);

    // Original header buttons
    GtkWidget *notifications_button = gtk_button_new_with_label("🔔 Notifications");
    gtk_widget_set_name(notifications_button, "header-button");
    setup_notifications_button(notifications_button, main_window);
    
    GtkWidget *user_profile = gtk_button_new_with_label("👤 User Profile");
    gtk_widget_set_name(user_profile, "header-button");
    g_signal_connect(user_profile, "clicked", G_CALLBACK(show_profile_dialog), main_window);
    
    gtk_box_pack_end(GTK_BOX(header), user_profile, FALSE, FALSE, 15);
    gtk_box_pack_end(GTK_BOX(header), notifications_button, FALSE, FALSE, 0);

    // Main content area (horizontal split)
    GtkWidget *content_area = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(main_box), content_area, TRUE, TRUE, 0);

    // Improved sidebar
    GtkWidget *sidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_name(sidebar, "sidebar");
    gtk_box_pack_start(GTK_BOX(content_area), sidebar, FALSE, FALSE, 0);
    gtk_widget_set_size_request(sidebar, 280, -1);

    // Sidebar buttons with improved styling
    const char *sidebar_buttons[] = {
    "📅 Dashboard", 
    "📆 Events",  
    "📊 Reports", 
    "📥 Import Events",  // New import events button
    "💳 Credits",
    "⚙️ Settings"
    };

    for (int i = 0; i < G_N_ELEMENTS(sidebar_buttons); i++) {
    GtkWidget *button = gtk_button_new_with_label(sidebar_buttons[i]);
    gtk_widget_set_name(button, "sidebar-button");
    gtk_box_pack_start(GTK_BOX(sidebar), button, FALSE, FALSE, 5);

    switch(i) {
    case 0:  // Dashboard
        g_signal_connect(button, "clicked", G_CALLBACK(switch_to_notebook_page), GINT_TO_POINTER(0));
        break;
    case 1:  // Events
        g_signal_connect(button, "clicked", G_CALLBACK(switch_to_notebook_page), GINT_TO_POINTER(1));
        break;
    case 2:  // Reports (New Window)
    {
        g_signal_connect(button, "clicked", G_CALLBACK(show_reports_window), main_window);
        break;
    }
    case 3:  // Import Events
        g_signal_connect(button, "clicked", G_CALLBACK(on_import_event_button_clicked), main_window);
        break;
    case 4:  // Credits
        g_signal_connect(button, "clicked", G_CALLBACK(show_credits_dialog), main_window);
        break;
    case 5:  // Settings
        g_signal_connect(button, "clicked", G_CALLBACK(show_settings_dialog), main_window);
        break;
    }
}


    // Main workspace area with notebook
    GtkWidget *workspace = gtk_notebook_new();
    gtk_widget_set_name(workspace, "main-workspace");
    gtk_box_pack_start(GTK_BOX(content_area), workspace, TRUE, TRUE, 0);

    // Store the workspace notebook in global variable
    global_workspace = workspace;

    // ============= DASHBOARD TAB =============
    GtkWidget *dashboard_label = gtk_label_new("Dashboard");
    GtkWidget *dashboard_content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_name(dashboard_content, "dashboard-content");

    // Add padding container
    GtkWidget *padding_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_margin_start(padding_box, 30);
    gtk_widget_set_margin_end(padding_box, 30);
    gtk_widget_set_margin_top(padding_box, 20);
    gtk_widget_set_margin_bottom(padding_box, 20);
    gtk_box_pack_start(GTK_BOX(dashboard_content), padding_box, TRUE, TRUE, 0);

    // Welcome and user greeting with enhanced typography
    GtkWidget *welcome_label = gtk_label_new(NULL);
    gtk_widget_set_name(welcome_label, "welcome-label");
    gtk_widget_set_halign(welcome_label, GTK_ALIGN_START);
    update_welcome_message(welcome_label);
    gtk_box_pack_start(GTK_BOX(padding_box), welcome_label, FALSE, FALSE, 0);

    // Stats Container with shadow
    GtkWidget *stats_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_name(stats_container, "stats-container");
    gtk_box_pack_start(GTK_BOX(padding_box), stats_container, FALSE, FALSE, 20);

    // Quick Stats Grid with modern layout
    GtkWidget *stats_grid = gtk_grid_new();
    gtk_widget_set_name(stats_grid, "stats-grid");
    gtk_grid_set_column_spacing(GTK_GRID(stats_grid), 30);
    gtk_grid_set_row_spacing(GTK_GRID(stats_grid), 30);
    gtk_widget_set_margin_start(stats_grid, 30);
    gtk_widget_set_margin_end(stats_grid, 30);
    gtk_widget_set_margin_top(stats_grid, 30);
    gtk_widget_set_margin_bottom(stats_grid, 30);
    gtk_container_add(GTK_CONTAINER(stats_container), stats_grid);

    // Create modern stat boxes
    // Create modern stat boxes
const char *stat_labels[] = {
    "Total Events", "Active Events", "Team Members", "Upcoming Events"
};
const char *stat_icons[] = {"📊", "🔄", "👥", "📅"};

for (int i = 0; i < 4; i++) {
    GtkWidget *stat_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);
    gtk_widget_set_name(stat_box, "stat-box");

    // Add a data tag for easier debugging
    g_object_set_data(G_OBJECT(stat_box), "stat-index", GINT_TO_POINTER(i));
    
    // Icon container
    GtkWidget *icon_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *icon_label = gtk_label_new(stat_icons[i]);
    gtk_widget_set_name(icon_label, "stat-icon");
    gtk_box_pack_start(GTK_BOX(icon_box), icon_label, TRUE, TRUE, 0);
    
    // Text container
    GtkWidget *text_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *value_label = gtk_label_new("0");  // Explicitly set initial value to "0"
    GtkWidget *label = gtk_label_new(stat_labels[i]);
    
    gtk_widget_set_name(value_label, "stat-value");
    gtk_widget_set_name(label, "stat-label");
    gtk_widget_set_halign(value_label, GTK_ALIGN_START);
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    
    // IMPORTANT: Pack value_label FIRST, then label
    gtk_box_pack_start(GTK_BOX(text_box), value_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(text_box), label, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(stat_box), icon_box, FALSE, FALSE, 15);
    gtk_box_pack_start(GTK_BOX(stat_box), text_box, TRUE, TRUE, 0);
    
    gtk_grid_attach(GTK_GRID(stats_grid), stat_box, i % 2, i / 2, 1, 1);
}
    update_dashboard_stats(stats_grid);

    // Recent Events Section with card-like design
    GtkWidget *events_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_widget_set_name(events_container, "events-container");
    gtk_box_pack_start(GTK_BOX(padding_box), events_container, TRUE, TRUE, 0);

    // Events header with icon
    GtkWidget *events_header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *events_icon = gtk_label_new("📋");
    GtkWidget *events_title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(events_title), "<span font='18'>Recent Events</span>");
    gtk_widget_set_name(events_title, "section-title");
    gtk_box_pack_start(GTK_BOX(events_header), events_icon, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(events_header), events_title, FALSE, FALSE, 5);
    gtk_widget_set_margin_start(events_header, 15);
    gtk_widget_set_margin_bottom(events_header, 15);
    gtk_box_pack_start(GTK_BOX(events_container), events_header, FALSE, FALSE, 0);

    // Events list with modern styling
    GtkWidget *dashboard_events_list = gtk_tree_view_new();
    gtk_widget_set_name(dashboard_events_list, "events-list");
    GtkWidget *dashboard_events_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_name(dashboard_events_scroll, "events-scroll");
    gtk_container_add(GTK_CONTAINER(dashboard_events_scroll), dashboard_events_list);
    gtk_box_pack_start(GTK_BOX(events_container), dashboard_events_scroll, TRUE, TRUE, 0);

    // Create columns for the dashboard events list with improved styling
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "padding", 10, NULL);
    column = gtk_tree_view_column_new_with_attributes("Event Name", renderer, "text", 0, NULL);
    gtk_tree_view_column_set_expand(column, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(dashboard_events_list), column);

    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "padding", 10, NULL);
    column = gtk_tree_view_column_new_with_attributes("Date", renderer, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(dashboard_events_list), column);

    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "padding", 10, NULL);
    column = gtk_tree_view_column_new_with_attributes("Status", renderer, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(dashboard_events_list), column);

    // Load recent events
    load_recent_events(dashboard_events_list);

    // Set up auto-refresh for dashboard
    setup_dashboard_auto_refresh(welcome_label, stats_grid, dashboard_events_list);

    // Apply enhanced dashboard CSS (without header styling)
    GtkCssProvider *dashboard_css_provider = gtk_css_provider_new();
    const gchar *dashboard_css = 
    "* {"
    "    font-family: 'Segoe UI', Roboto, sans-serif;"
    "}"
    "#header-button {"
    "    background: none;"
    "    border: 1px solid #e0e0e0;"
    "    border-radius: 5px;"
    "    padding: 8px 15px;"
    "}"
    "#header-button:hover {"
    "    background-color: #f5f5f5;"
    "}"
    "#sidebar {"
    "    background-color: #f8f9fa;"
    "    border-right: 1px solid #e9ecef;"
    "    padding-top: 20px;"
    "}"
    "#sidebar-button {"
    "    background: none;"
    "    border: none;"
    "    border-radius: 8px;"
    "    padding: 12px 20px;"
    "    margin: 0 15px;"
    "    text-align: left;"
    "    transition: all 0.3s ease;"
    "}"
    "#sidebar-button:hover {"
    "    background-color: #e9ecef;"
    "}"
    "#dashboard-content {"
    "    background-color: #f8f9fa;"
    "}"
    "#welcome-label {"
    "    margin-bottom: 20px;"
    "}"
    "#stats-container {"
    "    background-color: white;"
    "    border-radius: 12px;"
    "    box-shadow: 0 2px 4px rgba(0,0,0,0.05);"
    "}"
    "#stat-box {"
    "    background-color: #ffffff;"
    "    padding: 20px;"
    "    border-radius: 8px;"
    "    transition: all 0.3s ease;"
    "}"
    "#stat-icon {"
    "    font-size: 24px;"
    "    color: #3498db;"
    "}"
    "#stat-value {"
    "    font-size: 28px;"
    "    font-weight: 600;"
    "    color: #2c3e50;"
    "}"
    "#stat-label {"
    "    font-size: 14px;"
    "    color: #6c757d;"
    "    font-weight: 500;"
    "}"
    "#events-container {"
    "    background-color: white;"
    "    border-radius: 12px;"
    "    padding: 20px;"
    "    box-shadow: 0 2px 4px rgba(0,0,0,0.05);"
    "    margin-top: 20px;"
    "}"
    "#section-title {"
    "    color: #2c3e50;"
    "    font-weight: 600;"
    "}"
    "#events-list {"
    "    background-color: transparent;"
    "    border: none;"
    "}"
    "#events-scroll {"
    "    border: none;"
    "    background-color: transparent;"
    "}"
    "treeview {"
    "    background-color: transparent;"
    "    border: none;"
    "}"
    "treeview header {"
    "    background-color: #f8f9fa;"
    "    border-bottom: 2px solid #e9ecef;"
    "    padding: 8px;"
    "}"
    "treeview header button {"
    "    color: #495057;"
    "    font-weight: 600;"
    "    padding: 8px;"
    "}"
    "treeview:selected {"
    "    background-color: #e7f5ff;"  // Light blue selection
    "    color: #1864ab;"            // Darker blue text for contrast
    "}"
    "treeview:hover {"
    "    background-color: #f1f3f5;"
    "}";
    
    gtk_css_provider_load_from_data(dashboard_css_provider, dashboard_css, -1, NULL);
    GdkScreen *dashboard_screen = gdk_screen_get_default();
    gtk_style_context_add_provider_for_screen(dashboard_screen, 
        GTK_STYLE_PROVIDER(dashboard_css_provider), 
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(dashboard_css_provider);

    // Append dashboard to notebook
    gtk_notebook_append_page(GTK_NOTEBOOK(workspace), dashboard_content, dashboard_label);


    // Events tab with enhanced layout
    GtkWidget *events_label = gtk_label_new("Events");
    GtkWidget *events_content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);

    
    // Events action buttons with improved styling and more options
    GtkWidget *events_action_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);

    
    GtkWidget *add_event_btn = gtk_button_new_with_label("➕ Add Event");
    g_signal_connect(add_event_btn, "clicked", G_CALLBACK(on_add_event_button_clicked), main_window);

    GtkWidget *edit_event_btn = gtk_button_new_with_label("✏️ Edit Event");
    g_signal_connect(edit_event_btn, "clicked", G_CALLBACK(on_edit_event_button_clicked), main_window);

    GtkWidget *delete_event_btn = gtk_button_new_with_label("🗑️ Delete Event");
    g_signal_connect(delete_event_btn, "clicked", G_CALLBACK(on_delete_event_button_clicked), main_window);

    GtkWidget *view_event_btn = gtk_button_new_with_label("👀 View Event Details");
    g_signal_connect(view_event_btn, "clicked", G_CALLBACK(on_view_event_button_clicked), main_window);

    GtkWidget *search_event_btn = gtk_button_new_with_label("🔍 Search Events");
    g_signal_connect(search_event_btn, "clicked", G_CALLBACK(on_search_event_button_clicked), main_window);

    GtkWidget *sort_event_btn = gtk_button_new_with_label("📊 Sort Events");
    g_signal_connect(sort_event_btn, "clicked", G_CALLBACK(on_sort_event_button_clicked), main_window);

    GtkWidget *filter_event_btn = gtk_button_new_with_label("🧩 Filter Events");
    g_signal_connect(filter_event_btn, "clicked", G_CALLBACK(on_filter_event_button_clicked), main_window);

    GtkWidget *export_event_btn = gtk_button_new_with_label("📤 Export Events");
    g_signal_connect(export_event_btn, "clicked", G_CALLBACK(on_export_event_button_clicked), main_window);
    
    // Set consistent button styling
    const char *button_names[] = {
        "add_event", "edit_event", "delete_event", 
        "view_event", "search_event", "sort_event", 
        "filter_event", "export_event"
    };
    
    GtkWidget *event_buttons[] = {
        add_event_btn, edit_event_btn, delete_event_btn, 
        view_event_btn, search_event_btn, sort_event_btn, 
        filter_event_btn, export_event_btn
    };

    for (int i = 0; i < G_N_ELEMENTS(event_buttons); i++) {
        gtk_widget_set_name(event_buttons[i], button_names[i]);
        gtk_box_pack_start(GTK_BOX(events_action_box), event_buttons[i], FALSE, FALSE, 0);
    }

    gtk_box_pack_start(GTK_BOX(events_content), events_action_box, FALSE, FALSE, 15);

    // Events list with more columns
    GtkWidget *events_scroll = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget *events_list = gtk_tree_view_new();
    gtk_container_add(GTK_CONTAINER(events_scroll), events_list);
    gtk_box_pack_start(GTK_BOX(events_content), events_scroll, TRUE, TRUE, 0);

    // Create more detailed columns for events
    const char *column_names[] = {
       "Event ID", "Event Name", "Date", "Location", 
       "Category", "Status", "Attendees", "Budget"
    };

    for (int i = 0; i < G_N_ELEMENTS(column_names); i++) {
        GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
        GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(
           column_names[i], renderer, "text", i, NULL
       );
       gtk_tree_view_append_column(GTK_TREE_VIEW(events_list), column);
    }   

    // Add search and filter box above the events list
    GtkWidget *search_filter_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *search_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(search_entry), "Search events...");
    GtkWidget *filter_combo = gtk_combo_box_text_new();

    // Add filter options
    const char *filter_options[] = {
        "All Events", "Upcoming", "Completed", 
        "Cancelled", "By Category", "By Budget Range"
    };

     for (int i = 0; i < G_N_ELEMENTS(filter_options); i++) {
      gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(filter_combo), filter_options[i]);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(filter_combo), 0);

    gtk_box_pack_start(GTK_BOX(search_filter_box), search_entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(search_filter_box), filter_combo, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(events_content), search_filter_box, FALSE, FALSE, 10);

    // Add some additional CSS for the new buttons and search area
    GtkCssProvider *events_css_provider = gtk_css_provider_new();
const gchar *events_css = 
    "button#add_event {"
    "     margin-left: 40px;"
    "}"
    "button#add_event, button#edit_event, button#delete_event, "
    "button#view_event, button#search_event, "
    "button#sort_event, button#filter_event, button#export_event {"
    "    background-color: #f0f0f0;"
    "    color: #121212;"
    "    font-size: 18px;"
    "    border-radius: 5px;"
    "    padding: 20px 15px;"
    "    margin: 30px 10px;"
    "    box-shadow: none;"
    "    transition: all 0.4s ease;"
    "}"
    
    "button#add_event:hover, button#edit_event:hover, "
    "button#delete_event:hover, button#view_event:hover, "
    "button#search_event:hover, button#sort_event:hover, "
    "button#filter_event:hover, button#export_event:hover {"
    "    background-color: #e0e0e0;"
    "    box-shadow: 5 5px 10px rgba(0,0,0,0.1);"
    "}"
    "entry {"
    "    background-color: #f9f9f9;"
    "    border: 1px solid #ddd;"
    "    border-radius: 5px;"
    "    padding: 10px;"
    "}";

    gtk_css_provider_load_from_data(events_css_provider, events_css, -1, NULL);
    GdkScreen *events_screen = gdk_screen_get_default();
    gtk_style_context_add_provider_for_screen(events_screen, 
        GTK_STYLE_PROVIDER(events_css_provider), 
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    g_object_unref(events_css_provider);

    // Append to notebook
    gtk_notebook_append_page(GTK_NOTEBOOK(workspace), events_content, events_label);

    // Enhanced status bar
    GtkWidget *statusbar = gtk_statusbar_new();
    guint context_id = gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), "EventPro");
    gtk_statusbar_push(GTK_STATUSBAR(statusbar), context_id, "Ready - All systems operational");
    gtk_box_pack_start(GTK_BOX(main_box), statusbar, FALSE, FALSE, 5);

    // Add CSS for button styling
    GtkCssProvider *css_provider = gtk_css_provider_new();
    const gchar *css_data = 
    "button#header-button {"
    "    color: black;"
    "    border-radius: 5px;"
    "    padding: 10px 15px;"
    "    font-size: 18px;"
    "    box-shadow: none;"
    "    transition: box-shadow 0.4s ease;"
    "}"

    "button#header-button:hover {"
    "    box-shadow: 5px 5px 10px rgba(0,0,0,0.25);"
    "}"

    "button#sidebar-button {"
    "    color: #333;"
    "    border: 1px solid #ddd;"
    "    border-radius: 5px;"
    "    padding: 10px 15px;"
    "    font-size: 17px;"
    "    box-shadow: none;"
    "    transition: box-shadow 0.4s ease;"
    "}"

    "button#sidebar-button:hover {"
    "    box-shadow: 5px 5px 10px rgba(0,0,0,0.25);"
    "}"

    "button#action-button {"
    "    color: black;"
    "    border-radius: 5px;"
    "    padding: 15px 15px;"
    "    font-size: 19px;"
    "    margin-left: 15px;"
    "    margin-top: 25px"
    "}";

    gtk_css_provider_load_from_data(css_provider, css_data, -1, NULL);
    GdkScreen *screen = gdk_screen_get_default();
    gtk_style_context_add_provider_for_screen(screen, 
        GTK_STYLE_PROVIDER(css_provider), 
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(css_provider);

    // Show main window
    gtk_widget_show_all(main_window);

}

// Show loading screen
static void show_loading_screen(void) {
    // Allocate loading screen widgets structure
    LoadingScreenWidgets *widgets = g_malloc(sizeof(LoadingScreenWidgets));

    // Create loading window
    widgets->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(widgets->window), "Event Pro");
    gtk_window_set_default_size(GTK_WINDOW(widgets->window), 1600, 900);
    gtk_window_maximize(GTK_WINDOW(widgets->window));
    gtk_window_set_resizable(GTK_WINDOW(widgets->window), FALSE);
    gtk_container_set_border_width(GTK_CONTAINER(widgets->window), 10);

    // Apply black background
    setup_window_styling(widgets->window, "#000000");

    // Main layout box
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(widgets->window), main_box);

    // Top-left logo section
    GtkWidget *top_left_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    gtk_widget_set_halign(top_left_box, GTK_ALIGN_START);
    gtk_widget_set_valign(top_left_box, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(main_box), top_left_box, FALSE, FALSE, 0);

    GtkWidget *logo_image = gtk_image_new_from_file("eventpro_logo.png");
    gtk_box_pack_start(GTK_BOX(top_left_box), logo_image, FALSE, FALSE, 10);

    // Center content box
    GtkWidget *center_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(center_box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(center_box, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(main_box), center_box, TRUE, TRUE, 0);

    // Loading image
    GtkWidget *loading_image = gtk_image_new_from_file("loading_image.png");
    gtk_box_pack_start(GTK_BOX(center_box), loading_image, FALSE, FALSE, 10);

    // Progress bar
    widgets->progress_bar = gtk_progress_bar_new();
    gtk_widget_set_size_request(widgets->progress_bar, 800, 50);
    gtk_box_pack_start(GTK_BOX(center_box), widgets->progress_bar, FALSE, FALSE, 10);

    // Loading label
    widgets->loading_label = gtk_label_new("Loading...");
    gtk_widget_set_name(widgets->loading_label, "loading_hint");
    gtk_box_pack_start(GTK_BOX(center_box), widgets->loading_label, FALSE, FALSE, 10);

    // Loading label styling
    GtkCssProvider *label_css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(label_css_provider,
        "label#loading_hint { font-size: 24px; color: #00FF00; }", -1, NULL);
    
    GtkStyleContext *label_context = gtk_widget_get_style_context(widgets->loading_label);
    gtk_style_context_add_provider(label_context, 
        GTK_STYLE_PROVIDER(label_css_provider), 
        GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(label_css_provider);

    // Store widgets in window data
    g_object_set_data_full(G_OBJECT(widgets->window), "loading_widgets", 
                            widgets, (GDestroyNotify)g_free);

    // Start progress and hint timers
    widgets->progress_timer_id = g_timeout_add(50, update_progress, widgets);
    widgets->hint_timer_id = g_timeout_add(1000, update_loading_text, widgets);

    // Timer to quit loading screen
    g_timeout_add(7000, (GSourceFunc)create_main_window, widgets->window);

    // Show everything
    gtk_widget_show_all(widgets->window);
}

// Main function
int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    show_loading_screen();
    gtk_main();
    return 0;
}