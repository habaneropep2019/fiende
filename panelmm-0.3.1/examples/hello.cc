#include <panel--.h>
#include <gnome--.h>

using namespace Gnome;
using namespace Gtk;

class Hello_Applet: public Applet
{
    void callback();
public:
    Hello_Applet();
};

Hello_Applet::Hello_Applet():
    Applet("hello_applet")
{
    Button* button = new Button("Hello world!");
    button->clicked.connect(slot(this, &Hello_Applet::callback));
    add(*manage(button));

    add_menuitem("test", "Greet me", slot(this, &Hello_Applet::callback));
}

void Hello_Applet::callback()
{
    Gnome::Dialogs::ok ("Hello!");
}

int main(int argc, char **argv)
{
    Applet_Main m("hello_applet", "1.0", argc, argv);
    Hello_Applet a;

    a.show_all();
    m.run();
}
