import sys; _sys = sys; del sys
import gtk; _gtk = gtk; del gtk
import _gnomeui, gnome

if not gnome.gnome_init_called:
	_gnomeui.gnome_init(gnome.app_id, gnome.app_version)
	_gnomeui.gtk_ted_set_app_name(gnome.app_id)
	gnome.gnome_init_called = 1
	# gnome_init calls gnomelib_init
	gnome.gnomelib_init_called = 1
del gnome

# lets get our constants ...
from uiconsts import *

_obj2inst = _gtk._obj2inst
_filtprops = _gtk._filtprops

class GnomeDialog(_gtk.GtkWindow):
	get_type = _gnomeui.gnome_dialog_get_type
	def __init__(self, title='', b1=None, b2=None, b3=None, b4=None,
		     b5=None, b6=None, b7=None, b8=None, b9=None, b10=None,
		     _obj=None):
		if _obj: self._o = _obj; return
		buttons = filter(lambda x: x, (b1,b2,b3,b4,b5,b6,b7,b8,b9,b10))
		self._o = apply(_gnomeui.gnome_dialog_new, (title,) + buttons)
	def __getattr__(self, attr):
		attrs = {
			"vbox": _gnomeui.gnome_dialog_get_vbox,
		}
		if attrs.has_key(attr):
			return _obj2inst(attrs[attr](self._o))
		return _gtk.GtkWindow.__getattr__(self, attr)
	def set_parent(self, parent):
		_gnomeui.gnome_dialog_set_parent(self._o, parent._o)
	def run(self):
		return _gnomeui.gnome_dialog_run(self._o)
	def run_and_close(self):
		return _gnomeui.gnome_dialog_run_and_close(self._o)
	def button_connect(self, button, callback):
		_gnomeui.gnome_dialog_button_connect(self._o, button, callback)
	def set_default(self, button):
		_gnomeui.gnome_dialog_set_default(self._o, button)
	def set_sensitive(self, button, setting):
		_gnomeui.gnome_dialog_set_sensitive(self._o, button, setting)
	def set_destroy(self, self_destruct):
		print "Deprecated -- use set_close"
		_gnomeui.gnome_dialog_set_destroy(self._o, self_destruct)
	def set_accelerator(self, button, ac_key, ac_mods):
		_gnomeui.gnome_dialog_set_accelerator(self._o, button, ac_key,
						      ac_mods)
	def close(self, obj=None):
		_gnomeui.gnome_dialog_close(self._o)
	def close_hides(self, just_hide):
		_gnomeui.gnome_dialog_close_hides(self._o, just_hide)
	def set_close(self, click_closes):
		_gnomeui.gnome_dialog_set_close(self._o, click_closes)
	def editable_enters(self, editable):
		_gnomeui.gnome_dialog_editable_enters(self._o, editable._o)
	def append_buttons(self, b1=None, b2=None, b3=None, b4=None, b5=None,
			   b6=None, b7=None, b8=None, b9=None, b10=None):
		buttons = filter(lambda x: x, (b1,b2,b3,b4,b5,b6,b7,b8,b9,b10))
		apply(_gnomeui.gnome_dialog_append_buttons, (self._o,)+buttons)
	def append_button(self, name):
		_gnomeui.gnome_dialog_append_button(self._o, name)
	def append_button_with_pixmap(self, name, pixmap):
		_gnomeui.gnome_dialog_append_button_with_pixmap(self._o, name,
								pixmap)
_gtk._name2cls['GnomeDialog'] = GnomeDialog

# these are wrappers for the convenience functions.  They really return
# GnomeDialog's.
class GnomeOkDialog(GnomeDialog):
	def __init__(self, msg, parent=None):
		if parent:
			self._o = _gnomeui.gnome_ok_dialog_parented(msg,
								    parent._o)
		else:
			self._o = _gnomeui.gnome_ok_dialog(msg)
class GnomeErrorDialog(GnomeDialog):
	def __init__(self, error, parent=None):
		if parent:
			self._o = _gnomeui.gnome_error_dialog_parented(
				error, parent._o)
		else:
			self._o = _gnomeui.gnome_error_dialog(error)
class GnomeWarningDialog(GnomeDialog):
	def __init__(self, warning, parent=None):
		if parent:
			self._o = _gnomeui.gnome_warning_dialog_parented(
				warning, parent._o)
		else:
			self._o = _gnomeui.gnome_warning_dialog(warning)
class GnomeQuestionDialog(GnomeDialog):
	def __init__(self, question, cb, parent=None):
		if parent:
			self._o = _gnomeui.gnome_question_dialog_parented(
				question, cb, parent._o)
		else:
			self._o = _gnomeui.gnome_question_dialog(question, cb)
class GnomeOkCancelDialog(GnomeDialog):
	def __init__(self, message, cb, parent=None):
		if parent:
			self._o = _gnomeui.gnome_ok_cancel_dialog_parented(
				message, cb, parent._o)
		else:
			self._o = _gnomeui.gnome_ok_cancel_dialog(message, cb)
class GnomeRequestDialog(GnomeDialog):
	def __init__(self, password, prompt, default_text, max_length,
		     callback, parent):
		if parent: parent = parent._o
		self._o = _gnomeui.gnome_request_dialog(password, prompt,
							default_text,
							max_length, callback,
							parent)

class GnomeAbout(GnomeDialog):
	get_type = _gnomeui.gnome_about_get_type
	def __init__(self, title=None, version=None, copyright=None, list=[],
		     comments=None, logo=None, _obj=None):
		if _obj: self._o = _obj; return
		self._o = _gnomeui.gnome_about_new(title, version, copyright,
						   list, comments, logo)
_gtk._name2cls['GnomeAbout'] = GnomeAbout

class GnomeAnimator(_gtk.GtkWidget):
	get_type = _gnomeui.gnome_animator_get_type
	def __init__(self, width=100, height=100, _obj=None):
		if _obj: self._o = _obj; return
		self._o = _gnomeui.gnome_animator_new_with_size(width, height)
	def set_loop_type(self, loop_type):
		_gnomeui.gnome_animator_set_loop_type(self._o, loop_type)
	def get_loop_type(self):
		return _gnomeui.gnome_animator_get_loop_type(self._o)
	def set_playback_direction(self, dir):
		_gnomeui.gnome_animator_set_playback_direction(self._o, dir)
	def get_playback_direction(self):
		return _gnomeui.gnome_animator_get_playback_direction(self._o)
	def append_frame_from_imlib(self, image, xofs, yofs,
				    interval, width=-1, height=-1):
		if (width, height) == (-1, -1):
			return _gnomeui.gnome_animator_append_frame_from_imlib(
				self._o, image._im, xofs, yofs, interval)
		return _gnomeui.gnome_animator_append_frame_from_imlib_at_size(
			self._o, image._im, xofs, yofs, interval, width,height)
	def append_frame_from_file(self, file, xofs, yofs,
				   interval, width=-1, height=-1):
		if (width, height) == (-1, -1):
			return _gnomeui.gnome_animator_append_frame_from_file(
				self._o, file, xofs, yofs, interval)
		return _gnomeui.gnome_animator_append_frame_from_file_at_size(
			self._o, file, xofs, yofs, interval, width, height)
	def append_frames_from_imlib(self, image, xofs, yofs,
				     interval, xunit, width=-1, height=-1):
	       if (width, height) == (-1, -1):
		       return _gnomeui.gnome_animator_append_frames_from_imlib(
			     self._o, image._im, xofs, yofs, interval, xunit)
	       return _gnomeui.gnome_animator_append_frames_from_imlib_at_size(
			self._o, image._im, xofs, yofs, interval, xunit,
			width, height)
	def append_frames_from_file(self, file, xofs, yofs,
				   interval, xunit, width=-1, height=-1):
		if (width, height) == (-1, -1):
			return _gnomeui.gnome_animator_append_frames_from_file(
				self._o, file, xofs, yofs, interval, xunit)
		return _gnomeui.gnome_animator_append_frames_from_file_at_size(
			self._o, file, xofs, yofs, interval,xunit,width,height)
	def append_frame_from_gnome_pixmap(self, pixmap, xofs, yofs, interval):
		return _gnomeui.gnome_animator_append_frame_from_gnome_pixmap(
			self._o, pixmap._o, xofs, yofs, interval)
	def start(self):
		_gnomeui.gnome_animator_start(self._o)
	def stop(self):
		_gnomeui.gnome_animator_stop(self._o)
	def advance(self, num):
		return _gnomeui.gnome_animator_advance(self._o, num)
	def goto_frame(self, frame_num):
		_gnomeui.gnome_animator_goto_frame(self._o, frame_num)
	def get_current_frame_number(self):
		return _gnomeui.gnome_animator_get_current_frame(self._o)
	def get_status(self):
		return _gnomeui.gnome_animator_get_status(self._o)
	def set_playback_speed(self, speed):
		_gnomeui.gnome_animator_set_playback_speed(self._o, speed)
	def get_playback_speed(self):
		return _gnomeui.gnome_animator_get_playback_speed(self._o)
_gtk._name2cls['GnomeAnimator'] = GnomeAnimator

