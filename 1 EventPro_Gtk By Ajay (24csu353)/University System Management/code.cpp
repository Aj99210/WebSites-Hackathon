#include <iostream>
#include <gtkmm.h>
#include <sqlite3.h>
#include <string>
#include <vector>
#include <ctime>

// Constants
const int MAX_USERNAME_LENGTH = 50;
const int MAX_PASSWORD_LENGTH = 50;
const int MAX_EMAIL_LENGTH = 100;
const int MAX_NAME_LENGTH = 100;

// Data Structures
struct UserProfile {
    std::string username;
    std::string password;
    std::string email;
    std::string role; // admin, teacher, student
    std::string department;
    std::string join_date;
};

struct Course {
    int course_id;
    std::string name;
    std::string code;
    std::string department;
    int credits;
    std::string semester;
    std::string status; // active, inactive
};

struct Student {
    int student_id;
    std::string name;
    std::string email;
    std::string department;
    std::string enrollment_date;
    std::string status; // active, graduated, dropped
};

struct Teacher {
    int teacher_id;
    std::string name;
    std::string email;
    std::string department;
    std::string specialization;
    std::string hire_date;
};

struct DashboardStats {
    int total_students;
    int total_teachers;
    int total_courses;
    int active_courses;
};

// Main Application Window
class UniversitySystem : public Gtk::Window {
public:
    UniversitySystem();
    virtual ~UniversitySystem();

protected:
    // Widgets
    Gtk::Notebook main_notebook;
    Gtk::Box main_box;
    Gtk::HeaderBar header_bar;
    Gtk::MenuButton menu_button;
    Gtk::PopoverMenu menu_popover;
    Gtk::Box menu_box;
    
    // Menu Items
    Gtk::Button profile_button;
    Gtk::Button settings_button;
    Gtk::Button reports_button;
    Gtk::Button logout_button;

    // Dashboard Widgets
    Gtk::Box dashboard_box;
    Gtk::Label welcome_label;
    Gtk::Grid stats_grid;
    Gtk::TreeView recent_activities_view;

    // Database
    sqlite3* db;
    bool initialize_database();

    // Signal Handlers
    void on_menu_button_clicked();
    void on_profile_button_clicked();
    void on_settings_button_clicked();
    void on_reports_button_clicked();
    void on_logout_button_clicked();

    // Helper Functions
    void setup_window_styling();
    void create_dashboard();
    void update_dashboard_stats();
    void setup_menu_items();
};

UniversitySystem::UniversitySystem() {
    // Set window properties
    set_title("University Management System");
    set_default_size(1200, 800);
    
    // Initialize database
    if (!initialize_database()) {
        Gtk::MessageDialog dialog(*this, "Database Error", false, Gtk::MESSAGE_ERROR);
        dialog.set_secondary_text("Failed to initialize database. The application will exit.");
        dialog.run();
        exit(1);
    }

    // Setup main container
    main_box.set_orientation(Gtk::ORIENTATION_VERTICAL);
    add(main_box);

    // Setup header bar
    header_bar.set_show_close_button(true);
    header_bar.set_title("University Management System");
    set_titlebar(header_bar);

    // Setup menu button
    menu_button.set_image_from_icon_name("open-menu-symbolic");
    menu_button.set_popover(menu_popover);
    header_bar.pack_end(menu_button);

    // Setup menu items
    setup_menu_items();

    // Create dashboard
    create_dashboard();

    // Show all widgets
    show_all_children();
}

UniversitySystem::~UniversitySystem() {
    if (db) {
        sqlite3_close(db);
    }
}

