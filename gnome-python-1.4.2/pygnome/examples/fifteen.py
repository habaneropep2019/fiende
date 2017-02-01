#!/usr/bin/env python

# Author: Chi-Deok Hwang <cdhwang@sr.hei.co.kr>
# A translation of the puzzle example using the canvas.

from gtk import *
from gnome.ui import *
from GDK import *
import whrandom

PIECE_SIZE = 50

def test_win (board):
	for i in range(15):
		if not board[i] or board[i].get_data('piece_num') != i: return
	dlg=GnomeOkDialog('You stud, you win!')
	dlg.set_modal(TRUE)
	dlg.run()
	return
def get_piece_color(piece):
	y,x=divmod(piece, 4)
	r = ((4-x) * 255) / 4
	g = ((4-y) * 255) / 4
	b = 128
	return "#%02x%02x%02x" % (r, g, b)

def piece_event (item, event, canvas):
	if event.type == ENTER_NOTIFY:
		text = item.get_data ('text')
		text.set (fill_color='white')
	elif event.type == LEAVE_NOTIFY:
		text = item.get_data ('text')
		text.set (fill_color='black')
	elif event.type == BUTTON_PRESS:
		board = canvas.get_data ('board')
		num = item.get_data ('piece_num')
		pos = item.get_data ('piece_pos')
		y,x = divmod(pos, 4)
		move = TRUE
		if y > 0 and not board[(y-1)*4+x]:
			dx = 0.0; dy = -1.0; y = y - 1
		elif y < 3 and not board[(y+1)*4+x]:
			dx = 0.0; dy = 1.0; y = y + 1
		elif x > 0 and not board[y*4 + x-1]:
			dx = -1.0;dy = 0.0; x = x - 1
		elif x < 3 and not board[y*4 + x + 1]:
			dx = 1.0;dy = 0.0;x = x + 1
		else: 
			move = FALSE
		if move:
			newpos = y*4 + x
			board[pos] = None
			board[newpos] = item
			item.move (dx*PIECE_SIZE, dy*PIECE_SIZE)
			item.set_data ('piece_pos', newpos)
			test_win(board)
	return 1

def scramble(obj, canvas):
	SCRAMBLE_MOVES = 256
	board = canvas.get_data('board')
	pos = board.index(None)
	for i in range(SCRAMBLE_MOVES):
		while 1:
			dir = whrandom.randint(0,3)
			x = y = 0
			if dir == 0 and pos > 3:
				y = -1
			elif dir == 1 and pos < 12:
				y = 1
			elif dir == 2 and pos%4:
				x = -1
			elif dir == 3 and pos%4 != 3:
				x = 1
			else:
				continue
			break
		oldpos = pos + y*4 + x
		board[pos] = board[oldpos]
		board[oldpos] = None
		board[pos].set_data('piece_pos', pos)
		board[pos].move (-x*PIECE_SIZE,-y*PIECE_SIZE)
		canvas.update_now()
		pos = oldpos


def create_fifteen():
	win = GtkWindow()
	win.connect('destroy', mainquit)
	win.set_title('Canvas Fifteen')

	vbox = GtkVBox(spacing=4)
	vbox.set_border_width(4)
	win.add (vbox)

	align = GtkAlignment(xa=0.5,ya=0.5)
	vbox.pack_start(align)

	frame = GtkFrame()
	frame.set_shadow_type(SHADOW_IN)
	align.add (frame)

	canvas = GnomeCanvas()
	canvas.set_usize (PIECE_SIZE * 4 + 1, PIECE_SIZE * 4 + 1)
	canvas.set_scroll_region (0, 0, PIECE_SIZE * 4 + 1, PIECE_SIZE * 4 + 1)
	frame.add (canvas)

	board = []
	canvas.set_data ('board', board)
	root = canvas.root()
	for i in range(15):
		y,x = divmod(i, 4)
		group = root.add('group')
		group.move(x*PIECE_SIZE, y*PIECE_SIZE)
		board.append(group)
		group.add('rect', x1=0.0,y1=0.0,
			  x2=PIECE_SIZE,y2=PIECE_SIZE,
			  fill_color=get_piece_color(i),
			  outline_color='black',
			  width_pixels=0)
		text = group.add('text', text="%d" % (i + 1),
			  x=PIECE_SIZE/2.0, y=PIECE_SIZE/2.0,
			  font='-adobe-helvetica-bold-r-normal--24-240-75-75-p-*-iso8859-1',
			  anchor=ANCHOR_CENTER,
			  fill_color='black') 
		group.set_data ('piece_num', i)
		group.set_data ('piece_pos', i)
		group.set_data ('text', text)
		group.connect ('event', piece_event, canvas)
	board.append (None)

	button = GtkButton('Scramble')
	vbox.pack_start (button, expand=FALSE, fill=FALSE)
	button.set_data ('board', board)
	button.connect ('clicked', scramble, canvas)

	win.show_all()

create_fifteen()

mainloop()