class GnomeApp(_gtk.GtkWindow):
	get_type = _gnomeui.gnome_app_get_type
	def __init__(self, appname='', title='', _obj=None):
		if _obj: self._o = _obj; return
		self._o = _gnomeui.gnome_app_new(appname, title)
	def set_menus(self, menubar):
		_gnomeui.gnome_app_set_menus(self._o, menubar._o)
	def set_toolbar(self, toolbar):
		_gnomeui.gnome_app_set_toolbar(self._o, toolbar._o)
	def set_statusbar(self, statusbar):
		_gnomeui.gnome_app_set_statusbar(self._o, statusbar._o)
	def set_statusbar_custom(self, container, statusbar):
		_gnomeui.gnome_app_set_statusbar_custom(self._o, container._o,
							statusbar._o)
	def set_contents(self, contents):
		_gnomeui.gnome_app_set_contents(self._o, contents._o)
	def add_toolbar(self, toolbar, name, behavior, placement, band_num,
			band_position, offset):
		_gnomeui.gnome_app_add_toolbar(self._o, toolbar._o, name,
					       behavior, placement, band_num,
					       band_position, offset)
	def add_docked(self, widget, name, behavior, placement, band_num,
			band_position, offset):
		_gnomeui.gnome_app_add_docked(self._o, widget._o, name,
					      behavior, placement, band_num,
					      band_position, offset)
	def add_dock_item(self, item, placement, band_num, band_position,
			  offset):
		_gnomeui.gnome_app_add_dock_item(self._o, item._o, placement,
						 band_num, band_position,
						 offset)
	def get_dock(self):
		return GnomeDock(_obj=_gnomeui.gnome_app_get_dock(self._o))
	def get_dock_item_by_name(self, name):
		return GnomeDockItem(
			_obj=_gnomeui.gnome_app_get_dock_item_by_name(self._o,
								      name))
	def create_menus(self, menuinfo):
		_gnomeui.gnome_app_create_menus(self._o, menuinfo)
	def create_toolbar(self, toolbarinfo):
		_gnomeui.gnome_app_create_toolbar(self._o, toolbarinfo)
	def remove_menus(self, path, num):
		_gnomeui.gnome_app_remove_menus(self._o, path, num)
	def remove_menu_range(self, path, start, num):
		_gnomeui.gnome_app_remove_menu_range(self._o, path, start, num)
	def insert_menus(self, path, menuinfo):
		_gnomeui.gnome_app_insert_menus(self._o, path, menuinfo)
	def install_menu_hints(self, menuinfo):
		# to be called AFTER adding a statusbar
		_gnomeui.gnome_app_install_menu_hints(self._o, menuinfo)

	def message(self, message):
		_gnomeui.gnome_app_message(self._o, message)
	def flash(self, message):
		_gnomeui.gnome_app_flash(self._o, message)
	def error(self, error):
		_gnomeui.gnome_app_error(self._o, error)
	def warning(self, warning):
		_gnomeui.gnome_app_warning(self._o, warning)
	def question(self, question, callback):
		_gnomeui.gnome_app_question(self._o, question, callback)
	def ok_cancel(self, question, callback):
		_gnomeui.gnome_app_ok_cancel(self._o, question, callback)
	def request_string(self, prompt, callback):
		_gnomeui.gnome_app_request_string(self._o, prompt, callback)
	def request_password(self, prompt, callback):
		_gnomeui.gnome_app_request_password(self._o, prompt, callback)
_gtk._name2cls['GnomeApp'] = GnomeApp

class GnomeAppBar(_gtk.GtkHBox):
	get_type = _gnomeui.gnome_appbar_get_type
	def __init__(self, has_progress=1, has_status=1,
		     interactivity=PREFERENCES_USER, _obj=None):
		if _obj: self._o = _obj; return
		self._o = _gnomeui.gnome_appbar_new(has_progress, has_status,
						    interactivity)
	def interactive(self):
		return _gnomeui.GNOME_APPBAR_INTERACTIVE(self._o)
	def set_status(self, status):
		_gnomeui.gnome_appbar_set_status(self._o, status)
	def set_default(self, default_status):
		_gnomeui.gnome_appbar_set_default(self._o, default_status)
	def push(self, status):
		_gnomeui.gnome_appbar_push(self._o, status)
	def pop(self):
		_gnomeui.gnome_appbar_pop(self._o)
	def clear_stack(self):
		_gnomeui.gnome_appbar_clear_stack(self._o)
	def set_progress(self, percentage):
		_gnomeui.gnome_appbar_set_progress(self._o, percentage)
	def get_progress(self):
		return _obj2inst(_gnomeui.gnome_appbar_get_progress(self._o))
	def refresh(self):
		_gnomeui.gnome_appbar_refresh(self._o)
	def set_prompt(self, prompt, modal=_gtk.FALSE):
		_gnomeui.gnome_appbar_set_prompt(self._o, prompt, modal)
	def clear_prompt(self):
		_gnomeui.gnome_appbar_clear_prompt(self._o)
	def get_response(self):
		return _gnomeui.gnome_appbar_get_response(self._o)
_gtk._name2cls['GnomeAppBar'] = GnomeAppBar

class GnomeCalculator(_gtk.GtkVBox):
	get_type = _gnomeui.gnome_calculator_get_type
	def __init__(self, _obj=None):
		if _obj: self._o = _obj; return
		self._o = _gnomeui.gnome_calculator_new()
	def clear(self, reset=1):
		_gnomeui.gnome_calculator_clear(self._o, reset)
	def set(self, result):
		_gnomeui.gnome_calculator_set(self._o, result)
	def get_result(self):
		return _gnomeui.gnome_calculator_get_result(self._o)
_gtk._name2cls['GnomeCalculator'] = GnomeCalculator

class GnomeCanvas(_gtk.GtkLayout):
	get_type = _gnomeui.gnome_canvas_get_type
	def __init__(self, aa=_gtk.FALSE, _obj=None):
		if _obj: self._o = _obj; return
		if aa:
			self._o = _gnomeui.gnome_canvas_new_aa()
		else:
			self._o = _gnomeui.gnome_canvas_new()
	def root(self):
		return GnomeCanvasGroup(
			_obj=_gnomeui.gnome_canvas_root(self._o))
	def set_scroll_region(self, x1,y1, x2,y2):
		_gnomeui.gnome_canvas_set_scroll_region(self._o, x1,y1, x2,y2)
	def get_scroll_region(self):
		return _gnomeui.gnome_canvas_get_scroll_region(self._o)
	def set_pixels_per_unit(self, n):
		_gnomeui.gnome_canvas_set_pixels_per_unit(self._o, n)
	def set_size(self, width, height):
		print "GnomeCanvas.set_size deprecated -- use set_usize"
		self.set_usize(width, height)
	def scroll_to(self, cx, cy):
		_gnomeui.gnome_canvas_scroll_to(self._o, cx, cy)
	def get_scroll_offsets(self):
		return _gnomeui.gnome_canvas_get_scroll_offsets(self._o)
	def update_now(self):
		_gnomeui.gnome_canvas_update_now(self._o)
	def get_item_at(self, x, y):
		ret = _gnomeui.gnome_canvas_get_item_at(self._o, x, y)
		if (ret): return _obj2inst(ret)
		return None
	def request_redraw(self, x1,y1, x2,y2):
		_gnomeui.gnome_canvas_request_redraw(self._o, x1,y1, x2,y2)
	def w2c(self, wx, wy):
		return _gnomeui.gnome_canvas_w2c(self._o, wx, wy)
	def c2w(self, cx, cy):
		return _gnomeui.gnome_canvas_c2w(self._o, cx, cy)
	def get_color(self, spec):
		return _gnomeui.gnome_canvas_get_color(self._o, spec)
	def set_stipple_origin(self, gc):
		_gnomeui.gnome_canvas_set_stipple_origin(self._o, gc)
_gtk._name2cls['GnomeCanvas'] = GnomeCanvas

class GnomeCanvasItem(_gtk.GtkObject):
	get_type = _gnomeui.gnome_canvas_item_get_type
	def __init__(self, _obj=None):
		if _obj: self._o = _obj; return
	def set(self, **args):
		_filtprops(args)
		_gnomeui.gnome_canvas_item_set(self._o, args)
	def move(self, dx, dy):
		_gnomeui.gnome_canvas_item_move(self._o, dx, dy)
	def affine_relative(self, affine):
		_gnomeui.gnome_canvas_item_affine_relative(self._o, affine)
	def affine_absolute(self, affine):
		_gnomeui.gnome_canvas_item_affine_absolute(self._o, affine)
	def raise_(self, positions):
		_gnomeui.gnome_canvas_item_raise(self._o, positions)
	def lower(self, positions):
		_gnomeui.gnome_canvas_item_lower(self._o, positions)
	def raise_to_top(self):
		_gnomeui.gnome_canvas_item_raise_to_top(self._o)
	def lower_to_bottom(self):
		_gnomeui.gnome_canvas_item_lower_to_bottom(self._o)
	def show(self):
		_gnomeui.gnome_canvas_item_show(self._o)
	def hide(self):
		_gnomeui.gnome_canvas_item_hide(self._o)
	def grab(self, mask, cursor, event_time):
		_gnomeui.gnome_canvas_item_grab(self._o, mask,
						cursor, event_time)
	def ungrab(self, event_time):
		_gnomeui.gnome_canvas_item_ungrab(self._o, event_time)
	def reparent(self, new_group):
		_gnomeui.gnome_canvas_item_reparent(self._o, new_group._o)
	def grab_focus(self):
		_gnomeui.gnome_canvas_item_grab_focus(self._o)
	def get_bounds(self):
		return _gnomeui.gnome_canvas_item_get_bounds(self._o)
	def w2i(self, x, y):
		return _gnomeui.gnome_canvas_item_w2i(self._o, x, y)
	def i2w(self, x, y):
		return _gnomeui.gnome_canvas_item_i2w(self._o, x, y)
