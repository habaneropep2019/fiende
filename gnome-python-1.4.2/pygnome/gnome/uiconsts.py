# the GnomeUIInfo related stuff
APP_UI_ENDOFINFO          = 0
APP_UI_ITEM               = 1
APP_UI_TOGGLEITEM         = 2
APP_UI_RADIOITEMS         = 3
APP_UI_SUBTREE            = 4
APP_UI_SEPARATOR          = 5
APP_UI_HELP               = 6
#APP_UI_BUILDER_DATA       = 7
APP_UI_ITEM_CONFIGURABLE  = 8
APP_UI_ITEM_SUBTREE_STOCK = 9

APP_PIXMAP_NONE     = 0
APP_PIXMAP_STOCK    = 1
#APP_PIXMAP_DATA     = 2 # not supported by pygnome
APP_PIXMAP_FILENAME = 3

# UIInfo entries are (type, label, tooltip, callback, cbdata,
#                     pix_type, pixmap, accelerator_key, accelerator_mods)

UIINFO_END       = (APP_UI_ENDOFINFO, None, None, None, None,
		    APP_PIXMAP_NONE, None, 0, 0)
UIINFO_SEPARATOR = (APP_UI_SEPARATOR, None, None, None, None,
		    APP_PIXMAP_NONE, None, 0, 0)
def UIINFO_ITEM(label, tip=None, cb=None, image=None):
	if image:
		return (APP_UI_ITEM, label, tip, cb, None,
			APP_PIXMAP_FILENAME, image, 0, 0)
	else:
		return (APP_UI_ITEM, label, tip, cb, None,
			APP_PIXMAP_NONE, None, 0, 0)
def UIINFO_ITEM_STOCK(label, tip=None, cb=None, stock=None):
	return (APP_UI_ITEM, label, tip, cb, None,
		APP_PIXMAP_STOCK, stock, 0, 0)
	
def UIINFO_TOGGLEITEM(label, tip=None, cb=None, image=None):
	if image:
		return (APP_UI_TOGGLEITEM, label, tip, cb, None,
			APP_PIXMAP_FILENAME, image, 0, 0)
	else:
		return (APP_UI_TOGGLEITEM, label, tip, cb, None,
			APP_PIXMAP_NONE, None, 0, 0)
def UIINFO_HELP(name):
	return (APP_UI_HELP, None, None, name, None,
		APP_PIXMAP_NONE, None, 0, 0)
def UIINFO_SUBTREE(label, tree):
	return (APP_UI_SUBTREE, label, None, tree, None,
		APP_PIXMAP_NONE, None, 0, 0)
def UIINFO_SUBTREE_HINT(label, hint, tree):
	return (APP_UI_SUBTREE, label, hint, tree, None,
		APP_PIXMAP_NONE, None, 0, 0)
def UIINFO_SUBTREE_STOCK(label, tree, image):
	return (APP_UI_SUBTREE, label, None, tree, None,
		APP_PIXMAP_STOCK, image, 0, 0)
def UIINFO_RADIOLIST(list):
	return (APP_UI_RADIOITEMS, None, None, list, None,
		APP_PIXMAP_NONE, None, 0, 0)
def UIINFO_RADIOITEM(label, tip=None, cb=None, image=None):
	print "Deprecated -- just use UIINFO_ITEM for radio list items"
	if image:
		return (APP_UI_ITEM, label, tip, cb, None,
			APP_PIXMAP_FILENAME, image, 0, 0)
	else:
		return (APP_UI_ITEM, label, tip, cb, None,
			APP_PIXMAP_NONE, None, 0, 0)

