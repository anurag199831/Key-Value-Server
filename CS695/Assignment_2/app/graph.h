#include <glibmm/main.h>
#include <gtkmm/drawingarea.h>
#include <manager.cpp>

class Graph : public Gtk::DrawingArea {
   private:
    std::string name;
    Manager* mgr;

   protected:
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
    bool on_timeout();

   public:
    Graph();
    Graph(Manager* mgr);
    ~Graph();
    void set_name(const string name) { this->name = std::move(name); }
};

Graph::Graph() {}
Graph::Graph(Manager* mgr) {
    this->mgr = mgr;
    Glib::signal_timeout().connect(sigc::mem_fun(*this, &Graph::on_timeout),
                                   1500);
}

Graph::~Graph() {}

bool Graph::on_timeout() {
    // force our program to redraw the entire clock.
    auto win = get_window();
    if (win) {
        Gdk::Rectangle r(0, 0, get_allocation().get_width(),
                         get_allocation().get_height());
        win->invalidate_rect(r, false);
    }
    return true;
}

bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) {}