_gtk._name2cls['GnomeCanvasItem'] = GnomeCanvasItem

class GnomeCanvasGroup(GnomeCanvasItem):
	"""has arguments 'x' and 'y'.  eg you can go new_item('group', x=4)"""
	get_type = _gnomeui.gnome_canvas_group_get_type
	def add(self, tp, **args):
		types = {
			'item':     _gnomeui.gnome_canvas_item_get_type,
			'group':    _gnomeui.gnome_canvas_group_get_type,
			'icon_text':_gnomeui.gnome_icon_text_item_get_type,
			'image':    _gnomeui.gnome_canvas_image_get_type,
			'line':     _gnomeui.gnome_canvas_line_get_type,
			'polygon':  _gnomeui.gnome_canvas_polygon_get_type,
			're':       _gnomeui.gnome_canvas_re_get_type,
			'rect':     _gnomeui.gnome_canvas_rect_get_type,
			'ellipse':  _gnomeui.gnome_canvas_ellipse_get_type,
			'text':     _gnomeui.gnome_canvas_text_get_type,
			'widget':   _gnomeui.gnome_canvas_widget_get_type
		}
		if type(tp) == type('string'):
			tp = types[tp]()
		_filtprops(args)
		return _obj2inst(_gnomeui.gnome_canvas_item_new(self._o,
								tp, args))
	def new_item(self, tp, **args):
		print "I renamed this func to GnomeCanvasGroup.add"
		return apply(self.add, (tp,), args)
	def children(self):
		return map(_obj2inst, _gnomeui.gnome_canvas_group_children(
			self._o))
_gtk._name2cls['GnomeCanvasGroup'] = GnomeCanvasGroup

class GnomeCanvasImage(GnomeCanvasItem):
	"""has arguments 'image', 'x', 'y', 'width', 'height', 'anchor'"""
	get_type = _gnomeui.gnome_canvas_image_get_type
_gtk._name2cls['GnomeCanvasImage'] = GnomeCanvasItem

class GnomeCanvasLine(GnomeCanvasItem):
	"""has arguments 'points', 'fill_color', 'width_pixels',
	'width_units', 'cap_style', 'join_style', 'first_arrowhead',
	'last_arrowhead', 'smooth', 'spline_steps', 'arrow_shape_a',
	'arrow_shape_b', 'arrow_shape_c'"""
	get_type = _gnomeui.gnome_canvas_line_get_type
_gtk._name2cls['GnomeCanvasLine'] = GnomeCanvasLine

class GnomeCanvasPolygon(GnomeCanvasItem):
	"""has argyments 'points', 'fill_color', 'fill_color_gdk',
	'outlint_color', 'outline_color_gdk', 'fill_stipple',
	'outline_stipple', 'width_pixels', 'width_units'"""
	get_type = _gnomeui.gnome_canvas_polygon_get_type
_gtk._name2cls['GnomeCanvasPolygon'] = GnomeCanvasPolygon

class GnomeCanvasRE(GnomeCanvasItem):
	"""has arguments 'x1', 'y1', 'x2', 'y2', 'fill_color', 'outline_color',
	'width_pixels', 'width_units'"""
	get_type = _gnomeui.gnome_canvas_re_get_type
_gtk._name2cls['GnomeCanvasRE'] = GnomeCanvasRE

class GnomeCanvasRect(GnomeCanvasRE):
	"""has arguments of GnomeCanvasRE"""
	get_type = _gnomeui.gnome_canvas_rect_get_type
_gtk._name2cls['GnomeCanvasRect'] = GnomeCanvasRect

class GnomeCanvasEllipse(GnomeCanvasRE):
	"""has arguments of GnomeCanvasRE"""
	get_type = _gnomeui.gnome_canvas_ellipse_get_type
_gtk._name2cls['GnomeCanvasEllipse'] = GnomeCanvasEllipse

class GnomeCanvasText(GnomeCanvasItem):
	"""has arguments 'text', 'x', 'y', 'font', 'anchor', 'justification',
	'fill_color'"""
	get_type = _gnomeui.gnome_canvas_text_get_type
_gtk._name2cls['GnomeCanvasText'] = GnomeCanvasText

class GnomeCanvasWidget(GnomeCanvasItem):
	"""has arguments 'widget', 'x', 'y', 'width', 'height', 'anchor',
	'size_pixels'"""
	get_type = _gnomeui.gnome_canvas_widget_get_type
_gtk._name2cls['GnomeCanvasWidget'] = GnomeCanvasWidget

class GnomeClient(_gtk.GtkObject):
	get_type = _gnomeui.gnome_client_get_type
	def __init__(self, connected=_gtk.TRUE, _obj=None):
		if _obj: self._o = _obj; return
		if connected:
		    self._o = _gnomeui.gnome_client_new()
		else:
		    self._o = _gnomeui.gnome_client_new_without_connection()
	def connect(self):
		_gnomeui.gnome_client_connect(self._o)
	def disconnect(self):
		_gnomeui.gnome_client_disconnect(self._o)
	def set_id(self, client_id):
		_gnomeui.gnome_client_set_id(self._o, client_id)
	def get_id(self):
		return _gnomeui.gnome_client_get_id(self._o)
	def get_previous_id(self):
		return _gnomeui.gnome_client_get_previous_id(self._o)
	def get_config_prefix(self):
		return _gnomeui.gnome_client_get_config_prefix(self._o)
	def get_global_config_prefix(self):
		return _gnomeui.gnome_client_get_global_config_prefix(self._o)
	def set_clone_command(self, vector):
		_gnomeui.gnome_client_set_clone_command(self._o, vector)
	def set_discard_command(self, vector):
		_gnomeui.gnome_client_set_discard_command(self._o, vector)
	def set_environment(self, name, value):
		_gnomeui.gnome_client_set_environment(self._o, name, value)
	def set_restart_command(self, vector):
		_gnomeui.gnome_client_set_restart_command(self._o, vector)
	def set_resign_command(self, vector):
		_gnomeui.gnome_client_set_resign_command(self._o, vector)
	def set_shutdown_command(self, vector):
		_gnomeui.gnome_client_set_shutdown_command(self._o, vector)
	def set_current_directory(self, dir):
		_gnomeui.gnome_client_set_current_directory(self._o, dir)
	def set_process_id(self, pid):
		_gnomeui.gnome_client_set_process_id(self._o, pid)
	def set_program(self, program):
		_gnomeui.gnome_client_set_program(self._o, program)
	def set_restart_style(self, style):
		_gnomeui.gnome_client_set_restart_style(self._o, style)
	def set_priority(self, priority):
		_gnomeui.gnome_client_set_priority(self._o, priority)
	def set_user_id(self, user_id):
		_gnomeui.gnome_client_set_user_id(self._o, user_id)
	def request_phase_2(self):
		_gnomeui.gnome_client_request_phase_2(self._o)
	def request_interaction(self, dialog, func):
		_gnomeui.gnome_client_request_interaction(self._o,dialog,func)
	def request_save(self, save_style, shutdown, interact_style, fast,
			 is_global):
		_gnomeui.gnome_client_request_save(self._o, save_style,
						   shutdown, interact_style,
						   fast, is_global)
	def flush(self):
		_gnomeui.gnome_client_flush(self._o)
	def get_client_flags(self):
		return _gnomeui.gnome_client_get_client_flags(self._o)
_gtk._name2cls['GnomeClient'] = GnomeClient

def interaction_key_return(key, cancel_shutdown):
	_gnomeui.gnome_interaction_key_return(key, cancel_shutdown)
# set up the default SM client
master_client = GnomeClient(_obj=_gnomeui.gnome_master_client())

class GnomeClonedClient(GnomeClient):
	def __init__(self):
		self._o = _gnomeui.gnome_cloned_client()