_APP_CONFIGURABLE_ITEM_NEW          = 0
_APP_CONFIGURABLE_ITEM_OPEN         = 1
_APP_CONFIGURABLE_ITEM_SAVE         = 2
_APP_CONFIGURABLE_ITEM_SAVE_AS      = 3
_APP_CONFIGURABLE_ITEM_REVERT       = 4
_APP_CONFIGURABLE_ITEM_PRINT        = 5
_APP_CONFIGURABLE_ITEM_PRINT_SETUP  = 6
_APP_CONFIGURABLE_ITEM_CLOSE        = 7
_APP_CONFIGURABLE_ITEM_EXIT         = 8
_APP_CONFIGURABLE_ITEM_CUT          = 9
_APP_CONFIGURABLE_ITEM_COPY         = 10
_APP_CONFIGURABLE_ITEM_PASTE        = 11
_APP_CONFIGURABLE_ITEM_CLEAR        = 12
_APP_CONFIGURABLE_ITEM_UNDO         = 13
_APP_CONFIGURABLE_ITEM_REDO         = 14
_APP_CONFIGURABLE_ITEM_FIND         = 15
_APP_CONFIGURABLE_ITEM_FIND_AGAIN   = 16
_APP_CONFIGURABLE_ITEM_REPLACE      = 17
_APP_CONFIGURABLE_ITEM_PROPERTIES   = 18
_APP_CONFIGURABLE_ITEM_PREFERENCES  = 19
_APP_CONFIGURABLE_ITEM_ABOUT        = 20
_APP_CONFIGURABLE_ITEM_SELECT_ALL   = 21
_APP_CONFIGURABLE_ITEM_NEW_WINDOW   = 22
_APP_CONFIGURABLE_ITEM_CLOSE_WINDOW = 23
_APP_CONFIGURABLE_ITEM_NEW_GAME     = 24
_APP_CONFIGURABLE_ITEM_PAUSE_GAME   = 25
_APP_CONFIGURABLE_ITEM_RESTART_GAME = 26
_APP_CONFIGURABLE_ITEM_UNDO_MOVE    = 27
_APP_CONFIGURABLE_ITEM_REDO_MOVE    = 28
_APP_CONFIGURABLE_ITEM_HINT         = 29
_APP_CONFIGURABLE_ITEM_SCORES       = 30
_APP_CONFIGURABLE_ITEM_END_GAME     = 31

def UIINFO_MENU_NEW_ITEM(label, tip, cb=None, data=None):
	return (APP_UI_ITEM_CONFIGURABLE, label, tip, cb, data,
		APP_PIXMAP_NONE, None, _APP_CONFIGURABLE_ITEM_NEW, 0)
def UIINFO_MENU_OPEN_ITEM(cb=None, data=None):
	return (APP_UI_ITEM_CONFIGURABLE, None, None, cb, data,
		APP_PIXMAP_NONE, None, _APP_CONFIGURABLE_ITEM_OPEN, 0)
def UIINFO_MENU_SAVE_ITEM(cb=None, data=None):
	return (APP_UI_ITEM_CONFIGURABLE, None, None, cb, data,
		APP_PIXMAP_NONE, None, _APP_CONFIGURABLE_ITEM_SAVE, 0)
def UIINFO_MENU_SAVE_AS_ITEM(cb=None, data=None):
	return (APP_UI_ITEM_CONFIGURABLE, None, None, cb, data,
		APP_PIXMAP_NONE, None, _APP_CONFIGURABLE_ITEM_SAVE_AS, 0)
def UIINFO_MENU_REVERT_ITEM(cb=None, data=None):
	return (APP_UI_ITEM_CONFIGURABLE, None, None, cb, data,
		APP_PIXMAP_NONE, None, _APP_CONFIGURABLE_ITEM_REVERT, 0)
def UIINFO_MENU_PRINT_ITEM(cb=None, data=None):
	return (APP_UI_ITEM_CONFIGURABLE, None, None, cb, data,
		APP_PIXMAP_NONE, None, _APP_CONFIGURABLE_ITEM_PRINT, 0)
def UIINFO_MENU_PRINT_SETUP_ITEM(cb=None, data=None):
	return (APP_UI_ITEM_CONFIGURABLE, None, None, cb, data,
		APP_PIXMAP_NONE, None, _APP_CONFIGURABLE_ITEM_PRINT_SETUP, 0)
def UIINFO_MENU_CLOSE_ITEM(cb=None, data=None):
	return (APP_UI_ITEM_CONFIGURABLE, None, None, cb, data,
		APP_PIXMAP_NONE, None, _APP_CONFIGURABLE_ITEM_CLOSE, 0)
def UIINFO_MENU_EXIT_ITEM(cb=None, data=None):
	return (APP_UI_ITEM_CONFIGURABLE, None, None, cb, data,
		APP_PIXMAP_NONE, None, _APP_CONFIGURABLE_ITEM_EXIT, 0)
