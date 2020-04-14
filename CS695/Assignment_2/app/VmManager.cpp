#include "VmManager.h"

#include <iostream>

VmManager::VmManager() {
	// This just sets the title of our new window.
	set_title("VM Manager");

	// sets the border width of the window.
	set_border_width(10);
	// put the box into the main window.
	add(m_box1);

	m_box1.set_orientation(Gtk::Orientation::ORIENTATION_VERTICAL);

	mLabelMain.set_text("Virtual Machine Manager");

	m_box1.pack_start(mLabelMain, Gtk::PACK_EXPAND_WIDGET, 20);
	m_box1.pack_start(m_grid1, Gtk::PACK_EXPAND_WIDGET, 20);

	m_grid1.set_row_spacing(20);
	m_grid1.set_column_spacing(20);

	m_grid1.set_row_homogeneous(true);
	m_grid1.set_column_homogeneous(true);
	m_grid1.set_border_width(10);
	updateViews();
	show_all_children();
}

VmManager::~VmManager() {}

void VmManager::updateViews() {
	auto numberOfdomains = mgr.numActiveDomains();
	for (size_t i = 0; i < numberOfdomains; i++) {
		mBoxVec.emplace_back();
	}
}