class GnomeColorPicker(_gtk.GtkButton):
	get_type = _gnomeui.gnome_color_picker_get_type
	def __init__(self, _obj=None):
		if _obj: self._o = _obj; return
		self._o = _gnomeui.gnome_color_picker_new()
	def set_d(self, r, g, b, a):
		_gnomeui.gnome_color_picker_set_d(self._o, r, g, b, a)
	def get_d(self):
		return _gnomeui.gnome_color_picker_get_d(self._o)
	def set_i8(self, r, g, b, a):
		_gnomeui.gnome_color_picker_set_i8(self._o, r, g, b, a)
	def get_i8(self):
		return _gnomeui.gnome_color_picker_get_i8(self._o)
	def set_i16(self, r, g, b, a):
		_gnomeui.gnome_color_picker_set_i16(self._o, r, g, b, a)
	def get_i16(self):
		return _gnomeui.gnome_color_picker_get_i16(self._o)
	def set_dither(self, dither):
		_gnomeui.gnome_color_picker_set_dither(self._o, dither)
	def set_use_alpha(self, use_alpha):
		_gnomeui.gnome_color_picker_set_use_alpha(self._o, use_alpha)
	def set_title(self, title):
		_gnomeui.gnome_color_picker_set_title(self._o, title)
_gtk._name2cls['GnomeColorPicker'] = GnomeColorPicker

class GnomeDateEdit(_gtk.GtkHBox):
	get_type = _gnomeui.gnome_date_edit_get_type
	def __init__(self, the_time=0, flags=0, _obj=None):
		if _obj: self._o = _obj; return
		self._o = _gnomeui.gnome_date_edit_new_flags(the_time, flags)
	def set_time(self, the_time):
		_gnomeui.gnome_date_edit_set_time(self._o, the_time)
	def set_popup_range(self, low_hour, up_hour):
		_gnomeui.gnome_date_edit_set_popup_range(self._o, low_hour,
							 up_hour)
	def get_date(self):
		return _gnomeui.gnome_date_edit_get_date(self._o)
	def set_flags(self, flags):
		_gnomeui.gnome_date_edit_set_flags(self._o, flags)
	def get_flags(self):
		return _gnomeui.gnome_date_edit_get_flags(self._o)
_gtk._name2cls['GnomeDateEdit'] = GnomeDateEdit

class GnomeDEntryEdit(_gtk.GtkObject):
	get_type = _gnomeui.gnome_dentry_edit_get_type
	def __init__(self, notebook=None, _obj=None):
		if _obj: self._o = _obj; return
		if notebook:
			self._o = _gnomeui.gnome_dentry_edit_new_notebook(
				notebook._o)
		else:
			self._o = _gnomeui.gnome_dentry_edit_new()
	def child1(self):
		return _obj2inst(_gnomeui.gnome_dentry_edit_child1(self._o))
	def child2(self):
		return _obj2inst(_gnomeui.gnome_dentry_edit_child2(self._o))
	def clear(self):
		_gnomeui.gnome_dentry_edit_clear(self._o)
	def load_file(self, filename):
		_gnomeui.gnome_dentry_edit_load_file(self._o, filename)
	# this is provided, because I haven't added dentry support to the
	# _gnome low-level module.
	def save_file(self, filename):
		_gnomeui.gnome_dentry_edit_save_file(self._o, filename)
	def get_icon(self):
		return _gnomeui.gnome_dentry_edit_get_icon(self._o)
	def get_name(self):
		return _gnomeui.gnome_dentry_edit_get_name(self._o)
	def get_name_entry(self):
		return _obj2inst(_gnomeui.gnome_dentry_edit_get_name_entry(
			self._o))
	def get_comment_entry(self):
		return _obj2inst(_gnomeui.gnome_dentry_edit_get_comment_entry(
			self._o))
	def get_exec_entry(self):
		return _obj2inst(_gnomeui.gnome_dentry_edit_get_exec_entry(
			self._o))
	def get_tryexec_entry(self):
		return _obj2inst(_gnomeui.gnome_dentry_edit_get_tryexec_entry(
			self._o))
	def get_doc_entry(self):
		return _obj2inst(_gnomeui.gnome_dentry_edit_get_doc_entry(
			self._o))
	def get_icon_entry(self):
		return _obj2inst(_gnomeui.gnome_dentry_edit_get_icon_entry(
			self._o))
_gtk._name2cls['GnomeDEntryEdit'] = GnomeDEntryEdit

class GnomeDockBand(_gtk.GtkContainer):
	get_type = _gnomeui.gnome_dock_band_get_type
	def __init__(self, _obj=None):
		if _obj: self._o = _obj; return
		self._o = _gnomeui.gnome_dock_band_new()
	def set_orientation(self, orient):
		_gnomeui.gnome_dock_band_set_orientation(self._o, orient)
	def get_orientation(self):
		return _gnomeui.gnome_dock_band_get_orientation(self._o)
	def insert(self, child, offset, position):
		return _gnomeui.gnome_dock_band_insert(self._o, child._o,
						       offset, position)
	def prepend(self, child, offset):
		return _gnomeui.gnome_dock_band_prepend(self._o, child._o,
							offset)
	def append(self, child, offset):
		return _gnomeui.gnome_dock_band_append(self._o, child._o,
						       offset)
	def set_child_offset(self, child, offset):
		_gnomeui.gnome_dock_band_set_child_offset(self._o, child._o,
							  offset)
	def get_child_offset(self, child):
		return _gnomeui.gnome_dock_band_get_child_offset(self._o,
								 child._o)
	def get_num_children(self):
		return _gnomeui.gnome_dock_band_get_num_children(self._o)
	def drag_begin(self, item):
		_gnomeui.gnome_dock_band_drag_begin(self._o, item._o)
	def drag_to(self, item, x, y):
		_gnomeui.gnome_dock_band_drag_to(self._o, item._o, x, y)
	def drag_end(self, item):
		_gnomeui.gnome_dock_band_drag_end(self._o, item._o)
_gtk._name2cls['GnomeDockBand'] = GnomeDockBand

class GnomeDockItem(_gtk.GtkBin):
	get_type = _gnomeui.gnome_dock_item_get_type
	def __init__(self, name="", behaviour=DOCK_ITEM_BEH_NORMAL, _obj=None):
		if _obj: self._o = _obj; return
		self._o = _gnomeui.gnome_dock_item_new(name, behaviour)
	def get_child(self):
		return _obj2inst(_gnomeui.gnome_dock_item_get_child(self._o))
	def get_name(self):
		return _gnomeui.gnome_dock_item_get_name(self._o)
	def set_shadow_type(self, type):
		_gnomeui.gnome_dock_item_set_shadow_type(self._o, type)
	def get_shadow_type(self):
		return _gnomeui.gnome_dock_item_get_shadow_type(self._o)
	def set_orientation(self, orient):
		_gnomeui.gnome_dock_item_set_orientation(self._o, orient)
	def get_orientation(self):
		return _gnomeui.gnome_dock_item_get_orientation(self._o)
	def get_behavior(self):
		return _gnomeui.gnome_dock_item_get_behavior(self._o)
_gtk._name2cls['GnomeDockItem'] = GnomeDockItem

class GnomeDock(_gtk.GtkContainer):
	get_type = _gnomeui.gnome_dock_get_type
	def __init__(self, _obj=None):
		if _obj: self._o = _obj; return
		self._o = _gnomeui.gnome_dock_new()
	def allow_floating_items(self, enable):
		_gnomeui.gnome_dock_allow_floating_items(self._o, enable)
	def add_item(self, item, placement, band_num, position, offset,
		     in_new_band):
		_gnomeui.gnome_dock_add_item(self._o, item._o, placement,
					     band_num, position, offset,
					     in_new_band)
	def add_floating_item(self, item, x, y, orientation):
		_gnomeui.gnome_dock_add_item(self._o, item._o, x,y,orientation)
	def set_client_area(self, widget):
		_gnomeui.gnome_dock_set_client_area(self._o, widget._o)
	def get_client_area(self):
		return _obj2inst(_gnomeui.gnome_dock_get_client_area(self._o))
_gtk._name2cls['GnomeDock'] = GnomeDock

class GnomeDruid(_gtk.GtkContainer):
	get_type = _gnomeui.gnome_druid_get_type
	def __init__(self, _obj=None):
		if _obj: self._o = _obj; return
		self._o = _gnomeui.gnome_druid_new()
	def set_buttons_sensitive(self, back_sens, next_sens, cancel_sens):
		_gnomeui.gnome_druid_set_buttons_sensitive(self._o, back_sens,
							   next_sens,
							   cancel_sens)
	def set_show_finish(self, show_finish):
		_gnomeui.gnome_druid_set_show_finish(self._o, show_finish)
	def prepend_page(self, page):
		_gnomeui.gnome_druid_prepend_page(self._o, page._o)
	def insert_page(self, back_page, page):
		_gnomeui.gnome_druid_insert_page(self._o, back_page._o,page._o)
	def append_page(self, page):
		_gnomeui.gnome_druid_append_page(self._o, page._o)
	def set_page(self, page):
		_gnomeui.gnome_druid_set_page(self._o, page._o)
_gtk._name2cls['GnomeDruid'] = GnomeDruid

class GnomeDruidPage(_gtk.GtkBin):
	get_type = _gnomeui.gnome_druid_page_get_type
	def next(self):
		return _gnomeui.gnome_druid_page_next(self._o)
	def prepare(self):
		_gnomeui.gnome_druid_page_prepare(self._o)
	def back(self):
		return _gnomeui.gnome_druid_page_back(self._o)
	def cancel(self):
		return _gnomeui.gnome_druid_page_cancel(self._o)
	def finish(self):
		_gnomeui.gnome_druid_page_finish(self._o)
