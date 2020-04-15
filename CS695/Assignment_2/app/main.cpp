#include <gtkmm/application.h>

#include "VmManager.cpp"

int main(int argc, char *argv[]) {
    auto app = Gtk::Application::create(argc, argv, "org.iitb.cse.pranav");

    VmManager window;
    // Shows the window and returns when it is closed.
    return app->run(window);
}