def UIINFO_MENU_CUT_ITEM(cb=None, data=None):
	return (APP_UI_ITEM_CONFIGURABLE, None, None, cb, data,
		APP_PIXMAP_NONE, None, _APP_CONFIGURABLE_ITEM_CUT, 0)
def UIINFO_MENU_COPY_ITEM(cb=None, data=None):
	return (APP_UI_ITEM_CONFIGURABLE, None, None, cb, data,
		APP_PIXMAP_NONE, None, _APP_CONFIGURABLE_ITEM_COPY, 0)
def UIINFO_MENU_PASTE_ITEM(cb=None, data=None):
	return (APP_UI_ITEM_CONFIGURABLE, None, None, cb, data,
		APP_PIXMAP_NONE, None, _APP_CONFIGURABLE_ITEM_PASTE, 0)
def UIINFO_MENU_SELECT_ALL_ITEM(cb=None, data=None):
	return (APP_UI_ITEM_CONFIGURABLE, None, None, cb, data,
		APP_PIXMAP_NONE, None, _APP_CONFIGURABLE_ITEM_SELECT_ALL, 0)
def UIINFO_MENU_CLEAR_ITEM(cb=None, data=None):
	return (APP_UI_ITEM_CONFIGURABLE, None, None, cb, data,
		APP_PIXMAP_NONE, None, _APP_CONFIGURABLE_ITEM_CLEAR, 0)
def UIINFO_MENU_UNDO_ITEM(cb=None, data=None):
	return (APP_UI_ITEM_CONFIGURABLE, None, None, cb, data,
		APP_PIXMAP_NONE, None, _APP_CONFIGURABLE_ITEM_UNDO, 0)
def UIINFO_MENU_REDO_ITEM(cb=None, data=None):
	return (APP_UI_ITEM_CONFIGURABLE, None, None, cb, data,
		APP_PIXMAP_NONE, None, _APP_CONFIGURABLE_ITEM_REDO, 0)
def UIINFO_MENU_FIND_ITEM(cb=None, data=None):
	return (APP_UI_ITEM_CONFIGURABLE, None, None, cb, data,
		APP_PIXMAP_NONE, None, _APP_CONFIGURABLE_ITEM_FIND, 0)
def UIINFO_MENU_FIND_AGAIN_ITEM(cb=None, data=None):
	return (APP_UI_ITEM_CONFIGURABLE, None, None, cb, data,
		APP_PIXMAP_NONE, None, _APP_CONFIGURABLE_ITEM_FIND_AGAIN, 0)
def UIINFO_MENU_REPLACE_ITEM(cb=None, data=None):
	return (APP_UI_ITEM_CONFIGURABLE, None, None, cb, data,
		APP_PIXMAP_NONE, None, _APP_CONFIGURABLE_ITEM_REPLACE, 0)
def UIINFO_MENU_PROPERTIES_ITEM(cb=None, data=None):
	return (APP_UI_ITEM_CONFIGURABLE, None, None, cb, data,
		APP_PIXMAP_NONE, None, _APP_CONFIGURABLE_ITEM_PROPERTIES, 0)
def UIINFO_MENU_PREFERENCES_ITEM(cb=None, data=None):
	return (APP_UI_ITEM_CONFIGURABLE, None, None, cb, data,
		APP_PIXMAP_NONE, None, _APP_CONFIGURABLE_ITEM_PREFERENCES, 0)
def UIINFO_MENU_NEW_WINDOW_ITEM(cb=None, data=None):
	return (APP_UI_ITEM_CONFIGURABLE, None, None, cb, data,
		APP_PIXMAP_NONE, None, _APP_CONFIGURABLE_ITEM_NEW_WINDOW, 0)
def UIINFO_MENU_CLOSE_WINDOW_ITEM(cb=None, data=None):
	return (APP_UI_ITEM_CONFIGURABLE, None, None, cb, data,
		APP_PIXMAP_NONE, None, _APP_CONFIGURABLE_ITEM_CLOSE_WINDOW, 0)
def UIINFO_MENU_ABOUT_ITEM(cb=None, data=None):
	return (APP_UI_ITEM_CONFIGURABLE, None, None, cb, data,
		APP_PIXMAP_NONE, None, _APP_CONFIGURABLE_ITEM_ABOUT, 0)
def UIINFO_MENU_NEW_GAME_ITEM(cb=None, data=None):
	return (APP_UI_ITEM_CONFIGURABLE, None, None, cb, data,
		APP_PIXMAP_NONE, None, _APP_CONFIGURABLE_ITEM_NEW_GAME, 0)