_gtk._name2cls['GnomeDruidPage'] = GnomeDruidPage

class GnomeDruidPageStart(GnomeDruidPage):
	get_type = _gnomeui.gnome_druid_page_start_get_type
	def __init__(self, title=None, text=None, logo=None, watermark=None,
		     _obj=None):
		if _obj: self._o = _obj; return
		if logo: logo = logo._im
		if watermark: watermark = watermark._im
		self._o = _gnomeui.gnome_druid_page_start_new_with_vals(
			title, text, logo, watermark)
	def set_bg_color(self, colour):
		_gnomeui.gnome_druid_page_start_set_bg_color(self._o, colour)
	def set_textbox_color(self, colour):
		_gnomeui.gnome_druid_page_start_set_textbox_color(self._o,
								  colour)
	def set_logo_bg_color(self, colour):
		_gnomeui.gnome_druid_page_start_set_logo_bg_color(self._o,
								  colour)
	def set_title_color(self, colour):
		_gnomeui.gnome_druid_page_start_set_title_color(self._o,colour)
	def set_text_color(self, colour):
		_gnomeui.gnome_druid_page_start_set_text_color(self._o, colour)
	def set_text(self, text):
		_gnomeui.gnome_druid_page_start_set_text(self._o, text)
	def set_title(self, title):
		_gnomeui.gnome_druid_page_start_set_title(self._o, title)
	def set_logo(self, image):
		_gnomeui.gnome_druid_page_start_set_logo(self._o, image._im)
	def set_watermark(self, image):
		_gnomeui.gnome_druid_page_start_set_watermark(self._o,
							      image._im)
_gtk._name2cls['GnomeDruidPageStart'] = GnomeDruidPageStart

class GnomeDruidPageFinish(GnomeDruidPage):
	get_type = _gnomeui.gnome_druid_page_finish_get_type
	def __init__(self, title=None, text=None, logo=None, watermark=None,
		     _obj=None):
		if _obj: self._o = _obj; return
		if logo: logo = logo._im
		if watermark: watermark = watermark._im
		self._o = _gnomeui.gnome_druid_page_finish_new_with_vals(
			title, text, logo, watermark)
	def set_bg_color(self, colour):
		_gnomeui.gnome_druid_page_finish_set_bg_color(self._o, colour)
	def set_textbox_color(self, colour):
		_gnomeui.gnome_druid_page_finish_set_textbox_color(self._o,
								   colour)
	def set_logo_bg_color(self, colour):
		_gnomeui.gnome_druid_page_finish_set_logo_bg_color(self._o,
								   colour)
	def set_title_color(self, colour):
		_gnomeui.gnome_druid_page_finish_set_title_color(self._o,
								 colour)
	def set_text_color(self, colour):
		_gnomeui.gnome_druid_page_finish_set_text_color(self._o,colour)
	def set_text(self, text):
		_gnomeui.gnome_druid_page_finish_set_text(self._o, text)
	def set_title(self, title):
		_gnomeui.gnome_druid_page_finish_set_title(self._o, title)
	def set_logo(self, image):
		_gnomeui.gnome_druid_page_finish_set_logo(self._o, image._im)
	def set_watermark(self, image):
		_gnomeui.gnome_druid_page_finish_set_watermark(self._o,
							       image._im)
_gtk._name2cls['GnomeDruidPageFinish'] = GnomeDruidPageFinish

class GnomeDruidPageStandard(GnomeDruidPage):
	get_type = _gnomeui.gnome_druid_page_standard_get_type
	def __init__(self, title=None, logo=None, _obj=None):
		if _obj: self._o = _obj; return
		if logo: logo = logo._im
		self._o = _gnomeui.gnome_druid_page_standard_new_with_vals(
			title, logo)
	def __getattr__(self, attr):
		if attr == 'vbox':
			return _obj2inst(_gnomeui.gnome_druid_page_standard_get_vbox(
				self._o))
		else:
			return GnomeDruidPage.__getattr(self, attr)
	def set_bg_color(self, color):
		_gnomeui.gnome_druid_page_standard_set_bg_color(self._o, color)
	def set_logo_bg_color(self, color):
		_gnomeui.gnome_druid_page_standard_set_logo_bg_color(self._o,
								     color)
	def set_title_color(self, color):
		_gnomeui.gnome_druid_page_standard_set_title_color(self._o,
								   color)
	def set_title(self, title):
		_gnomeui.gnome_druid_page_set_title(self._o, title)
	def set_logo(self, image):
		_gnomeui.gnome_druid_page_set_logo(self._o, image._im)
_gtk._name2cls['GnomeDruidPageStandard'] = GnomeDruidPageStandard

class GnomeEntry(_gtk.GtkCombo):
	get_type = _gnomeui.gnome_entry_get_type
	def __init__(self, history_id='', _obj=None):
		if _obj: self._o = _obj; return
		self._o = _gnomeui.gnome_entry_new(history_id)
	def gtk_entry(self):
		return _obj2inst(_gnomeui.gnome_entry_gtk_entry(self._o))
	def set_history_id(self, history_id):
		_gnomeui.gnome_entry_set_history_id(self._o, history_id)
	def prepend_history(self, save, text):
		_gnomeui.gnome_entry_prepend_history(self._o, save, text)
	def append_history(self, save, text):
		_gnomeui.gnome_entry_append_history(self._o, save, text)
	def load_history(self):
		_gnomeui.gnome_entry_load_history(self._o)
	def save_history(self):
		_gnomeui.gnome_entry_save_history(self._o)
_gtk._name2cls['GnomeEntry'] = GnomeEntry

class GnomeFileEntry(_gtk.GtkHBox):
	get_type = _gnomeui.gnome_file_entry_get_type
	def __init__(self, history_id='', browse_dialog_title='', _obj=None):
		if _obj: self._o = _obj; return
		self._o = _gnomeui.gnome_file_entry_new(history_id,
							browse_dialog_title)
	def gnome_entry(self):
		return _obj2inst(_gnomeui.gnome_file_entry_gnome_entry(
			self._o))
	def gtk_entry(self):
		return _obj2inst(_gnomeui.gnome_file_entry_gtk_entry(self._o))
	def set_title(self, browse_dialog_title):
		_gnomeui.gnome_file_entry_set_title(self._o,
						    browse_dialog_title)
	def set_default_path(self, path):
		_gnomeui.gnome_file_entry_set_default_path(self._o, path)
	def get_full_path(self, file_must_exist):
		return _gnomeui.gnome_file_entry_get_full_path(self._o,
							       file_must_exist)
	def set_modal(self, is_modal=_gtk.TRUE):
		_gnomeui.gnome_file_entry_set_modal(self._o, is_modal)
_gtk._name2cls['GnomeFileEntry'] = GnomeFileEntry

class GnomeFontPicker(_gtk.GtkButton):
	get_type = _gnomeui.gnome_font_picker_get_type
	def __init__(self, _obj=None):
		if _obj: self._o = _obj; return
		self._o = _gnomeui.gnome_font_picker_new()
	def set_title(self, title):
		_gnomeui.gnome_font_picker_set_title(self._o, title)
	def get_mode(self):
		return _gnomeui.gnome_font_picker_get_mode(self._o)
	def set_mode(self, mode):
		_gnomeui.gnome_font_picker_set_mode(self._o, mode)
	def fi_set_use_font_in_label(self, use_font_in_label, size):
		_gnomeui.gnome_font_picker_fi_set_use_font_in_label(
			self._o, use_font_in_label, size)
	def fi_set_show_size(self, show_size):
		_gnomeui.gnome_font_picker_fi_set_show_size(self._o, show_size)
	def uw_set_widget(self, widget):
		_gnomeui.gnome_font_picker_uw_set_widget(self._o, widget._o)
	def get_font_name(self):
		return _gnomeui.gnome_font_picker_get_font_name(self._o)
	def get_font(self):
		return _gnomeui.gnoem_font_picker_get_font(self._o)
	def set_font_name(self, fontname):
		_gnomeui.gnome_font_picker_set_font_name(self._o, fontname)
	def get_preview_text(self):
		return _gnomeui.gnome_font_picker_get_preview_text(self._o)
	def set_preview_text(self, text):
		_gnomeui.gnome_font_picker_set_preview_text(self._o, text)
_gtk._name2cls['GnomeFontPicker'] = GnomeFontPicker

class GnomeGuru(_gtk.GtkVBox):
	get_type = _gnomeui.gnome_guru_get_type
	def __init__(self, name=None, graphic=None, dialog=None, _obj=None):
		if _obj: self._o = _obj; return
		self._o = _gnomeui.gnome_guru_new(name, graphic._o, dialog._o)
	def append_page(self, name, page):
		_gnomeui.gnome_guru_append_page(self._o, name, page._o)
	def next_set_sensitive(self, sensitivity):
		_gnomeui.gnome_guru_next_set_sensitivity(self._o, sensitivity)
	def back_set_sensitive(self, sensitivity):
		_gnomeui.gnome_guru_back_set_sensitivity(self._o, sensitivity)
	def current_page(self):
		return _gnomeui.gnome_guru_current_page(self._o)
