#include "VmManager.h"

#include <gtkmm/box.h>

#include <iostream>

#include "graph.h"

VmManager::VmManager() {
	// This just sets the title of our new window.
	set_title("VM Manager");

	// sets the border width of the window.
	set_border_width(10);
	// put the box into the main window.
	add(m_box1);

	m_box1.set_orientation(Gtk::Orientation::ORIENTATION_VERTICAL);

	mLabelMain.set_text("Virtual Machine Manager");

	m_box1.pack_start(mLabelMain, Gtk::PACK_SHRINK, 20);
	m_box1.pack_start(m_grid1, Gtk::PACK_EXPAND_WIDGET, 20);

	m_grid1.set_row_spacing(20);
	m_grid1.set_column_spacing(20);

	m_grid1.set_row_homogeneous(true);
	m_grid1.set_column_homogeneous(true);
	m_grid1.set_border_width(10);
	_updateViews();
	show_all_children();
}

VmManager::~VmManager() {}

void VmManager::_updateViews() {
	for (size_t i = 0; i < 1; i++) {
		mBoxVec.emplace_back();
		__getBoxWithWidgets(mBoxVec.at(i));
		m_grid1.attach(mBoxVec.at(i), i % 2, i / 2);
	}
}

void VmManager::__getBoxWithWidgets(Gtk::Box &box) {
	Gtk::Label *lbl = Gtk::make_managed<Gtk::Label>("Testing");
	Graph *draw = Gtk::make_managed<Graph>();
	Gtk::Allocation alloc;
	alloc.set_width(150);
	alloc.set_height(50);
	draw->size_allocate(alloc);
	box.set_orientation(Gtk::Orientation::ORIENTATION_VERTICAL);
	box.pack_start(*lbl, Gtk::PACK_SHRINK);
	box.pack_start(*draw);
}