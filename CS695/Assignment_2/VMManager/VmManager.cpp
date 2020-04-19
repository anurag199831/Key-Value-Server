//
// Created by pranav on 18/04/20.
//

#include "VmManager.h"

#include <iostream>

#include "Graph.h"

VmManager::VmManager() {
  add(m_box1);

  m_box1.set_orientation(Gtk::Orientation::ORIENTATION_VERTICAL);

  auto mLabelMain = Gtk::make_managed<Gtk::Label>("Virtual Manager");
  mLabelMain->set_text("Virtual Machine Manager");

  m_box1.pack_start(*mLabelMain, Gtk::PACK_SHRINK);
  m_box1.pack_start(m_grid1, Gtk::PACK_EXPAND_WIDGET, 20);

  m_grid1.set_row_spacing(50);
  m_grid1.set_column_spacing(50);

  m_grid1.set_row_homogeneous(true);
  m_grid1.set_column_homogeneous(true);
  m_grid1.set_border_width(50);
  _createViews();
  show_all_children();
}

VmManager::~VmManager() = default;

void VmManager::_createViews() {
  auto namesOfVM = mgr.getAllDefinedDomainNames();
  string name;

  for (size_t i = 0; i < namesOfVM.size(); i++) {
	name = namesOfVM.at(i);

	terminationFlags.insert(make_pair(name, false));
	terminationMutexes.insert(make_pair(name, new mutex()));

	auto outerBox = Gtk::make_managed<Gtk::Box>();

	__getBoxWithWidgets(outerBox);
	__fillBoxWithName(outerBox, namesOfVM.at(i));
	__fillBoxWithIP(outerBox, namesOfVM.at(i));
	__setButtonsInBox(outerBox, namesOfVM.at(i));

	m_grid1.attach(*outerBox, i % 2, i / 2);
  }
}

void VmManager::__getBoxWithWidgets(Gtk::Box *box) {
  box->set_orientation(Gtk::Orientation::ORIENTATION_VERTICAL);
  box->set_spacing(5);

  auto lbl = Gtk::make_managed<Gtk::Label>();
  auto draw = Gtk::make_managed<Graph>();
  auto ip = Gtk::make_managed<Gtk::Label>();
  auto onButton = Gtk::make_managed<Gtk::Button>();
  auto offButton = Gtk::make_managed<Gtk::Button>();

  Gtk::Allocation alloc;
  alloc.set_width(150);
  alloc.set_height(50);
  draw->size_allocate(alloc);

  box->pack_start(*lbl, Gtk::PACK_SHRINK);
  box->pack_start(*draw);
  box->pack_start(*ip, Gtk::PACK_SHRINK);
  box->pack_start(*onButton, Gtk::PACK_SHRINK);
  box->pack_start(*offButton, Gtk::PACK_SHRINK);
}

void VmManager::__fillBoxWithName(Gtk::Box *box, const string &nameOfVM) {
  auto children = box->get_children();
  Gtk::Widget *name = children.at(0);
  if (GTK_IS_LABEL(name->gobj())) {
	auto label = dynamic_cast<Gtk::Label *>(name);
	label->set_text(nameOfVM);
  } else {
	std::cerr << "VmManager::__fillBoxWithName: label not found as first "
				 "element of the box"
			  << std::endl;
  }
}

void VmManager::__fillBoxWithIP(Gtk::Box *box, const string &nameOfVM) {
  auto children = box->get_children();
  string ip;
  if (mgr.isVmPowered(nameOfVM)) {
	ip = mgr.getIP(nameOfVM);
  } else {
	ip = "--";
  }
  if (ip.empty()) { ip = "--"; }

  Gtk::Widget *name = children.at(2);
  if (GTK_IS_LABEL(name->gobj())) {
	auto label = dynamic_cast<Gtk::Label *>(name);
	label->set_text("IP: " + ip);
  } else {
	std::cerr << "VmManager::__fillBoxWithIP: label not found as third element "
				 "of the box"
			  << std::endl;
  }
}

