#include "objects.h"
#include <glib.h>
/* for argp */
#include <libgnome/libgnome.h>

Horde bill;
Network net;
Library OS;
Bucket bucket;
Spark spark;
Game game;
UI ui;

int Game::RAND(int lb, int ub) {
	return (rand()%(ub-lb+1) + lb);
}

int Game::MAX(int x, int y) {
	return (x>y ? x : y);
}

int Game::MIN(int x, int y) {
	return (x<y ? x : y);
}

int Game::INTERSECT(int x1, int y1, int w1, int h1, int x2, int y2, int w2,
	int h2)
{
	return (((x2-x1<=w1 && x2-x1>=0) || (x1-x2<=w2 && x1-x2>=0))
		&& ((y2-y1<=h1 && y2-y1>=0) || (y1-y2<=h2 && y1-y2>=0)));
}

void Game::setup_level (int lev) {
	level = lev;
	bill.setup();
	grabbed = EMPTY;
	ui.set_cursor(DEFAULTC);
	net.setup();
	iteration = efficiency = 0;
}

void Game::start(int lev) {
	/* Looks like the high number we can get is 8+3*level, so ensure
	   G_MAXINT is not going to be passed and crash */

	if (lev > G_MAXINT/8+3) return;

	state = PLAYING;
	score = 0;
	ui.restart_timer();
	ui.set_pausebutton(TRUE);
	setup_level(lev);
}

void Game::quit() {
	exit(0);
}

void Game::update_info() {
	static char str[80];
	sprintf (str, "Bill:%d/%d  System:%d/%d/%d  Level: %d Score:%d",
		bill.on_screen, bill.off_screen, net.base, net.off,
		net.win, level, score);
	ui.draw_str(str, 5, scrheight-5);
	efficiency += ((100*net.base-10*net.win)/net.units);
}
  
void Game::update_score (int action) {
	switch (action){
		case ENDLEVEL: score+=(level*efficiency/iteration); break;
		default: score+=(action*action*BILLPOINTS);
	}
}

void Game::warp_to_level (int lev) {
	if (state==PLAYING) {
		if (lev <= level || lev > G_MAXINT/8+3 ) return;
		setup_level(lev);
	}
	else {
		if (lev<=0) return;
		start(lev);
	}
}

void Game::button_press(int x, int y) {
	int i, counter=0, flag=0;
	if (state != PLAYING) return;
	ui.set_cursor(DOWNC);
	if (bucket.clicked(x, y)) {
		ui.set_cursor(BUCKETC);
		grabbed = BUCKET;
	}
	for (i=0; i < bill.MAX_BILLS && !flag; i++) {
		if (bill.list[i].state == bill.list[i].OFF
			|| bill.list[i].state == bill.list[i].DYING)
				continue;
		if (bill.list[i].state == bill.list[i].STRAY &&
			bill.list[i].clickedstray(x, y))
		{
			ui.set_cursor (bill.list[i].cargo);
			grabbed = i;
			flag = 1;
		}
		else if (bill.list[i].state != bill.list[i].STRAY &&
			bill.list[i].clicked(x, y))
		{
			if (bill.list[i].state == bill.list[i].AT) 
				net.computers[bill.list[i].target_c].busy=0;
			bill.list[i].index = -1;
			bill.list[i].cels = bill.dcels;
			bill.list[i].x_offset = -2;
			bill.list[i].y_offset = -15;
			bill.list[i].state = bill.list[i].DYING;
			counter++;
		}
	}
	if (counter) update_score(counter);
}

void Game::button_release(int x, int y) {
	int i;
	ui.set_cursor (DEFAULTC);
	if (state != PLAYING || grabbed == EMPTY)
		return;
	if (grabbed == BUCKET) {
		grabbed = EMPTY;
		for (i=0; i<net.ncables; i++)
			if (net.cables[i].onspark(x, y)) {
				net.cables[i].active=0;
				net.cables[i].delay = spark.delay(level);
			}
		return;
	}
	for (i=0; i<net.units; i++)
		if (net.computers[i].oncomputer(x, y)
			&&
			net.computers[i].compatible (bill.list[grabbed].cargo)
			&&
			(net.computers[i].os == OS.WINGDOWS || 
			 net.computers[i].os == OS.OFF))
		{
			net.base++; 
			if (net.computers[i].os == OS.WINGDOWS)
				net.win--;
			else
				net.off--;
			net.computers[i].os = bill.list[grabbed].cargo;
			bill.list[grabbed].state = bill.list[grabbed].OFF;
			grabbed = EMPTY;
			return;
		}
	grabbed = EMPTY;
}	

void Game::update() {
        void show_scores(int);
        int pos;
	switch (state) {
	case PLAYING:
		ui.clear(); 
		bucket.draw();
		net.update();
		net.draw();
		bill.update();
		bill.draw();
		update_info();
		if (!(bill.on_screen+bill.off_screen)) {
			update_score(ENDLEVEL);
			state = BETWEEN;
		}
		if ((net.base+net.off)<=1) state = END;
		break;
	case END:
		ui.clear();
		net.toasters();
		net.draw();
		ui.refresh();
		ui.popup_dialog(ENDGAME);
		pos = gnome_score_log(score, NULL, TRUE);
		show_scores(pos);
		ui.clear();
		ui.draw_centered(logo);
		ui.kill_timer();
		ui.set_pausebutton (FALSE);
		state = WAITING;
		break;
	case BETWEEN:
		ui.update_scorebox(level, score);
		ui.popup_dialog (SCORE);
		state = PLAYING;
		/* This is a bad kludge, but i don't think anybody can pass
		   the 268435458 level (G_MAXINT/8+3 in a normal pcs). */
		if (++level < G_MAXINT/8+3)
			setup_level(level);
		else
			setup_level(1);
		break;
	}
	ui.refresh();
	iteration++;
}

static void parseAnArg(poptContext con, enum poptCallbackReason reason,
		       const struct poptOption *opt, const char *arg,
		       void *data) {
  if (reason != POPT_CALLBACK_REASON_OPTION) return;
  if (opt->val == -1)
    game.level = game.MAX(1, atoi(arg));
}
static const struct poptOption options[] = {
  { NULL, '\0', POPT_ARG_CALLBACK,
    (void *) &parseAnArg, 0, NULL },
  { "warp", 'l', POPT_ARG_STRING, NULL, -1,
    "Start at a different level", "LEVEL" },
  POPT_AUTOHELP
  { NULL, '\0', 0, NULL, 0 }
};

void Game::main(int argc, char **argv) {
	int c;
	extern char *optarg;

	level = 0;
	ui.initialize(argc, argv, options);
	srand(time(NULL));
	ui.make_mainwin();
	ui.graph_init();
	ui.clear();
	logo.load("logo");
	ui.draw_centered(logo);
	ui.refresh();
	ui.make_windows();
	//scores.read();
	//scores.update();

	bill.load_pix();
	OS.load_pix();
	net.load_pix();
	bucket.load_pix();
	spark.load_pix();

	ui.load_cursors();

	state = WAITING;
	if (level) start(level);
	else ui.set_pausebutton(FALSE);
	ui.MainLoop();
	exit(0);
}

int main(int argc, char **argv) {
        if (gnome_score_init("gnome-xbill")) {
	        printf("Couldn't set up score server\n");
		exit(1);
	}
	game.main(argc, argv);
}