def UIINFO_MENU_PAUSE_GAME_ITEM(cb=None, data=None):
	return (APP_UI_ITEM_CONFIGURABLE, None, None, cb, data,
		APP_PIXMAP_NONE, None, _APP_CONFIGURABLE_ITEM_PAUSE_GAME, 0)
def UIINFO_MENU_RESTART_GAME_ITEM(cb=None, data=None):
	return (APP_UI_ITEM_CONFIGURABLE, None, None, cb, data,
		APP_PIXMAP_NONE, None, _APP_CONFIGURABLE_ITEM_RESTART_GAME, 0)
def UIINFO_MENU_UNDO_MOVE_ITEM(cb=None, data=None):
	return (APP_UI_ITEM_CONFIGURABLE, None, None, cb, data,
		APP_PIXMAP_NONE, None, _APP_CONFIGURABLE_ITEM_UNDO_MOVE, 0)
def UIINFO_MENU_REDO_MOVE_ITEM(cb=None, data=None):
	return (APP_UI_ITEM_CONFIGURABLE, None, None, cb, data,
		APP_PIXMAP_NONE, None, _APP_CONFIGURABLE_ITEM_REDO_MOVE, 0)
def UIINFO_MENU_HINT_ITEM(cb=None, data=None):
	return (APP_UI_ITEM_CONFIGURABLE, None, None, cb, data,
		APP_PIXMAP_NONE, None, _APP_CONFIGURABLE_ITEM_HINT, 0)
def UIINFO_MENU_SCORES_ITEM(cb=None, data=None):
	return (APP_UI_ITEM_CONFIGURABLE, None, None, cb, data,
		APP_PIXMAP_NONE, None, _APP_CONFIGURABLE_ITEM_SCORES, 0)
def UIINFO_MENU_END_GAME_ITEM(cb=None, data=None):
	return (APP_UI_ITEM_CONFIGURABLE, None, None, cb, data,
		APP_PIXMAP_NONE, None, _APP_CONFIGURABLE_ITEM_END_GAME, 0)

INTERACT_NONE   = 0
INTERACT_ERRORS = 1
INTERACT_ANY    = 2
DIALOG_ERROR  = 0
DIALOG_NORMAL = 1
SAVE_GLOBAL = 0
SAVE_LOCAL  = 1
SAVE_BOTH   = 2
RESTART_IF_RUNNING  = 0
RESTART_ANYWAY      = 1
RESTART_IMMEDIATELY = 2
RESTART_NEVER       = 3

CLIENT_IDLE                = 0
CLIENT_SAVING_PHASE_1      = 1
CLIENT_WAITING_FOR_PHASE_2 = 2
CLIENT_SAVING_PHASE_2      = 3
CLIENT_FROZEN              = 4
CLIENT_DISCONNECTED        = 5
CLIENT_REGISTERING         = 6

ICON_LIST_ICONS      = 0
ICON_LIST_TEXT_BELOW = 1
ICON_LIST_TEXT_RIGHT = 2

MESSAGE_BOX_INFO     = "info"
MESSAGE_BOX_WARNING  = "warning"
MESSAGE_BOX_ERROR    = "error"
MESSAGE_BOX_QUESTION = "question"
MESSAGE_BOX_GENERIC  = "generic"

