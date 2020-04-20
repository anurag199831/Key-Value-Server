//
// Created by pranav on 18/04/20.
//

#ifndef ASSIGNMENT_2_VMMANAGER_H
#define ASSIGNMENT_2_VMMANAGER_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/window.h>

#include <condition_variable>
#include <thread>
#include <unordered_map>

#include "../manager/Manager.h"

class VmManager : public Gtk::Window {
   private:
	Manager *mgr;

	unordered_map<string, thread *> launchThreads;
	unordered_map<string, thread *> ipUpdaterThreads;
	unordered_map<string, thread *> drawingThreads;

	unordered_map<string, bool> terminationFlags;
	unordered_map<string, mutex *> terminationMutexes;

	thread* sanitiserThread;
	mutex sanitizerMutex;
	condition_variable sanitizeVariable;
	bool sanitizerThreadTerminationFlag;

	void _fillViewsInGrid(Gtk::Grid *grid);

	static void _getBoxWithWidgets(Gtk::Box *box);

	static void _fillBoxWithName(Gtk::Box *box, const string &nameOfVM);

	void _fillBoxWithIP(Gtk::Box *box, const string &nameOfVM);

	void _drawGraphInBox(Gtk::Box *box, const string& nameOfVm,bool clear);

	void _setButtonsInBox(Gtk::Box *box, const string &nameOfVM);

   public:
	VmManager();
	~VmManager() override;

   protected:
	Gtk::Box m_box1;


	// Signal handlers:
	void on_start_button_clicked(const Glib::ustring &name, Gtk::Box *box,
								 Gtk::Button *start, Gtk::Button *shut);
	void on_shut_button_clicked(const Glib::ustring &name, Gtk::Button *start,
								Gtk::Button *shut);
};

#endif	// ASSIGNMENT_2_VMMANAGER_H
