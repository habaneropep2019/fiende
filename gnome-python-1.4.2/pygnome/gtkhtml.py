import gtk; _gtk = gtk; del gtk
import _gtkhtml

HTML_STREAM_OK = 0
HTML_STREAM_ERROR = 1

class GtkHTML (_gtk.GtkLayout):
    get_type = _gtkhtml.gtk_html_get_type
    def __init__ (self, _obj=None):
        if _obj: self._o = _obj; return
        self._o = _gtkhtml.gtk_html_new ()

    def allow_selection (self, allow):
        _gtkhtml.gtk_html_allow_selection (self._o, allow)

    def select_word (self):
        _gtkhtml.gtk_html_select_word (self._o)

    def select_line (self):
        _gtkhtml.gtk_html_select_line (self._o)

    def begin (self):
        return _gtkhtml.gtk_html_begin (self._o)

    def begin_content (self, content_type):
        return _gtkhtml.gtk_html_begin_content (self._o, content_type)

    def write (self, handle, buf):
        _gtkhtml.gtk_html_write (self._o, handle, buf)

    def end (self, handle, status):
        _gtkhtml.gtk_html_end (self._o, handle, status)

    def load_empty (self):
        _gtkhtml.gtk_html_load_empty (self._o)

    def load_from_string (self, buf):
        _gtkhtml.gtk_html_load_from_string (self._o, buf)

    def get_title (self):
        return _gtkhtml.gtk_html_get_title (self._o)

    def jump_to_anchor (self, anchor):
        return _gtkhtml.gtk_html_jump_to_anchor (self._o, anchor)

    def set_default_background_color (self, colour):
        _gtkhtml.gtk_html_set_default_background_color (self._o, colour)

    def set_default_content_type (self, content_type):
        _gtkhtml.gtk_html_set_default_content_type (self._o, content_type)

_gtk._name2cls['GtkHTML'] = GtkHTML

class GtkHTMLEmbedded (_gtk.GtkBin):
    get_type = _gtkhtml.gtk_html_embedded_get_type
    def __init__ (self, classid='', name='', type='', data='', width=-1, height=-1, _obj=None):
        if _obj: self._o = _obj; return
        self._o = _gtkhtml.gtk_html_embedded_new (classid, name, type, data, width, height)

    def set_parameter (self, param, value):
        _gtkhtml.gtk_html_embedded_set_parameter (self._o, param, value)

    def get_parameter (self, param):
        return _gtkhtml.gtk_html_embedded_get_parameter (self._o, param)

    def set_descent (self, descent):
        _gtkhtml.gtk_html_embedded_set_descent (self._o, descent)

    def __getattr__ (self, attr):
        attrs = {
            'classid': _gtkhtml.gtk_html_embedded_get_object_classid,
            'name': _gtkhtml.gtk_html_embedded_get_object_name,
            'type': _gtkhtml.gtk_html_embedded_get_object_type,
            'data': _gtkhtml.gtk_html_embedded_get_object_data
        }
        if attrs.has_key(attr):
            return attrs[attr](self._o)
        return _gtk.GtkBin.__getattr__(self, attr)

    def __setattr__ (self, attr, value):
        attrs = {
            'classid': _gtkhtml.gtk_html_embedded_set_object_classid,
            'name': _gtkhtml.gtk_html_embedded_set_object_name,
            'type': _gtkhtml.gtk_html_embedded_set_object_type,
            'data': _gtkhtml.gtk_html_embedded_set_object_data
        }
        if attrs.has_key(attr):
            attrs[attr](self._o, value)
        else:
            self.__dict__[attr] = value

    def __delattr__ (self, attr):
        attrs = {
            'classid': _gtkhtml.gtk_html_embedded_set_object_classid,
            'name': _gtkhtml.gtk_html_embedded_set_object_name,
            'type': _gtkhtml.gtk_html_embedded_set_object_type,
            'data': _gtkhtml.gtk_html_embedded_set_object_data
        }
        if attrs.has_key(attr):
            attrs[attr](self._o, None)
        else:
            del self.__dict__[attr]
        
_gtk._name2cls['GtkHTMLEmbedded'] = GtkHTMLEmbedded