STOCK_PIXMAP_NEW         = "New"
STOCK_PIXMAP_OPEN        = "Open"
STOCK_PIXMAP_CLOSE       = "Close"
STOCK_PIXMAP_REVERT      = "Revert"
STOCK_PIXMAP_SAVE        = "Save"
STOCK_PIXMAP_SAVE_AS     = "Save As"
STOCK_PIXMAP_CUT         = "Cut"
STOCK_PIXMAP_COPY        = "Copy"
STOCK_PIXMAP_PASTE       = "Paste"
STOCK_PIXMAP_CLEAR       = "Clear"
STOCK_PIXMAP_PROPERTIES  = "Properties"
STOCK_PIXMAP_PREFERENCES = "Preferences"
STOCK_PIXMAP_HELP        = "Help"
STOCK_PIXMAP_SCORES      = "Scores"
STOCK_PIXMAP_PRINT       = "Print"
STOCK_PIXMAP_SEARCH      = "Search"
STOCK_PIXMAP_SRCHRPL     = "Search/Replace"
STOCK_PIXMAP_BACK        = "Back"
STOCK_PIXMAP_FORWARD     = "Forward"
STOCK_PIXMAP_FIRST       = "First"
STOCK_PIXMAP_LAST        = "Last"
STOCK_PIXMAP_HOME        = "Home"
STOCK_PIXMAP_STOP        = "Stop"
STOCK_PIXMAP_REFRESH     = "Refresh"
STOCK_PIXMAP_UNDO        = "Undo"
STOCK_PIXMAP_REDO        = "Redo"
STOCK_PIXMAP_TIMER       = "Timer"
STOCK_PIXMAP_TIMER_STOP  = "Timer Stopped"
STOCK_PIXMAP_MAIL        = "Mail"
STOCK_PIXMAP_MAIL_RCV    = "Receive Mail"
STOCK_PIXMAP_MAIL_SND    = "Send Mail"
STOCK_PIXMAP_MAIL_RPL    = "Reply to Mail"
STOCK_PIXMAP_MAIL_FWD    = "Forward Mail"
STOCK_PIXMAP_MAIL_NEW    = "New Mail"
STOCK_PIXMAP_TRASH       = "Trash"
STOCK_PIXMAP_TRASH_FULL  = "Trash Full"
STOCK_PIXMAP_UNDELETE    = "Undelete"
STOCK_PIXMAP_SPELLCHECK  = "Spellchecker"
STOCK_PIXMAP_MIC         = "Microphone"
STOCK_PIXMAP_LINE_IN     = "Line In"
STOCK_PIXMAP_CDROM       = "Cdrom"
STOCK_PIXMAP_VOLUME      = "Volume"
STOCK_PIXMAP_MIDI        = "Midi"
STOCK_PIXMAP_BOOK_RED    = "Book Red"
STOCK_PIXMAP_BOOK_GREEN  = "Book Green"
STOCK_PIXMAP_BOOK_BLUE   = "Book Blue"
STOCK_PIXMAP_BOOK_YELLOW = "Book Yellow"
STOCK_PIXMAP_BOOK_OPEN   = "Book Open"
STOCK_PIXMAP_ABOUT       = "About"
STOCK_PIXMAP_QUIT        = "Quit"
STOCK_PIXMAP_MULTIPLE    = "Multiple"
STOCK_PIXMAP_NOT         = "Not"
STOCK_PIXMAP_CONVERT     = "Convert"
STOCK_PIXMAP_JUMP_TO     = "Jump To"
STOCK_PIXMAP_UP          = "Up"
STOCK_PIXMAP_DOWN        = "Down"
STOCK_PIXMAP_TOP         = "Top"
STOCK_PIXMAP_BOTTOM      = "Bottom"
STOCK_PIXMAP_ATTACH      = "Attach"
STOCK_PIXMAP_INDEX       = "Index"
STOCK_PIXMAP_FONT        = "Font"
STOCK_PIXMAP_EXEC        = "Exec"
STOCK_PIXMAP_ALIGN_LEFT     = "Left"
STOCK_PIXMAP_ALIGN_RIGHT    = "Right"
STOCK_PIXMAP_ALIGN_CENTER   = "Center"
STOCK_PIXMAP_ALIGN_JUSTIFY  = "Justify"
STOCK_PIXMAP_TEXT_BOLD      = "Bold"
STOCK_PIXMAP_TEXT_ITALIC    = "Italic"
STOCK_PIXMAP_TEXT_UNDERLINE = "Underline"
STOCK_PIXMAP_TEXT_STRIKEOUT = "Strikeout"
STOCK_PIXMAP_TEXT_INDENT    = "Text Indent"
STOCK_PIXMAP_TEXT_UNINDENT  = "Text Unindent"
STOCK_PIXMAP_ADD         = "Add"
STOCK_PIXMAP_REMOVE      = "Remove"
STOCK_PIXMAP_TABLE_BORDERS = "Table Borders"
STOCK_PIXMAP_TABLE_FILL    = "Table Fill"
STOCK_PIXMAP_TEXT_BULLETED_LIST = "Text Bulleted List"
STOCK_PIXMAP_TEXT_NUMBERED_LIST = "Text Numbered List"

