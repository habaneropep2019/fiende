import gtk; _gtk = gtk; del gtk
import _zvt

_obj2inst = _gtk._obj2inst

# selection types for get_buffer ...
SELTYPE_NONE  = 0
SELTYPE_CHAR  = 1
SELTYPE_WORD  = 2
SELTYPE_LINE  = 3
SELTYPE_MAGIC = 4

class ZvtTerm(_gtk.GtkWidget):
	get_type = _zvt.zvt_term_get_type
	def __init__(self, cols=-1, rows=-1, _obj=None):
		if _obj: self._o = _obj; return
		if cols != -1 and rows != -1:
			self._o = _zvt.zvt_term_new_with_size(cols, rows)
		else:
			self._o = _zvt.zvt_term_new()
	def __getattr__(self, attr):
		if attr == 'adjustment':
			return _gtk.GtkAdjustment(_obj=
					_zvt.zvt_term_get_adjustment(self._o))
		elif attr == 'charwidth':
			return _zvt.zvt_term_get_charwidth(self._o)
		elif attr == 'charheight':
			return _zvt.zvt_term_get_charheight(self._o)
		return _gtk.GtkWidget.__getattr__(self, attr)
	def reset(self, hard):
		_zvt.zvt_term_reset(self._o, hard)
	def feed(self, text):
		_zvt.zvt_term_feed(self._o, text)
	def forkpty(self, do_uwtmp_log=_gtk.FALSE):
		return _zvt.zvt_term_forkpty(self._o, do_uwtmp_log)
	def closepty(self):
		return _zvt.zvt_term_closepty(self._o)
	def killchild(self, signal):
		return _zvt.zvt_term_killchild(self._o, signal)
	def bell(self):
		_zvt.zvt_term_bell(self._o)
	def set_scrollback(self, lines):
		_zvt.zvt_term_set_scrollback(self._o, lines)
	def get_buffer(self, type, sx,sy, ex,ey):
		return _zvt.zvt_term_get_buffer(self._o, type, sx,sy, ex,ey)
	def set_font_name(self, name):
		_zvt.zvt_term_set_font_name(self._o, name)
	def set_fonts(self, font, font_bold):
		_zvt.zvt_term_set_fonts(self._o, font, font_bold)
	def hide_pointer(self):
		_zvt.zvt_term_hide_pointer(self._o)
	def show_pointer(self):
		_zvt.zvt_term_show_pointer(self._o)
	def set_bell(self, state):
		_zvt.zvt_term_set_bell(self._o, state)
	def get_bell(self):
		return _zvt.zvt_term_get_bell(self._o)
	def set_blink(self, state):
		_zvt.zvt_term_set_blink(self._o, state)
	def set_scroll_on_keystroke(self, state):
		_zvt.zvt_term_set_scroll_on_keystroke(self._o, state)
	def set_scroll_on_output(self, state):
		_zvt.zvt_term_set_scroll_on_output(self._o, state)
	# color_list is a list of 18 (r,g,b) 3-tuples
	def set_color_scheme(self, color_list):
		_zvt.zvt_term_set_color_scheme(self._o, color_list)
	def set_default_color_scheme(self):
		_zvt.zvt_term_set_default_color_scheme(self._o)
	def set_del_key_swap(self, state):
		_zvt.zvt_term_set_del_key_swap(self._o, state)
	def set_wordclass(self, wordclass):
		_zvt.zvt_term_set_wordclass(self._o, wordclass)
	def match_add(self, regex, highlight_mask):
		return _zvt.zvt_term_match_add(self._o, regex,
					       highlight_mask, None)
	def match_clear(self, regex):
		_zvt.zvt_term_match_clear(self._o, regex)
	def set_background(self, pixmap_file, transparent=_gtk.FALSE,
			   shaded=_gtk.FALSE):
		_zvt.zvt_term_set_background(self._o, pixmap_file, transparent,
					     shaded)
	def set_shadow_type(self, shadow_type):
		_zvt.zvt_term_set_shadow_type(self._o, shadow_type)
	def set_size(self, width, height):
		_zvt.zvt_term_set_size(self._o, width, height)
_gtk._name2cls['ZvtTerm'] = ZvtTerm
