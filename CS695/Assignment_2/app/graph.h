#include <glibmm/main.h>
#include <gtkmm/drawingarea.h>

#include <vector>

class Graph : public Gtk::DrawingArea {
   private:
	std::vector<int> mCurrState;
	std::vector<int> mPrevState;
	enum { DRAW_HOLD, DRAW_START } mState;

	const int TIMEOUT_INTERVAL_IN_MILLIS = 1000;

   protected:
	bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
	bool on_timeout();

   public:
	Graph();
	~Graph();
	void setVectorToDraw(const std::vector<int> vec);
};

Graph::Graph() {
	Glib::signal_timeout().connect(sigc::mem_fun(*this, &Graph::on_timeout),
								   TIMEOUT_INTERVAL_IN_MILLIS);
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
	std::vector<int> vecToDraw;
	if (mState == DRAW_HOLD) {
		vecToDraw = mPrevState;
	} else if (mState == DRAW_START) {
		vecToDraw = mCurrState;
	}

	Gtk::Allocation allocation = get_allocation();
	const int width = allocation.get_width();
	const int height = allocation.get_height();

	const int start_width = width / 101;
	const int start_height = 5 * height / 110;
	const int bin_width = width / vecToDraw.size() + 1;
	const int bin_height = height / 110;

	get_window()->freeze_updates();
	cr->save();
	cr->set_line_width(2.0);
	cr->set_source_rgb(0.8, 0.0, 0.0);

	for (size_t i = 0; i < vecToDraw.size(); i++) {
		cr->line_to(start_width + bin_width * i,
					height - (start_height + bin_height * vecToDraw.at(i)));
	}

	cr->stroke();
	cr->restore();
	get_window()->thaw_updates();
	return true;
}

void Graph::setVectorToDraw(const std::vector<int> vec) {
	mPrevState = std::move(mCurrState);
	mCurrState = std::move(vec);
	mState = DRAW_START;
}