STOCK_PIXMAP_EXIT        = STOCK_PIXMAP_QUIT

STOCK_PIXMAP_REGULAR  = "regular"
STOCK_PIXMAP_DISABLED = "disabled"
STOCK_PIXMAP_FOCUSED  = "focused"

STOCK_BUTTON_OK       = "Button_Ok"
STOCK_BUTTON_CANCEL   = "Button_Cancel"
STOCK_BUTTON_YES      = "Button_Yes"
STOCK_BUTTON_NO       = "Button_No"
STOCK_BUTTON_CLOSE    = "Button_Close"
STOCK_BUTTON_APPLY    = "Button_Apply"
STOCK_BUTTON_HELP     = "Button_Help"
STOCK_BUTTON_NEXT     = "Button_Next"
STOCK_BUTTON_PREV     = "Button_Prev"

STOCK_MENU_BLANK      = "Menu_"
STOCK_MENU_NEW        = "Menu_New"
STOCK_MENU_SAVE       = "Menu_Save"
STOCK_MENU_SAVE_AS    = "Menu_Save As"
STOCK_MENU_REVERT     = "Menu_Revert"
STOCK_MENU_OPEN       = "Menu_Open"
STOCK_MENU_CLOSE      = "Menu_Close"
STOCK_MENU_QUIT       = "Menu_Quit"
STOCK_MENU_CUT        = "Menu_Cut"
STOCK_MENU_COPY       = "Menu_Copy"
STOCK_MENU_PASTE      = "Menu_Paste"
STOCK_MENU_PROP       = "Menu_Properties"
STOCK_MENU_PREF       = "Menu_Preferences"
STOCK_MENU_ABOUT      = "Menu_About"
STOCK_MENU_SCORES     = "Menu_Scores"
STOCK_MENU_UNDO       = "Menu_Undo"
STOCK_MENU_REDO       = "Menu_Redo"
STOCK_MENU_PRINT      = "Menu_Print"
STOCK_MENU_SEARCH     = "Menu_Search"
STOCK_MENU_SRCHRPL    = "Menu_Search/Replace"
STOCK_MENU_BACK       = "Menu_Back"
STOCK_MENU_FORWARD    = "Menu_Forward"
STOCK_MENU_FIRST      = "Menu_First"
STOCK_MENU_LAST       = "Menu_Last"
STOCK_MENU_HOME       = "Menu_Home"
STOCK_MENU_STOP       = "Menu_Stop"
STOCK_MENU_REFRESH    = "Menu_Refresh"
STOCK_MENU_MAIL       = "Menu_Mail"
STOCK_MENU_MAIL_RCV   = "Menu_Receive Mail"
STOCK_MENU_MAIL_SND   = "Menu_Send Mail"
STOCK_MENU_MAIL_RPL   = "Menu_Reply to Mail"
STOCK_MENU_MAIL_FWD   = "Menu_Forward Mail"
STOCK_MENU_MAIL_NEW   = "Menu_New Mail"
STOCK_MENU_TRASH      = "Menu_Trash"
STOCK_MENU_TRASH_FULL = "Menu_Trash Full"
STOCK_MENU_UNDELETE   = "Menu_Undelete"
STOCK_MENU_TIMER      = "Menu_Timer"
STOCK_MENU_TIMER_STOP = "Menu_Timer Stopped"
STOCK_MENU_SPELLCHECK = "Menu_Spellchecker"
STOCK_MENU_MIC        = "Menu_Microphone"
STOCK_MENU_LINE_IN    = "Menu_Line In"
STOCK_MENU_CDROM      = "Menu_Cdrom"
STOCK_MENU_VOLUME     = "Menu_Volume"
STOCK_MENU_MIDI       = "Menu_Midi"
STOCK_MENU_BOOK_RED   = "Menu_Book Red"
STOCK_MENU_BOOK_GREEN = "Menu_Book Green"
STOCK_MENU_BOOK_BLUE  = "Menu_Book Blue"
STOCK_MENU_BOOK_YELLOW= "Menu_Book Yellow"
STOCK_MENU_BOOK_OPEN  = "Menu_Book Open"
STOCK_MENU_CONVERT    = "Menu_Convert"
STOCK_MENU_JUMP_TO    = "Menu_Jump To"
STOCK_MENU_UP          = "Menu_Up"
STOCK_MENU_DOWN        = "Menu_Down"
STOCK_MENU_TOP         = "Menu_Top"
STOCK_MENU_BOTTOM      = "Menu_Bottom"
STOCK_MENU_ATTACH      = "Menu_Attach"
STOCK_MENU_INDEX       = "Menu_Index"
STOCK_MENU_FONT        = "Menu_Font"
STOCK_MENU_EXEC        = "Menu_Exec"
STOCK_MENU_ALIGN_LEFT     = "Menu_Left"
STOCK_MENU_ALIGN_RIGHT    = "Menu_Right"
STOCK_MENU_ALIGN_CENTER   = "Menu_Center"
STOCK_MENU_ALIGN_JUSTIFY  = "Menu_Justify"
STOCK_MENU_TEXT_BOLD      = "Menu_Bold"
STOCK_MENU_TEXT_ITALIC    = "Menu_Italic"
STOCK_MENU_TEXT_UNDERLINE = "Menu_Underline"
STOCK_MENU_TEXT_STRIKEOUT = "Menu_Strikeout"

