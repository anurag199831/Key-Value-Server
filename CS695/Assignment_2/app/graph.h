#include <glibmm/main.h>
#include <gtkmm/drawingarea.h>

#include <vector>

class Graph : public Gtk::DrawingArea {
   private:
	std::string name;
	enum { DRAW_HOLD, DRAW_START } state;

   protected:
	bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
	bool on_timeout();

   public:
	Graph();
	~Graph();
	void set_name(const std::string name) { this->name = std::move(name); }
};

Graph::Graph() {
	Glib::signal_timeout().connect(sigc::mem_fun(*this, &Graph::on_timeout),
								   1000);
}

Graph::~Graph() {}

bool Graph::on_timeout() {
	auto win = get_window();
	if (win) {
		Gdk::Rectangle r(0, 0, get_allocation().get_width(),
						 get_allocation().get_height());
		win->invalidate_rect(r, false);
	}
	return true;
}

bool Graph::on_draw(const Cairo::RefPtr<Cairo::Context>& cr) {
	// if (state == DRAW_HOLD) return true;

	std::vector<int> a;
	for (size_t i = 0; i < 40; i++) {
		a.push_back(rand() % 101);
	}

	Gtk::Allocation allocation = get_allocation();
	const int width = allocation.get_width();
	const int height = allocation.get_height();

	// coordinates for the center of the window
	const int start_width = width / 101;
	const int start_height = 5 * height / 110;
	const int bin_width = width / a.size() + 1;
	const int bin_height = height / 110;
	get_window()->freeze_updates();
	cr->save();
	cr->set_line_width(2.0);
	cr->set_source_rgb(0.8, 0.0, 0.0);

	for (size_t i = 0; i < a.size(); i++) {
		cr->line_to(start_width + bin_width * i,
					height - (start_height + bin_height * a.at(i)));
	}

	cr->stroke();
	cr->restore();
	get_window()->thaw_updates();
	return true;
}