_gtk._name2cls['GnomeGuru'] = GnomeGuru

class GnomeHRef(_gtk.GtkButton):
	get_type = _gnomeui.gnome_href_get_type
	def __init__(self, url='', label=None, _obj=None):
		if _obj: self._o = _obj; return
		self._o = _gnomeui.gnome_href_new(url, label)
	def set_url(self, url):
		_gnomeui.gnome_href_set_url(self._o, url)
	def get_url(self):
		return _gnomeui.gnome_href_get_url(self._o)
	def set_label(self, label):
		_gnomeui.gnome_href_set_label(self._o, label)
	def get_label(self):
		return _gnomeui.gnome_href_get_label(self._o)
_gtk._name2cls['GnomeHRef'] = GnomeHRef

class GnomeIconEntry(_gtk.GtkVBox):
	get_type = _gnomeui.gnome_icon_entry_get_type
	def __init__(self, history_id='', browse_dialog_title='', _obj=None):
		if _obj: self._o = _obj; return
		self._o = _gnomeui.gnome_icon_entry_new(history_id,
							browse_dialog_title)
	def set_pixmap_subdir(self, subdir):
		_gnomeui.gnome_icon_entry_set_pixmap_subdir(self._o, subdir)
	def set_icon(self, filename):
		_gnomeui.gnome_icon_entry_set_icon(self._o, filename)
	def gnome_file_entry(self):
		return GnomeFileEntry(_obj=
			_gnomeui.gnome_icon_entry_gnome_file_entry(self._o))
	def gnome_entry(self):
		return GnomeEntry(_obj=
			_gnomeui.gnome_icon_entry_gnome_entry(self._o))
	def gtk_entry(self):
		return _gtk.GtkEntry(_obj=
			_gnomeui.gnome_icon_entry_gtk_entry(self._o))
	def get_filename(self):
		return _gnomeui.gnome_icon_entry_get_filename(self._o)
_gtk._name2cls['GnomeIconEntry'] = GnomeIconEntry

class GnomeIconList(GnomeCanvas):
	get_type = _gnomeui.gnome_icon_list_get_type
	def __init__(self, icon_width=70, adj=None,
		     is_editable=_gtk.FALSE, _obj=None):
		if _obj: self._o = _obj; return
		if adj: adj = adj._o
		self._o = _gnomeui.gnome_icon_list_new(icon_width, adj,
						       is_editable)
	def set_selection_mode(self, mode):
		_gnomeui.gnome_icon_list_set_selection_mode(self._o, mode)
	def append(self, icon_filename, text):
		return _gnomeui.gnome_icon_list_append(self._o, icon_filename,
						       text)
	def append_imlib(self, image, text):
		return _gnomeui.gnome_icon_list_append_imlib(self._o,
							     image._im, text)
	def insert(self, pos, icon_filename, text):
		_gnomeui.gnome_icon_list_insert(self._o, pos, icon_filename,
						text)
	def insert_imlib(self, pos, image, text):
		_gnomeui.gnome_icon_list_insert_imlib(self._o, pos, image._im,
						      text)
	def clear(self):
		_gnomeui.gnome_icon_list_clear(self._o)
	def remove(self, pos):
		_gnomeui.gnome_icon_list_remove(self._o, pos)
	def set_icon_data(self, pos, data):
		_gnomeui.gnome_icon_list_set_icon_data(self._o, pos, data)
	def get_icon_data(self, pos):
		return _gnomeui.gnome_icon_list_get_icon_data(self._o, pos)
	def find_icon_from_data(self, data):
		return _gnomeui.gnome_icon_list_find_icon_from_data(self._o,
								    data)
	def select_icon(self, pos):
		_gnomeui.gnome_icon_list_select_icon(self._o, pos)
	def unselect_icon(self, pos):
		_gnomeui.gnome_icon_list_unselect_icon(self._o, pos)
	def select_all(self, event):
		_gnomeui.gnome_icon_list_select_all(self._o, event)
	def freeze(self):
		_gnomeui.gnome_icon_list_freeze(self._o)
	def thaw(self):
		_gnomeui.gnome_icon_list_thaw(self._o)
	def moveto(self, pos, yalign=0.5):
		_gnomeui.gnome_icon_list_moveto(self._o, pos, yalign)
	def icon_is_visible(self, pos):
		return _gnomeui.gnome_icon_list_icon_is_visible(self._o, pos)
	def set_icon_width(self, width):
		_gnomeui.gnome_icon_list_set_icon_width(self._o, width)
	def set_row_spacing(self, spacing):
		_gnomeui.gnome_icon_list_set_row_spacing(self._o, spacing)
	def set_col_spacing(self, spacing):
		_gnomeui.gnome_icon_list_set_col_spacing(self._o, spacing)
	def set_text_spacing(self, spacing):
		_gnomeui.gnome_icon_list_set_text_spacing(self._o, spacing)
	def set_icon_border(self, spacing):
		_gnomeui.gnome_icon_list_set_icon_border(self._o, spacing)
	def set_separators(self, separators):
		_gnomeui.gnome_icon_list_set_separators(self._o, separators)
	def set_hadjustment(self, adj):
		_gnomeui.gnome_icon_list_set_hadjustment(self._o, adj._o)
	def set_vadjustment(self, adj):
		_gnomeui.gnome_icon_list_set_vadjustment(self._o, adj._o)
	def get_icon_at(self, x, y):
		return _gnomeui.gnome_icon_list_get_icon_at(self._o, x, y)
	def get_items_per_line(self):
		return _gnomeui.gnome_icon_list_get_items_per_line(self._o)
_gtk._name2cls['GnomeIconList'] = GnomeIconList

class GnomeIconTextItem(GnomeCanvasItem):
	get_type = _gnomeui.gnome_icon_text_item_get_type
	def setxy(self, x, y):
		_gnomeui.gnome_icon_text_item_setxy(self._o, x, y)
	def select(self, select):
		_gnomeui.gnome_icon_text_item_select(self._o, select)
	def get_text(self):
		return _gnomeui.gnome_icon_text_item_get_text(self._o)
	def stop_editing(self):
		_gnomeui.gnome_icon_text_stop_editing(self._o)

class GnomeIconSelection(_gtk.GtkVBox):
	get_type = _gnomeui.gnome_icon_selection_get_type
	def __init__(self, _obj=None):
		if _obj: self._o = _obj; return
		self._o = _gnomeui.gnome_icon_selection_new()
	def add_defaults(self):
		_gnomeui.gnome_icon_selection_add_defaults(self._o)
	def add_directory(self, dir):
		_gnomeui.gnome_icon_selection_add_directory(self._o, dir)
	def show_icons(self):
		_gnomeui.gnome_icon_selection_show_icons(self._o)
	def clear(self, not_shown=_gtk.TRUE):
		_gnomeui.gnome_icon_selection_clear(self._o, not_shown)
	def get_icon(self, full_path=_gtk.TRUE):
		return _gnomeui.gnome_icon_selection_get_icon(self._o,
							      full_path)
	def select_icon(self, filename):
		_gnomeui.gnome_icon_selection_select_icon(self._o, filename)
_gtk._name2cls['GnomeIconSelection'] = GnomeIconSelection

class GnomeLess(_gtk.GtkVBox):
	get_type = _gnomeui.gnome_less_get_type
	def __init__(self, _obj=None):
		if _obj: self._o = _obj; return
		self._o = _gnomeui.gnome_less_new()
	def clear(self):
		_gnomeui.gnome_less_clear(self._o)
	def show_file(self, path):
		return _gnomeui.gnome_less_show_file(self._o, path)
	def show_command(self, command_line):
		return _gnomeui.gnome_less_show_command(self._o, command_line)
	def show_string(self, string):
		_gnomeui.gnome_less_show_string(self._o, string)
	def show_filestream(self, fp):
		return _gnomeui.gnome_less_show_filestream(self._o, fp)
	def write_file(self, filename):
		return _gnomeui.gnome_less_write_file(self._o, filename)
	def set_font(self, font):
		_gnomeui.gnome_less_set_font(self._o, font)
	def set_fixed_font(self, fixed):
		_gnomeui.gnome_less_set_fixed_font(self._o, fixed)
	def reshow(self):
		_gnomeui.gnome_less_reshow(self._o)
_gtk._name2cls['GnomeLess'] = GnomeLess