STOCK_MENU_EXIT     = STOCK_MENU_QUIT

PAD       = 8
PAD_SMALL = 4
PAD_BIG   = 12

CLOCK_INCREASING = 0
CLOCK_DECREASING = 1
CLOCK_REALTIME   = 2

YES = 0
NO = 1
OK = 0
CANCEL = 1

MDI_NOTEBOOK     = 0
MDI_TOPLEVEL     = 1
MDI_MODAL        = 2
MDI_MS           = 3  # I don't think this is implemented yet */
MDI_DEFAULT_MODE = 42


PREFERENCES_NEVER  = 0
PREFERENCES_USER   = 1
PREFERENCES_ALWAYS = 2

CAULDRON_ENTER = "GTK_CAULDRON_ENTER"
CAULDRON_ESCAPE = "GTK_CAULDRON_ESCAPE"

CAULDRON_TOPLEVEL     = 1 << 0
CAULDRON_DIALOG       = 1 << 1
CAULDRON_POPUP        = 1 << 2
CAULDRON_SPACE1       = 1 << 3
CAULDRON_SPACE2       = 2 << 3
CAULDRON_SPACE3       = 3 << 3
CAULDRON_SPACE4       = 4 << 3
CAULDRON_SPACE5       = 5 << 3
CAULDRON_SPACE6       = 6 << 3
CAULDRON_SPACE7       = 7 << 3
CAULDRON_SPACE8       = 8 << 3
CAULDRON_SPACE9       = 9 << 3
CAULDRON_SPACE10      = 10 << 3
CAULDRON_SPACE11      = 11 << 3
CAULDRON_SPACE12      = 12 << 3
CAULDRON_SPACE13      = 13 << 3
CAULDRON_SPACE14      = 14 << 3
CAULDRON_SPACE15      = 15 << 3
CAULDRON_IGNOREESCAPE = 1 << 7
CAULDRON_IGNOREENTER  = 1 << 8
CAULDRON_GRAB         = 1 << 9

FONT_PICKER_MODE_PIXMAP      = 0
FONT_PICKER_MODE_FONT_INFO   = 1
FONT_PICKER_MODE_USER_WIDGET = 2

ANIMATOR_STATUS_STOPPED = 0
ANIMATOR_STATUS_RUNNING = 1

DOCK_TOP      = 0
DOCK_RIGHT    = 1
DOCK_BOTTOM   = 2
DOCK_LEFT     = 3
DOCK_FLOATING = 4

DOCK_ITEM_BEH_NORMAL           = 0
DOCK_ITEM_BEH_EXCLUSIVE        = 1 << 0
DOCK_ITEM_BEH_NEVER_FLOATING   = 1 << 1
DOCK_ITEM_BEH_NEVER_VERTICAL   = 1 << 2
DOCK_ITEM_BEH_NEVER_HORIZONTAL = 1 << 3
DOCK_ITEM_BEH_LOCKED           = 1 << 4

CLIENT_IS_CONNECTED = 1 << 0
CLIENT_RESTARTED    = 1 << 1
CLIENT_RESTORED     = 1 << 2

DATE_EDIT_SHOW_TIME             = 1 << 0
DATE_EDIT_24_HR                 = 1 << 1
DATE_EDIT_WEEK_STARTS_ON_MONDAY = 1 << 2