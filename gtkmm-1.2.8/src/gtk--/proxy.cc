#include <gtk--/proxy.h>
#include <gtk--/object.h>

namespace Gtk
{

ProxyNode::~ProxyNode()
  {
    GtkObject *obj2=obj_;
    obj_=0;
    if (obj2&&!GTK_OBJECT_DESTROYED(obj2))
      gtk_signal_disconnect(obj2,connid_);
  }

void ProxyNode::notify(gpointer data)
  {
    // gtk+ has already disconnected us
    ProxyNode *node=(ProxyNode*)data;
    if (!node->obj_) return;
    node->obj_=0;
    node->slot_->invalid();
  }

void ProxyNode::connect(Gtk::Object* obj,
                            const char* name,
                            GtkSignalFunc func,
                            SigC::SlotData* sd,
                            bool after)
  {
    g_return_if_fail (obj!=0);
    g_return_if_fail (sd!=0);

    GtkObject* gtk_obj=GTK_OBJECT(obj->gtkobj());

    ProxyNode *node=new ProxyNode();
    node->obj_=gtk_obj;
    node->connid_=gtk_signal_connect_full(gtk_obj, name, func,
                                       (GtkCallbackMarshal)NULL,
                                       (gpointer)node,
                                       &notify,0,after);
    node->slot_=sd;
    node->callback_=&(sd->data_);

    //obj->register_data(sd->sender());
    sd->connect();

    // register disconnection data
    SigC::SlotList_ &l=sd->list_;
    l.insert_direct(l.begin(), node);
  }

}