class GnomeMDI(_gtk.GtkObject):
	get_type = _gnomeui.gnome_mdi_get_type
	def __init__(self, appname=None, title=None, _obj=None):
		if _obj: self._o = _obj; return
		self._o = _gnomeui.gnome_mdi_new(appname, title)
	def set_mode(self, mode):
		_gnomeui.gnome_mdi_set_mode(self._o, mode)
	def set_menubar_template(self, info):
		_gnomeui.gnome_mdi_set_menubar_template(self._o, info)
	def set_toolbar_template(self, info):
		_gnomeui.gnome_mdi_set_toolbar_template(self._o, info)
	def set_child_menu_path(self, path):
		_gnomeui.gnome_mdi_set_child_menu_path(self._o, path)
	def set_child_list_path(self, path):
		_gnomeui.gnome_mdi_set_child_list_path(self._o, path)
	def get_active_child(self):
		return GnomeMDIChild(
			_obj=_gnomeui.gnome_mdi_get_active_child(self._o))
	def find_child(self, name):
		return GnomeMDIChild(
			_obj=_gnomeui.gnome_mdi_find_child(self._o, name))
	def add_view(self, child):
		return _gnomeui.gnome_mdi_add_view(self._o, child._o)
	def add_toplevel_view(self, child):
		return _gnomeui.gnome_mdi_add_view(self._o, child._o)
	def remove_view(self, wid, force):
		return _gnomeui.gnome_mdi_remove_view(self._o, wid._o, force)
	def get_active_view(self):
		return _obj2inst(_gnomeui.gnome_mdi_get_active_view(self._o))
	def add_child(self, child):
		return _gnomeui.gnome_mdi_add_child(self._o, child._o)
	def remove_child(self, child, force):
		return _gnomeui.gnome_mdi_remove_child(self._o, child._o,force)
	def remove_all(self, force):
		return _gnomeui.gnome_mdi_remove_all(self._o, force)
	def open_toplevel(self):
		_gnomeui.gnome_mdi_open_toplevel(self._o)
	def update_child(self, child):
		_gnomeui.gnome_mdi_update_child(self._o, child._o)
	def get_active_window(self):
		return GnomeApp(_obj=
				_gnomeui.gnome_mdi_get_active_window(self._o))
	def register(self, wid):
		_gnomeui.gnome_mdi_register(self._o, wid._o)
	def unregister(self, wid):
		_gnomeui.gnome_mdi_unregister(self._o, wid._o)
	def get_app_from_view(self, view):
		return GnomeApp(_obj=
				_gnomeui.gnome_mdi_get_app_from_view(view._o))
	def get_child_from_view(self, view):
		return GnomeMDIChild(_obj=
				     _gnomeui.gnome_mdi_get_child_from_view(
					     view._o))
	def get_view_from_window(self, app):
		return _obj2inst(_gnomeui.gnome_mdi_get_view_from_window(
			self._o, app._o))
	def set_active_view(self, view):
		_gnomeui.gnome_mdi_set_active_view(self._o, view._o)
_gtk._name2cls['GnomeMDI'] = GnomeMDI

class GnomeMDIChild(_gtk.GtkObject):
	get_type = _gnomeui.gnome_mdi_child_get_type
	def __init__(self, _obj=None):
		if _obj: self._o = _obj; return
		raise RuntimeError, \
		      "can only initialise GnomeMDIChild subclasses"
	def add_view(self):
		return _obj2inst(_gnomeui.gnome_mdi_child_add_view(self._o))
	def remove_view(self, view):
		_gnomeui.gnome_mdi_child_remove_view(self._o, view._o)
	def set_name(self, name):
		_gnomeui.gnome_mdi_child_set_name(self._o, name)
	def set_menu_template(self, info):
		_gnomeui.gnome_mdi_child_set_menu_template(self._o, info)
_gtk._name2cls['GnomeMDIChild'] = GnomeMDIChild

class GnomeMDIGenericChild(GnomeMDIChild):
	get_type = _gnomeui.gnome_mdi_generic_child_get_type
	def __init__(self, name="", _obj=None):
		if _obj: self._o = _obj; return
		self._o = _gnomeui.gnome_mdi_generic_child_new(name)
	class __marshal:
		def __init__(self, func):
			self.func = func
		def __call__(self, *args):
			a = list(args)
			for i in range(len(args)):
				if type(args[i]) == _gtk._gtk.GtkObjectType:
					a[i] = _obj2inst(args[i])
			a = tuple(a)
			ret = apply(self.func, a)
			if hasattr(ret, '_o'): ret = ret._o
			return ret
	def set_view_creator(self, func, *args):
		mfunc = self.__marshal(func).__call__
		_gnomeui.gnome_mdi_generic_child_set_view_creator(self._o, mfunc,
								  args)
	def set_menu_creator(self, func, *args):
		mfunc = self.__marshal(func).__call__
		_gnomeui.gnome_mdi_generic_child_set_menu_creator(self._o, mfunc,
								  args)
	def set_config_func(self, func, *args):
		mfunc = self.__marshal(func).__call__
		_gnomeui.gnome_mdi_generic_child_set_config_func(self._o, mfunc,
								 args)
	def set_label_func(self, func, *args):
		mfunc = self.__marshal(func).__call__
		_gnomeui.gnome_mdi_generic_child_set_label_func(self._o, mfunc,
								args)
_gtk._name2cls['GnomeMDIGenericChild'] = GnomeMDIGenericChild

class GnomeMessageBox(GnomeDialog):
	get_type = _gnomeui.gnome_message_box_get_type
	def __init__(self, message='', messagebox_type='', b1=None, b2=None,
		     b3=None, b4=None, b5=None, b6=None, b7=None, b8=None,
		     b9=None, b10=None, _obj=None):
		if _obj: self._o = _obj; return
		buttons = filter(lambda x: x, (b1,b2,b3,b4,b5,b6,b7,b8,b9,b10))
		self._o = apply(_gnomeui.gnome_message_box_new,
				(message, messagebox_type) + buttons)
_gtk._name2cls['GnomeMessageBox'] = GnomeMessageBox

class GnomeNumberEntry(_gtk.GtkHBox):
	get_type = _gnomeui.gnome_number_entry_get_type
	def __init__(self, history_id=None, dlg_title=None, _obj=None):
		if _obj: self._o = _obj; return
		self._o = _gnomeui.gnome_number_entry_new(history_id,
							  dlg_title)
	def gnome_entry(self):
		return GnomeEntry(
			_obj=_gnomeui.gnome_number_entry_gnome_entry(self._o))
	def gtk_entry(self):
		return _gtk.GtkEntry(
			_obj=_gnomeui.gnome_number_entry_gtk_entry(self._o))
	def set_title(self, dlg_title):
		_gnomeui.gnome_number_entry_set_title(self._o, dlg_title)
	def get_number(self):
		return _gnomeui.gnome_number_entry_get_number(self._o)
_gtk._name2cls['GnomeNumberEntry'] = GnomeNumberEntry

class GnomePaperSelector(_gtk.GtkVBox):
	get_type = _gnomeui.gnome_paper_selector_get_type
	def __init__(self, _obj=None):
		if _obj: self._o = _obj; return
		self._o = _gnomeui.gnome_paper_selector_new()
	def get_name(self):
		return _gnomeui.gnome_paper_selector_get_name(self._o)
	def get_width(self):
		return _gnomeui.gnome_paper_selector_get_width(self._o)
	def get_height(self):
		return _gnomeui.gnome_paper_selector_get_height(self._o)
	def get_left_margin(self):
		return _gnomeui.gnome_paper_selector_get_left_margin(self._o)
	def get_right_margin(self):
		return _gnomeui.gnome_paper_selector_get_right_margin(self._o)
	def get_top_margin(self):
		return _gnomeui.gnome_paper_selector_get_top_margin(self._o)
	def get_bottom_margin(self):
		return _gnomeui.gnome_paper_selector_get_bottom_margin(self._o)
_gtk._name2cls['GnomePaperSelector'] = GnomePaperSelector

class GnomePixmap(_gtk.GtkWidget):
	get_type = _gnomeui.gnome_pixmap_get_type
	def __init__(self, filename='', width=-1, height=-1, _obj=None):
		if _obj: self._o = _obj; return
		if (width, height) == (-1, -1):
			if hasattr(filename, '_im'):
				self._o = _gnomeui.gnome_pixmap_new_from_imlib(
					filename._im)
			else:
				self._o = _gnomeui.gnome_pixmap_new_from_file(
					filename)
		else:
			if hasattr(filename, '_im'):
				self._o = _gnomeui.gnome_pixmap_new_from_imlib_at_size(
					filename._im, width, height)
			else:
				self._o = _gnomeui.gnome_pixmap_new_from_file_at_size(
					filename, width, height)
	def load_file(self, filename, width=-1, height=-1):
		if (width, height) == (-1, -1):
			_gnomeui.gnome_pixmap_load_file(self._o, filename)
		else:
			_gnomeui.gnome_pixmap_load_file_at_size(self._o,
						filename, width, height)
	def load_imlib(self, image, width=-1, height=-1):
		if (width, height) == (-1, -1):
			_gnomeui.gnome_pixmap_load_imlib(self._o, image._im)
		else:
			_gnomeui.gnome_pixmap_load_imlib_at_size(self._o,
						image._im, width, height)
_gtk._name2cls['GnomePixmap'] = GnomePixmap