void VmManager::__setButtonsInBox(Gtk::Box *box, const string &nameOfVM) {
  auto children = box->get_children();
  Gtk::Widget *startButton = children.at(3);
  Gtk::Widget *shutButton = children.at(4);
  if (GTK_IS_BUTTON(startButton->gobj()) and
	  GTK_IS_BUTTON(shutButton->gobj())) {
	auto start = dynamic_cast<Gtk::Button *>(startButton);
	auto shut = dynamic_cast<Gtk::Button *>(shutButton);
	start->add_label("Start Server");
	shut->add_label("Shutdown Server");

	start->set_sensitive(true);
	shut->set_sensitive(false);

	start->signal_clicked().connect(
		sigc::bind<Glib::ustring, Gtk::Box *, Gtk::Button *, Gtk::Button *>(
			sigc::mem_fun(*this, &VmManager::on_start_button_clicked), nameOfVM,
			box, start, shut));
	shut->signal_clicked().connect(
		sigc::bind<Glib::ustring, Gtk::Box *, Gtk::Button *, Gtk::Button *>(
			sigc::mem_fun(*this, &VmManager::on_shut_button_clicked), nameOfVM,
			box, start, shut));
  } else {
	std::cerr << "VmManager::__setStartButtonInBox: button not found as fourth "
				 "element of the box"
			  << std::endl;
  }
}


// Signal Handlers

void VmManager::on_start_button_clicked(const Glib::ustring &name,
										Gtk::Box *box, Gtk::Button *startButton,
										Gtk::Button *shutButton) {
  startButton->set_sensitive(false);
  shutButton->set_sensitive(true);
  auto launcherThread = new thread([this, name]() {
	mgr.startNewVm(name);
	mgr.powerOn(name);
	mgr.startWatching(name);
  });
  auto it = terminationFlags.find(name);
  if (it != terminationFlags.end()) {
	it->second = false;
  } else {
	std::cerr << "VmManager::on_start_button_clicked: no flag found for "
			  << name << std::endl;
  }
  launchThreads.insert(std::make_pair(std::string(name), launcherThread));

  auto ipUpdaterThread = new thread([this, box, name] {
	while (not terminationFlags.at(name)) {
	  {
		std::lock_guard lck(*terminationMutexes.at(name));
		if (terminationFlags.at(name)) {
		  break;
		} else {
		  __fillBoxWithIP(box, name);
		}
	  }
	  this_thread::sleep_for(chrono::seconds(2));
	}
  });
  ipUpdaterThreads.insert(make_pair(name, ipUpdaterThread));
}

void VmManager::on_shut_button_clicked(const Glib::ustring &name, Gtk::Box *box,
									   Gtk::Button *startButton,
									   Gtk::Button *shutButton) {
  auto itm = terminationMutexes.find(name);
  if (itm == terminationMutexes.end()) {
	std::cerr << "VmManager::on_shut_button_clicked: no mutex found for "
			  << name << std::endl;
	return;
  }

  std::lock_guard lck(*itm->second);

  auto it = terminationFlags.find(name);
  if (it != terminationFlags.end()) {
	it->second = true;
  } else {
	std::cerr << "VmManager::on_shut_button_clicked: no flag found for " << name
			  << std::endl;
  }

  mgr.shutdown(name);

  auto th = ipUpdaterThreads.find(name);
  if (th != ipUpdaterThreads.end()) {
	if (th->second != nullptr and th->second->joinable()) {
	  th->second->join();
	  delete th->second;
	  ipUpdaterThreads.erase(name);
	  shutButton->set_sensitive(false);
	  startButton->set_sensitive(true);
	}
  } else {
	std::cerr
		<< "VmManager::on_shut_button_clicked: no ipUpdater thread found for "
		<< name << std::endl;
  }
}