bool UniversitySystem::initialize_database() {
    int rc = sqlite3_open("university.db", &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    // Create tables if they don't exist
    const char* create_tables_sql = 
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT UNIQUE NOT NULL,"
        "password TEXT NOT NULL,"
        "email TEXT UNIQUE NOT NULL,"
        "role TEXT NOT NULL,"
        "department TEXT,"
        "join_date TEXT);"

        "CREATE TABLE IF NOT EXISTS courses ("
        "course_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "code TEXT UNIQUE NOT NULL,"
        "department TEXT NOT NULL,"
        "credits INTEGER NOT NULL,"
        "semester TEXT NOT NULL,"
        "status TEXT NOT NULL);"

        "CREATE TABLE IF NOT EXISTS students ("
        "student_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "email TEXT UNIQUE NOT NULL,"
        "department TEXT NOT NULL,"
        "enrollment_date TEXT NOT NULL,"
        "status TEXT NOT NULL);"

        "CREATE TABLE IF NOT EXISTS teachers ("
        "teacher_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "email TEXT UNIQUE NOT NULL,"
        "department TEXT NOT NULL,"
        "specialization TEXT,"
        "hire_date TEXT NOT NULL);";

    char* err_msg = nullptr;
    rc = sqlite3_exec(db, create_tables_sql, nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << err_msg << std::endl;
        sqlite3_free(err_msg);
        return false;
    }

    return true;
}

void UniversitySystem::setup_menu_items() {
    menu_box.set_orientation(Gtk::ORIENTATION_VERTICAL);
    menu_box.set_spacing(5);
    menu_box.set_margin_start(10);
    menu_box.set_margin_end(10);
    menu_box.set_margin_top(10);
    menu_box.set_margin_bottom(10);

    profile_button.set_label("Profile");
    settings_button.set_label("Settings");
    reports_button.set_label("Reports");
    logout_button.set_label("Logout");

    menu_box.pack_start(profile_button, Gtk::PACK_SHRINK);
    menu_box.pack_start(settings_button, Gtk::PACK_SHRINK);
    menu_box.pack_start(reports_button, Gtk::PACK_SHRINK);
    menu_box.pack_start(logout_button, Gtk::PACK_SHRINK);

    menu_popover.add(menu_box);

    // Connect signals
    profile_button.signal_clicked().connect(sigc::mem_fun(*this, &UniversitySystem::on_profile_button_clicked));
    settings_button.signal_clicked().connect(sigc::mem_fun(*this, &UniversitySystem::on_settings_button_clicked));
    reports_button.signal_clicked().connect(sigc::mem_fun(*this, &UniversitySystem::on_reports_button_clicked));
    logout_button.signal_clicked().connect(sigc::mem_fun(*this, &UniversitySystem::on_logout_button_clicked));
}

void UniversitySystem::create_dashboard() {
    // Setup dashboard box
    dashboard_box.set_orientation(Gtk::ORIENTATION_VERTICAL);
    dashboard_box.set_spacing(10);
    dashboard_box.set_margin_start(10);
    dashboard_box.set_margin_end(10);
    dashboard_box.set_margin_top(10);
    dashboard_box.set_margin_bottom(10);

    // Welcome label
    welcome_label.set_markup("<span size='x-large' weight='bold'>Welcome to University Management System</span>");
    dashboard_box.pack_start(welcome_label, Gtk::PACK_SHRINK);

    // Stats grid
    stats_grid.set_row_spacing(10);
    stats_grid.set_column_spacing(10);
    stats_grid.set_margin_top(20);
    dashboard_box.pack_start(stats_grid, Gtk::PACK_SHRINK);

    // Recent activities view
    recent_activities_view.set_vexpand(true);
    dashboard_box.pack_start(recent_activities_view);

    main_box.pack_start(dashboard_box);
}

void UniversitySystem::update_dashboard_stats() {
    // TODO: Implement dashboard stats update
}

void UniversitySystem::on_menu_button_clicked() {
    menu_popover.show_all();
}

void UniversitySystem::on_profile_button_clicked() {
    // TODO: Implement profile dialog
}

void UniversitySystem::on_settings_button_clicked() {
    // TODO: Implement settings dialog
}

void UniversitySystem::on_reports_button_clicked() {
    // TODO: Implement reports dialog
}

void UniversitySystem::on_logout_button_clicked() {
    // TODO: Implement logout functionality
}

int main(int argc, char* argv[]) {
    auto app = Gtk::Application::create(argc, argv, "org.university.system");
    
    UniversitySystem window;
    return app->run(window);
}