class GnomePixmapEntry(_gtk.GtkVBox):
	get_type = _gnomeui.gnome_pixmap_entry_get_type
	def __init__(self, history_id='', browse_dialog_title='',
		     do_preview=_gtk.TRUE, _obj=None):
		if _obj: self._o = _obj; return
		self._o = _gnomeui.gnome_pixmap_entry_new(history_id,
							  browse_dialog_title,
							  do_preview)
	def set_pixmap_subdir(self, subdir):
		_gnomeui.gnome_pixmap_entry_set_pixmap_subdir(self._o, subdir)
	def gnome_file_entry(self):
		return GnomeFileEntry(_obj=
			_gnomeui.gnome_pixmap_entry_gnome_file_entry(self._o))
	def gnome_entry(self):
		return GnomeEntry(_obj=
			_gnomeui.gnome_pixmap_entry_gnome_entry(self._o))
	def gtk_entry(self):
		return _gtk.GtkEntry(_obj=
			_gnomeui.gnome_pixmap_entry_gtk_entry(self._o))
	def set_preview(self, do_preview):
		_gnomeui.gnome_pixmap_entry_set_preview(self._o, do_preview)
	def set_preview_size(self, width, height):
		_gnomeui.gnome_pixmap_entry_set_preview_size(self._o, width,
							     height)
	def get_filename(self):
		return _gnomeui.gnome_pixmap_entry_get_filename(self._o)
_gtk._name2cls['GnomePixmapEntry'] = GnomePixmapEntry

# The popup help stuff
def add_help(self, string, uidata=None):
	if uidata:
		_gnomeui.gnome_widget_add_help_with_uidata(self._o, string,
							   uidata)
	else:
		_gnomeui.gnome_widget_add_help(self._o, string)
_gtk.GtkWidget.__dict__['add_help'] = add_help
del add_help

# I think this will work now (hopefully)
class GnomePopupMenu(_gtk.GtkMenu):
	def __init__(self, uiinfo):
		self._o = _gnomeui.gnome_popup_menu_new(uiinfo)
	def attach(self, child):
		_gnomeui.gnome_popup_menu_attach(self._o, child._o)

class GnomePropertyBox(GnomeDialog):
	get_type = _gnomeui.gnome_property_box_get_type
	def __init__(self, _obj=None):
		if _obj: self._o = _obj; return
		self._o = _gnomeui.gnome_property_box_new()
	def changed(self, obj=None):
		_gnomeui.gnome_property_box_changed(self._o)
	def set_state(self, state):
		print "GnomePropertyBox.set_state deprecated -- use set_modified"
		self.set_modified(state)
	def set_modified(self, modified):
		_gnomeui.gnome_property_box_set_modified(self._o, modified)
	def append_page(self, child, tab_label):
		_gnomeui.gnome_property_box_append_page(self._o, child._o,
							tab_label._o)
_gtk._name2cls['GnomePropertyBox'] = GnomePropertyBox

class GnomeScores(GnomeDialog):
	get_type = _gnomeui.gnome_scores_get_type
	def __init__(self, scores=[], clear=_gtk.FALSE, _obj=None):
		if _obj: self._o = _obj; return
		self._o = _gnomeui.gnome_scores_new(scores, clear)
	def set_logo_label(self, txt, font, color):
		_gnomeui.gnome_scores_set_logo_label(self._o, txt, font, color)
	def set_logo_pixmap(self, logo):
		_gnomeui.gnome_scores_set_logo_pixmap(self._o, logo)
	def set_logo_widget(self, w):
		_gnomeui.gnome_scores_set_logo_widget(self._o, w._o)
	def set_color(self, pos, color):
		_gnomeui.gnome_scores_set_color(self._o, pos, color)
	def set_def_color(self, color):
		_gnomeui.gnome_scores_set_def_color(self._o, color)
	def set_logo_label_title(self, txt):
		_gnomeui.gnome_scores_set_logo_label_title(self._o, txt)
	def set_current_player(self, pos):
		_gnomeui.gnome_scores_set_current_player(self._o, pos)
_gtk._name2cls['GnomeScores'] = GnomeScores

def scores_display(title, app_name, level=None, pos=-1):
	_gnomeui.gnome_scores_display(title, app_name, level, pos)

class GnomeSpell(_gtk.GtkVBox):
	get_type = _gnomeui.gnome_spell_get_type
	def __init__(self, _obj=None):
		if _obj: self._o = _obj; return
		self._o = _gnomeui.gnome_spell_new()
	def check(self, string):
		return _gnomeui.gnome_spell_check(self._o, string)
	def accept(self, word):
		_gnomeui.gnome_spell_accept(self._o, word)
	def insert(self, word, lowercase):
		_gnomeui.gnome_spell_insert(self._o, word, lowercase)
	def next(self):
		_gnomeui.gnome_spell_next(self._o)
	def kill(self):
		_gnomeui.gnome_spell_kill(self._o)
_gtk._name2cls['GnomeSpell'] = GnomeSpell

class GnomeStock(GnomePixmap):
	get_type = _gnomeui.gnome_stock_get_type
	def __init__(self, icon=None, _obj=None):
		if _obj: self._o = _obj; return
		if icon:
			self._o = _gnomeui.gnome_stock_new_with_icon(icon)
		else:
			self._o = _gnomeui.gnome_stock_new()
	def set_icon(self, icon):
		_gnomeui.gnome_stock_set_icon(self._o, icon)
_gtk._name2cls['GnomeStock'] = GnomeStock

def GnomeStockPixmapWidget(win, icon=None):
	print "Deprecated -- use GnomeStock instead of GnomeStockPixmapWidget"
	return GnomeStock(icon)

class GnomePixmapButton(_gtk.GtkButton):
	def __init__(self, pixmap, text, _obj=None):
		self._o = _gnomeui.gnome_pixmap_button(pixmap._o, text)

# these aren't really different types, but having different classes is easier
class GnomeStockButton(_gtk.GtkButton):
	def __init__(self, type):
		self._o = _gnomeui.gnome_stock_or_ordinary_button(type)
class GnomeStockMenuItem(_gtk.GtkMenuItem):
	def __init__(self, type, text):
		self.type = type
		self._o = _gnomeui.gnome_stock_menu_item(type, text)
	def get_accel(self):
		return _gnomeui.gnome_stock_menu_accel(self.type)
class GnomeStockTransparentWindow(_gtk.GtkWindow):
	def __init__(self, icon, subtype):
		self._o = _gnomeui.gnome_stock_transparent_window(icon,subtype)

def stock_menu_accel(type):
	return _gnomeui.gnome_stock_menu_accel(type)
def stock_menu_accel_parse(section):
	_gnomeui.gnome_stock_menu_accel_parse(section)

class GtkClock(_gtk.GtkLabel):
	get_type = _gnomeui.gtk_clock_get_type
	def __init__(self, type=CLOCK_REALTIME, _obj=None):
		if _obj: self._o = _obj; return
		self._o = _gnomeui.gtk_clock_new(type)
	def set_format(self, fmt):
		_gnomeui.gtk_clock_set_format(self._o, fmt)
	def set_seconds(self, seconds):
		_gnomeui.gtk_clock_set_seconds(self._o, seconds)
	def set_update_interval(self, seconds):
		_gnomeui.gtk_clock_set_update_interval(self._o, seconds)
	def start(self):
		_gnomeui.gtk_clock_start(self._o)
	def stop(self):
		_gnomeui.gtk_clock_stop(self._o)
_gtk._name2cls['GtkClock'] = GtkClock

class GtkDial(_gtk.GtkWidget):
	get_type = _gnomeui.gtk_dial_get_type
	def __init__(self, adj=None, _obj=None):
		if _obj: self._o = _obj; return
		if adj: adj = adj._o
		self._o = _gnomeui.gtk_dial_new(adj)
	def get_adjustment(self):
		return _obj2inst(_gnomeui.gtk_dial_get_adjustment(self._o))
	def set_update_policy(self, policy):
		_gnomeui.gtk_dial_set_update_policy(self._o, policy)
	def set_adjustment(self, adj):
		_gnomeui.gtk_dial_set_adjustment(self._o, adj._o)
	def set_percentage(self, pcnt):
		return _gnomeui.gtk_dial_set_percentage(self._o, pcnt)
	def get_percentage(self):
		return _gnomeui.gtk_dial_get_percentage(self._o)
	def set_value(self, value):
		return _gnomeui.gtk_dial_set_value(self._o, value)
	def get_value(self):
		return _gnomeui.gtk_dial_get_value(self._o)
	def set_view_only(self, view_only):
		_gnomeui.gtk_dial_set_view_only(self._o, view_only)
_gtk._name2cls['GtkDial'] = GtkDial

class GtkTed(_gtk.GtkTable):
	get_type = _gnomeui.gtk_ted_get_type
	def __init__(self, dialog_name='', layout=None, _obj=None):
		# last unnamed widget number
		self.__last = 0

		if _obj: self._o = _obj; return

		if layout == None:
			self._o = _gnomeui.gtk_ted_new(dialog_name)
		else:
			self._o = _gnomeui.gtk_ted_new_layout(dialog_name,
							      layout)
	def prepare(self):
		_gnomeui.gtk_ted_prepare(self._o)
	def add(self, child, name=None):
		if name == None:
			name = 'wid' + str(self.__last)
			self.__last = self.__last + 1
		_gnomeui.gtk_ted_add(self._o, child._o, name)
_gtk._name2cls['GtkTed'] = GtkTed


def dialog_cauldron(title, options, fmt, *args):
	return _gnomeui.gtk_dialog_cauldron(title, options, fmt, args)
