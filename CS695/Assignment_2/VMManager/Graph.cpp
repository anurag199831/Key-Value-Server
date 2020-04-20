//
// Created by pranav on 18/04/20.
//

#include "Graph.h"

#include <glibmm/main.h>

#include <iostream>

Graph::Graph() : mState(DRAW_HOLD) {
	Glib::signal_timeout().connect(sigc::mem_fun(*this, &Graph::on_timeout),
								   TIMEOUT_INTERVAL_IN_MILLIS);
}

Graph::~Graph() = default;

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
	std::vector<int> vecToDraw = {};
	if (mState == DRAW_HOLD) {
		std::cout << "Graph::on_draw: Hold flag set" << std::endl;
		vecToDraw = mPrevState;
	} else if (mState == DRAW_START) {
		std::cout << "Graph::on_draw: Draw flag set" << std::endl;
		vecToDraw = mCurrState;
	} else if (mState == DRAW_CLEAR) {
		std::cout << "Graph::on_draw: Clear flag set" << std::endl;
	}

	Gtk::Allocation allocation = get_allocation();
	const int width = allocation.get_width();
	const int height = allocation.get_height();

	const double start_width = static_cast<double>(width) / 101;
	const double start_height = static_cast<double>(5 * height) / 110;
	const double bin_width = static_cast<double>(width) / MAX_TICKS_ON_GRAPH;
	const double bin_height = static_cast<double>(height) / 110;

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
	mState = DRAW_HOLD;
	return true;
}

void Graph::setVectorToDraw(const std::vector<int>& vec) {
	mPrevState = std::move(mCurrState);
	mCurrState = vec;
	mState = DRAW_START;
}
void Graph::clear() { mState = DRAW_CLEAR; }
