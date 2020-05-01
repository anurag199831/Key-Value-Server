#include <gtkmm/application.h>

#include "VmManager.h"

int main(int argc, char *argv[]) {
	auto app = Gtk::Application::create(argc, argv, "org.iitb.cse.pranav");

	VmManager window;
	Gtk::Allocation alloc;

	alloc.set_width(1200);
	alloc.set_height(720);
	window.set_allocation(alloc);
	// Shows the window and returns when it is closed.
	return app->run(window);
}