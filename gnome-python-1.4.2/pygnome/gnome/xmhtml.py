import gtk; _gtk = gtk; del gtk
import _gtkxmhtml

class XmHTMLCallbackStruct:
	def __init__(self, cobject):
		for key, value in _gtkxmhtml.cobject_to_info(cobject).items():
			setattr(self, key, value)
		if hasattr(self, 'html'):
			self.html = _gtk._obj2inst(self.html)
_XmHTMLCallbackStruct = XmHTMLCallbackStruct
del XmHTMLCallbackStruct

class GtkXmHTML(_gtk.GtkContainer):
	get_type = _gtkxmhtml.gtk_xmhtml_get_type
	def __init__(self, _obj=None):
		if _obj: self._o = _obj; return
		self._o = _gtkxmhtml.gtk_xmhtml_new()
	class __cnv:
		def __init__(self, func):
			self.func = func
		def __call__(self, *args):
			return apply(self.func, (_gtk._obj2inst(args[0]),
					_XmHTMLCallbackStruct(args[1])))
	def connect(self, sig_name, handler):
		if sig_name in ("activate", "arm", "anchor_track", "frame",
				"form", "input", "link", "motion", "imagemap",
				"document", "focus", "losing_focus",
				"motion_track", "html_event"):
			callback = self.__cnv(handler)
			return _gtk._gtk.gtk_signal_connect(self._o, sig_name,
							    callback.__call__)
		else:
			_gtk.GtkContainer.connect(self, sig_name, handler)
	signal_connect = connect
	def connect_after(self, sig_name, handler):
		if sig_name in ("activate", "arm", "anchor_track", "frame",
				"form", "input", "link", "motion", "imagemap",
				"document", "focus", "losing_focus",
				"motion_track", "html_event"):
			callback = self.__cnv(handler)
			return _gtk._gtk.gtk_signal_connect_after(self._o,
								  sig_name,
							callback.__call__)
		else:
			_gtk.GtkContainer.connect_after(self, sig_name,
							     handler)
	signal_connect_after = connect_after
	def freeze(self, obj=None):
		_gtkxmhtml.gtk_xmhtml_freeze(self._o)
	def thaw(self, obj=None):
		_gtkxmhtml.gtk_xmhtml_thaw(self._o)
	def source(self, source):
		_gtkxmhtml.gtk_xmhtml_source(self._o, source)
	def set_string_direction(self, direction):
		_gtkxmhtml.gtk_xmhtml_set_string_direction(self._o, direction)
	def set_alignment(self, alignment):
		_gtkxmhtml.gtk_xmhtml_set_alignment(self._o, alignment)
	#def outline(self, flag):
	#	_gtkxmhtml.gtk_xmhtml_outline(self._o, flag)
	# these are spelt wrong in the library ...
	def set_font_family(self, family, sizes):
		_gtkxmhtml.gtk_xmhtml_set_font_familty(self._o, family, sizes)
	def set_font_family_fixed(self, family, sizes):
		_gtkxmhtml.gtk_xmhtml_set_font_familty_fixed(self._o, family,
							    sizes)
	def set_font_charset(self, charset):
		_gtkxmhtml.gtk_xmhtml_set_font_charset(self._o, charset)
	def set_allow_body_colors(self, enable):
		_gtkxmhtml.gtk_xmhtml_set_allow_body_colors(self._o, enable)
	def set_hilight_on_enter(self, flag):
		_gtkxmhtml.gtk_xmhtml_set_hilight_on_enter(self._o, flag)
	def set_anchor_visited_underline_type(self, underline_type):
		_gtkxmhtml.gtk_xmhtml_set_anchor_visited_underline_type(
			self._o, underline_type)
	def set_anchor_target_underline_type(self, underline_type):
		_gtkxmhtml.gtk_xmhtml_set_anchor_target_underline_type(self._o,
							underline_type)
	def set_allow_color_switching(self, flag):
		_gtkxmhtml.gtk_xmhtml_set_allow_color_switching(self._o, flag)
	def set_dithering(self, dither_type):
		_gtkxmhtml.gtk_xmhtml_set_dithering(self._o, dither_type)
	def set_allow_font_switching(self, flag):
		_gtkxmhtml.gtk_xmhtml_set_allow_font_switching(self._o, flag)
	def set_max_image_colors(self, max_colors):
		_gtkxmhtml.gtk_xmhtml_set_max_image_colors(self._o, max_colors)
	def set_allow_images(self, flag):
		_gtkxmhtml.gtk_xmhtml_set_allow_images(self._o, flag)
	def set_plc_intervals(self, min_delay, max_delay, def_delay):
		_gtkxmhtml.gtk_xmhtml_set_plc_intervals(self._o, min_delay,
							max_delay, def_delay)
	def set_def_body_image_url(self, url):
		_gtkxmhtml.gtk_xmhtml_set_def_body_image_url(self._o, url)
	def set_anchor_buttons(self, flag):
		_gtkxmhtml.gtk_xmhtml_set_anchor_buttons(self._o, flag)
	def set_anchor_cursor(self, cursor, flag):
		_gtkxmhtml.gtk_xmhtml_set_anchor_cursor(self._o, cursor, flag)
	def set_topline(self, line):
		_gtkxmhtml.gtk_xmhtml_set_topline(self._o, line)
	def get_topline(self):
		return _gtkxmhtml.gtk_xmhtml_get_topline(self._o)
	def set_freeze_animations(self, flag):
		_gtkxmhtml.gtk_xmhtml_set_freeze_animations(self._o, flag)
	#def get_source(self):
	#	return _gtkxmhtml.gtk_xmhtml_get_source(self._o)
	def set_screen_gamma(self, gamma):
		_gtkxmhtml.gtk_xmhtml_set_screen_gamma(self._o, gamma)
	def set_perfect_colors(self, flag):
		_gtkxmhtml.gtk_xmhtml_set_perfect_colors(self._o, flag)
	def set_uncompress_command(self, cmd):
		_gtkxmhtml.gtk_xmhtml_set_uncompress_command(self._o, cmd)
	def set_strict_checking(self, flag):
		_gtkxmhtml.gtk_xmhtml_set_strict_checking(self._o, flag)
	def set_bad_html_warnings(self, flag):
		_gtkxmhtml.gtk_xmhtml_set_bad_html_warnings(self._o, flag)
	def set_allow_form_coloring(self, flag):
		_gtkxmhtml.gtk_xmhtml_set_allow_form_coloring(self._o, flag)
	def set_imagemap_draw(self, flag):
		_gtkxmhtml.gtk_xmhtml_set_imagemap_draw(self._o, flag)
	def set_mime_type(self, mime_type):
		_gtkxmhtml.gtk_xmhtml_set_mime_type(self._o, mime_type)
	def set_alpha_processing(self, flag):
		_gtkxmhtml.gtk_xmhtml_set_alpha_processing(self._o, flag)
	def set_rgb_conv_mode(self, val):
		_gtkxmhtml.gtk_xmhtml_set_rgb_conv_mode(self._o, val)

# this adds the GtkXmHTML class to the low level to class conversion hash:
_gtk._name2cls['GtkXmHTML'] = GtkXmHTML
