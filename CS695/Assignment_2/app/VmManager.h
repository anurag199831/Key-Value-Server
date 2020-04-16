#include <gtkmm/box.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/window.h>

class VmManager : public Gtk::Window {
   private:
	void _updateViews();
	void __getBoxWithWidgets(Gtk::Box& box);

   public:
	VmManager();
	virtual ~VmManager();
	void debug();

   protected:
	Gtk::Box m_box1;
	Gtk::Grid m_grid1;
	Gtk::Label mLabelMain;
	std::vector<Gtk::Box> mBoxVec;
};
