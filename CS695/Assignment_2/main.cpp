#include <gtkmm/application.h>

#include "VMManager/VmManager.h"

int main(int argc, char *argv[]) {
	auto app = Gtk::Application::create(argc, argv, "org.iitb.cse.pranav");

	VmManager window;
	Gtk::Allocation alloc;

	alloc.set_width(1000);
	alloc.set_height(500);
	window.set_allocation(alloc);
	// Shows the window and returns when it is closed.
	return app->run(window);
}