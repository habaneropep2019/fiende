/*
 *	ScottFree Revision 1.15
 *
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 *
 *
 *	You must have an ANSI C compiler to build this program.
 */

#include <config.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <signal.h>

#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <gnome.h>
#include <zvt/zvtterm.h>

#include "Scott.h"

#include "north.xpm"
#include "east.xpm"
#include "south.xpm"
#include "west.xpm"
#include "up.xpm"
#include "down.xpm"
#include "inventory.xpm"

#define SCROLLBACK	100

ZvtTerm *Top;
ZvtTerm *Bottom;
GtkWidget *App;

Header GameHeader;
Tail GameTail;
Item *Items;
Room *Rooms;
char **Verbs;
char **Nouns;
char **Messages;
Action *Actions;
int LightRefill;
char NounText[16];
int Counters[16];	/* Range unknown */
int CurrentCounter;
int SavedRoom;
int RoomSaved[16];	/* Range unknown */
int DisplayUp;		/* Curses up */
int Redraw;		/* Update item window */
int Options;		/* Option flags set */
int Width;		/* Terminal width */
int TopHeight;		/* Height of top window */
int BottomHeight;	/* Height of bottom window */

static char Input_Buffer[256];
static int Position=0;

#define TRS80_LINE	"\n<------------------------------------------------------------>\n"

#define MyLoc	(GameHeader.PlayerRoom)

long BitFlags=0;	/* Might be >32 flags - I haven't seen >32 yet */

static int PerformActions(int vb,int no);

static void Fatal(char *type, char *x)
{
	GtkWidget *mb;
	if(DisplayUp)
	{
		mb=gnome_message_box_new(x, type, "OK", NULL, NULL);
		gnome_message_box_set_modal(GNOME_MESSAGE_BOX(mb));
		gnome_message_box_set_default(GNOME_MESSAGE_BOX(mb),0);
		gtk_widget_show(GTK_WIDGET(mb));
	}
	else
		fprintf(stderr,"%s.\n",x);
	exit(1);
}

static void Aborted()
{
	Fatal(GNOME_MESSAGE_BOX_GENERIC, "User exit");
}

static void ClearScreen(void)
{
#if 0
	werase(Bottom);
	wrefresh(Bottom);
#endif	
}

static void *MemAlloc(int size)
{
	void *t=(void *)malloc(size);
	if(t==NULL)
		Fatal(GNOME_MESSAGE_BOX_ERROR, "Out of memory");
	return(t);
}

static int RandomPercent(int n)
{
	unsigned int rv=rand()<<6;
	rv%=100;
	if(rv<n)
		return(1);
	return(0);
}

static int CountCarried(void)
{
	int ct=0;
	int n=0;
	while(ct<=GameHeader.NumItems)
	{
		if(Items[ct].Location==CARRIED)
			n++;
		ct++;
	}
	return(n);
}

static char *MapSynonym(const char *word)
{
	int n=1;
	char *tp;
	static char lastword[16];	/* Last non synonym */
	while(n<=GameHeader.NumWords)
	{
		tp=Nouns[n];
		if(*tp=='*')
			tp++;
		else
			strcpy(lastword,tp);
		if(g_strncasecmp(word,tp,GameHeader.WordLength)==0)
			return(lastword);
		n++;
	}
	return(NULL);
}

static int MatchUpItem(const char *text, int loc)
{
	const char *word=MapSynonym(text);
	int ct=0;
	
	if(word==NULL)
		word=text;
	
	while(ct<=GameHeader.NumItems)
	{
		if(Items[ct].AutoGet && Items[ct].Location==loc &&
			g_strncasecmp(Items[ct].AutoGet,word,GameHeader.WordLength)==0)
			return(ct);
		ct++;
	}
	return(-1);
}

static char *ReadString(FILE *f)
{
	char tmp[1024];
	char *t;
	int c,nc;
	int ct=0;
	do
	{
		c=fgetc(f);
	}
	while(c!=EOF && isspace(c));
	if(c!='"')
	{
		Fatal(GNOME_MESSAGE_BOX_WARNING, "Initial quote expected");
	}
	do
	{
		c=fgetc(f);
		if(c==EOF)
			Fatal(GNOME_MESSAGE_BOX_WARNING, "EOF in string");
		if(c=='"')
		{
			nc=fgetc(f);
			if(nc!='"')
			{
				ungetc(nc,f);
				break;
			}
		}
		if(c==0x60) 
			c='"'; /* pdd */
		tmp[ct++]=c;
	}
	while(1);
	tmp[ct]=0;
	t=MemAlloc(ct+1);
	memcpy(t,tmp,ct+1);
	return(t);
}
	
void LoadDatabase(FILE *f, int loud)
{
	int ni,na,nw,nr,mc,pr,tr,wl,lt,mn,trm;
	int ct;
	short lo;
	Action *ap;
	Room *rp;
	Item *ip;
/* Load the header */
	
	if(fscanf(f,"%*d %d %d %d %d %d %d %d %d %d %d %d",
		  &ni, &na,&nw,&nr,&mc,&pr,&tr,&wl,&lt,&mn,&trm /*,&ct */)<10)
		Fatal(GNOME_MESSAGE_BOX_ERROR, "Invalid database(bad header)");
	GameHeader.NumItems=ni;
	Items=(Item *)MemAlloc(sizeof(Item)*(ni+1));
	GameHeader.NumActions=na;
	Actions=(Action *)MemAlloc(sizeof(Action)*(na+1));
	GameHeader.NumWords=nw;
	GameHeader.WordLength=wl;
	Verbs=(char **)MemAlloc(sizeof(char *)*(nw+1));
	Nouns=(char **)MemAlloc(sizeof(char *)*(nw+1));
	GameHeader.NumRooms=nr;
	Rooms=(Room *)MemAlloc(sizeof(Room)*(nr+1));
	GameHeader.MaxCarry=mc;
	GameHeader.PlayerRoom=pr;
	GameHeader.Treasures=tr;
	GameHeader.LightTime=lt;
	LightRefill=lt;
	GameHeader.NumMessages=mn;
	Messages=(char **)MemAlloc(sizeof(char *)*(mn+1));
	GameHeader.TreasureRoom=trm;
	
/* Load the actions */

	ct=0;
	ap=Actions;
	if(loud)
		printf("Reading %d actions.\n",na);
	while(ct<na+1)
	{
		if(fscanf(f,"%hd %hd %hd %hd %hd %hd %hd %hd",
			&ap->Vocab,
			&ap->Condition[0],
			&ap->Condition[1],
			&ap->Condition[2],
			&ap->Condition[3],
			&ap->Condition[4],
			&ap->Action[0],
			&ap->Action[1])!=8)
		{
			printf("Bad action line (%d)\n",ct);
			exit(1);
		}
		ap++;
		ct++;
	}			
	ct=0;
	if(loud)
		printf("Reading %d word pairs.\n",nw);
	while(ct<nw+1)
	{
		Verbs[ct]=ReadString(f);
		Nouns[ct]=ReadString(f);
		ct++;
	}
	ct=0;
	rp=Rooms;
	if(loud)
		printf("Reading %d rooms.\n",nr);
	while(ct<nr+1)
	{
		fscanf(f,"%hd %hd %hd %hd %hd %hd",
			&rp->Exits[0],&rp->Exits[1],&rp->Exits[2],
			&rp->Exits[3],&rp->Exits[4],&rp->Exits[5]);
		rp->Text=ReadString(f);
		ct++;
		rp++;
	}
	ct=0;
	if(loud)
		printf("Reading %d messages.\n",mn);
	while(ct<mn+1)
	{
		Messages[ct]=ReadString(f);
		ct++;
	}
	ct=0;
	if(loud)
		printf("Reading %d items.\n",ni);
	ip=Items;
	while(ct<ni+1)
	{
		ip->Text=ReadString(f);
		ip->AutoGet=strchr(ip->Text,'/');
		/* Some games use // to mean no auto get/drop word! */
		if(ip->AutoGet && strcmp(ip->AutoGet,"//") && strcmp(ip->AutoGet,"/*"))
		{
			char *t;
			*ip->AutoGet++=0;
			t=strchr(ip->AutoGet,'/');
			if(t!=NULL)
				*t=0;
		}
		fscanf(f,"%hd",&lo);
		ip->Location=(unsigned char)lo;
		ip->InitialLoc=ip->Location;
		ip++;
		ct++;
	}
	ct=0;
	/* Discard Comment Strings */
	while(ct<na+1)
	{
		free(ReadString(f));
		ct++;
	}
	fscanf(f,"%d",&ct);
	if(loud)
		printf("Version %d.%02d of Adventure ",
		ct/100,ct%100);
	fscanf(f,"%d",&ct);
	if(loud)
		printf("%d.\nLoad Complete.\n\n",ct);
}

static int OutputPos=0;

static void OutReset(void)
{
#if 0
	OutputPos=0;
	wmove(Bottom,BottomHeight-1,0);
	wclrtoeol(Bottom);
#endif	
}

void
tty_put_in (ZvtTerm *term, char *text, int len)
{
	zvt_term_feed (term, text, len);
}

static void OutBuf(char *buffer)
{
	char word[80];
	int wp;
	while(*buffer)
	{
		if(OutputPos==0)
		{
			while(*buffer && isspace(*buffer))
			{
				if(*buffer=='\n')
				{
					tty_put_in(Bottom, "\r\n",2);
					OutputPos=0;
				}
				buffer++;
			}
		}
		if(*buffer==0)
			return;
		wp=0;
		while(*buffer && !isspace(*buffer))
		{
			word[wp++]=*buffer++;
		}
		word[wp]=0;
/*		fprintf(stderr,"Word '%s' at %d\n",word,OutputPos);*/
		if(OutputPos+strlen(word)>(Width-2))
		{
			tty_put_in(Bottom, "\r\n",2);
			OutputPos=0;
		}
		tty_put_in(Bottom,word, strlen(word));
		OutputPos+=strlen(word);
		
		if(*buffer==0)
			return;
		
		if(*buffer=='\n')
		{
			tty_put_in(Bottom, "\r\n",2);
			OutputPos=0;
		}
		else
		{
			OutputPos++;
			if(OutputPos<(Width-1))
				tty_put_in(Bottom," ",1);
		}
		buffer++;
	}
}

static void Output(char *a)
{
	char block[512];
	strcpy(block,a);
	OutBuf(block);
}

static void OutputNumber(int a)
{
	char buf[16];
	sprintf(buf,"%d ",a);
	OutBuf(buf);
}
		
static void Look(void)
{
	static char *ExitNames[6]=
	{
		"North","South","East","West","Up","Down"
	};
	Room *r;
	int ct,f;
	int pos;

	tty_put_in(Top, "\033[0;0H\033[J",9);
	
	if((BitFlags&(1<<DARKBIT)) && Items[LIGHT_SOURCE].Location!= CARRIED
	            && Items[LIGHT_SOURCE].Location!= MyLoc)
	{
		if(Options&YOUARE)
			tty_put_in(Top,"You can't see. It is too dark!\r\n",
				strlen("You can't see. It is too dark!\r\n"));
		else
			tty_put_in(Top,"I can't see. It is too dark!\r\n",
				strlen("I can't see. It is too dark!\r\n"));
		if (Options & TRS80_STYLE)
			tty_put_in(Top,TRS80_LINE, strlen(TRS80_LINE));
		return;
	}
	r=&Rooms[MyLoc];
	if(*r->Text=='*')
		tty_put_in(Top,r->Text+1, strlen(r->Text+1));
	else
	{
		if(Options&YOUARE)
			tty_put_in(Top,"You are ",8);
		else
			tty_put_in(Top,"I'm in a ",9);
		tty_put_in(Top, r->Text, strlen(r->Text));
	}
	ct=0;
	f=0;
	tty_put_in(Top,"\r\n\nObvious exits: ",18);
	while(ct<6)
	{
		if(r->Exits[ct]!=0)
		{
			if(f==0)
				f=1;
			else
				tty_put_in(Top,", ",2);
			tty_put_in(Top,ExitNames[ct], strlen(ExitNames[ct]));
		}
		ct++;
	}
	if(f==0)
		tty_put_in(Top,"none",4);
	tty_put_in(Top,".\r\n",3);
	ct=0;
	f=0;
	pos=0;

	while(ct<=GameHeader.NumItems)
	{
		if(Items[ct].Location==MyLoc)
		{
			if(f==0)
			{
				if(Options&YOUARE)
				{
					tty_put_in(Top,"\r\nYou can also see: ", 19);
					pos=18;
				}
				else
				{
					tty_put_in(Top,"\r\nI can also see: ", 17);
					pos=16;
				}
				f++;
			}
			else if (!(Options & TRS80_STYLE))
			{
				tty_put_in(Top," - ",3);
				pos+=3;
			}
			if(pos+strlen(Items[ct].Text)>(Width-10))
			{
				pos=0;
				tty_put_in(Top,"\r\n",2);
			}
			tty_put_in(Top,Items[ct].Text, strlen(Items[ct].Text));
			pos += strlen(Items[ct].Text);
			if (Options & TRS80_STYLE)
			{
				tty_put_in(Top,". ",2);
				pos+=2;
			}
		}
		ct++;
	}
	tty_put_in(Top,"\r\n",2);
	if (Options & TRS80_STYLE)
		tty_put_in(Top,TRS80_LINE, strlen(TRS80_LINE));
}

static int WhichWord(const char *word, char **list)
{
	int n=1;
	int ne=1;
	const char *tp;
	while(ne<=GameHeader.NumWords)
	{
		tp=list[ne];
		if(*tp=='*')
			tp++;
		else
			n=ne;
		if(g_strncasecmp(word,tp,GameHeader.WordLength)==0)
			return(n);
		ne++;
	}
	return(-1);
}

static GtkWidget *file_selector;

static void Input_Begin(int type)
{
	if(file_selector)
		return;
	if(type==1)
	{
		if(Redraw!=0)
		{
			Look();
			Redraw=0;
		}
		PerformActions(0,0);
		if(Redraw!=0)
		{
			Look();
			Redraw=0;
		}
	}
	Output("\nTell me what to do ? ");
}



static void file_picked(GtkWidget *w, gpointer data)
{
	int ct;
	FILE *f;
	gint save_this=GPOINTER_TO_INT (data);
	
	if(save_this)
	{
		f=fopen(gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selector)),"w");
		if(f==NULL)
		{
			Output("Unable to create save file.\n");
			return;
		}
		for(ct=0;ct<16;ct++)
		{
			fprintf(f,"%d %d\n",Counters[ct],RoomSaved[ct]);
		}
		fprintf(f,"%ld %d %hd %d %d %hd\n",BitFlags, (BitFlags&(1<<DARKBIT))?1:0,
			MyLoc,CurrentCounter,SavedRoom,GameHeader.LightTime);
		for(ct=0;ct<=GameHeader.NumItems;ct++)
			fprintf(f,"%hd\n",(short)Items[ct].Location);
		fclose(f);
		Output("Saved.\n");
	}
	gtk_widget_destroy(file_selector);
	file_selector=NULL;
	Input_Begin(0);
}

static void SaveGame(void)
{
	if(file_selector)
		return;
		
	file_selector=gtk_file_selection_new("Save as");


        gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->cancel_button),
		"clicked", (GtkSignalFunc) file_picked, (void *)0);
                        
	gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button),
		"clicked", (GtkSignalFunc) file_picked, (void *)1);
                                                
	gtk_widget_show(file_selector);
}


static void LoadGame(char *name)
{
	FILE *f=fopen(name,"r");
	int ct=0;
	short lo;
	short DarkFlag;
	if(f==NULL)
	{
		Output("Unable to restore game.");
		return;
	}
	for(ct=0;ct<16;ct++)
	{
		fscanf(f,"%d %d\n",&Counters[ct],&RoomSaved[ct]);
	}
	fscanf(f,"%ld %hd %hd %d %d %hd\n",
		&BitFlags,&DarkFlag,&MyLoc,&CurrentCounter,&SavedRoom,
		&GameHeader.LightTime);
	/* Backward compatibility */
	if(DarkFlag)
		BitFlags|=(1<<15);
	for(ct=0;ct<=GameHeader.NumItems;ct++)
	{
		fscanf(f,"%hd\n",&lo);
		Items[ct].Location=(unsigned char)lo;
	}
	fclose(f);
}

static int PerformLine(int ct)
{
	int continuation=0;
	int param[5],pptr=0;
	int act[4];
	int cc=0;
	while(cc<5)
	{
		int cv,dv;
		cv=Actions[ct].Condition[cc];
		dv=cv/20;
		cv%=20;
		switch(cv)
		{
			case 0:
				param[pptr++]=dv;
				break;
			case 1:
				if(Items[dv].Location!=CARRIED)
					return(0);
				break;
			case 2:
				if(Items[dv].Location!=MyLoc)
					return(0);
				break;
			case 3:
				if(Items[dv].Location!=CARRIED&&
					Items[dv].Location!=MyLoc)
					return(0);
				break;
			case 4:
				if(MyLoc!=dv)
					return(0);
				break;
			case 5:
				if(Items[dv].Location==MyLoc)
					return(0);
				break;
			case 6:
				if(Items[dv].Location==CARRIED)
					return(0);
				break;
			case 7:
				if(MyLoc==dv)
					return(0);
				break;
			case 8:
				if((BitFlags&(1<<dv))==0)
					return(0);
				break;
			case 9:
				if(BitFlags&(1<<dv))
					return(0);
				break;
			case 10:
				if(CountCarried()==0)
					return(0);
				break;
			case 11:
				if(CountCarried())
					return(0);
				break;
			case 12:
				if(Items[dv].Location==CARRIED||Items[dv].Location==MyLoc)
					return(0);
				break;
			case 13:
				if(Items[dv].Location==0)
					return(0);
				break;
			case 14:
				if(Items[dv].Location)
					return(0);
				break;
			case 15:
				if(CurrentCounter>dv)
					return(0);
				break;
			case 16:
				if(CurrentCounter<=dv)
					return(0);
				break;
			case 17:
				if(Items[dv].Location!=Items[dv].InitialLoc)
					return(0);
				break;
			case 18:
				if(Items[dv].Location==Items[dv].InitialLoc)
					return(0);
				break;
			case 19:/* Only seen in Brian Howarth games so far */
				if(CurrentCounter!=dv)
					return(0);
				break;
		}
		cc++;
	}
	/* Actions */
	act[0]=Actions[ct].Action[0];
	act[2]=Actions[ct].Action[1];
	act[1]=act[0]%150;
	act[3]=act[2]%150;
	act[0]/=150;
	act[2]/=150;
	cc=0;
	pptr=0;
	while(cc<4)
	{
		if(act[cc]>=1 && act[cc]<52)
		{
			Output(Messages[act[cc]]);
			Output("\n");
		}
		else if(act[cc]>101)
		{
			Output(Messages[act[cc]-50]);
			Output("\n");
		}
		else switch(act[cc])
		{
			case 0:/* NOP */
				break;
			case 52:
				if(CountCarried()==GameHeader.MaxCarry)
				{
					if(Options&YOUARE)
						Output("You are carrying too much. ");
					else
						Output("I've too much to carry! ");
					break;
				}
				if(Items[param[pptr]].Location==MyLoc)
					Redraw=1;
				Items[param[pptr++]].Location= CARRIED;
				break;
			case 53:
				Redraw=1;
				Items[param[pptr++]].Location=MyLoc;
				break;
			case 54:
				Redraw=1;
				MyLoc=param[pptr++];
				break;
			case 55:
				if(Items[param[pptr]].Location==MyLoc)
					Redraw=1;
				Items[param[pptr++]].Location=0;
				break;
			case 56:
				BitFlags|=1<<DARKBIT;
				break;
			case 57:
				BitFlags&=~(1<<DARKBIT);
				break;
			case 58:
				BitFlags|=(1<<param[pptr++]);
				break;
			case 59:
				if(Items[param[pptr]].Location==MyLoc)
					Redraw=1;
				Items[param[pptr++]].Location=0;
				break;
			case 60:
				BitFlags&=~(1<<param[pptr++]);
				break;
			case 61:
				if(Options&YOUARE)
					Output("You are dead.\n");
				else
					Output("I am dead.\n");
				BitFlags&=~(1<<DARKBIT);
				MyLoc=GameHeader.NumRooms;/* It seems to be what the code says! */
				Look();
				break;
			case 62:
			{
				/* Bug fix for some systems - before it could get parameters wrong */
				int i=param[pptr++];
				Items[i].Location=param[pptr++];
				Redraw=1;
				break;
			}
			case 63:
doneit:	
				gtk_main_quit();
				break;
			case 64:
				Look();
				break;
			case 65:
			{
				int ct=0;
				int n=0;
				while(ct<=GameHeader.NumItems)
				{
					if(Items[ct].Location==GameHeader.TreasureRoom &&
					  *Items[ct].Text=='*')
					  	n++;
					ct++;
				}
				if(Options&YOUARE)
					Output("You have stored ");
				else
					Output("I've stored ");
				OutputNumber(n);
				Output(" treasures.  On a scale of 0 to 100, that rates ");
				OutputNumber((n*100)/GameHeader.Treasures);
				Output(".\n");
				if(n==GameHeader.Treasures)
				{
					Output("Well done.\n");
					goto doneit;
				}
				break;
			}
			case 66:
			{
				int ct=0;
				int f=0;
				if(Options&YOUARE)
					Output("You are carrying:\n");
				else
					Output("I'm carrying:\n");
				while(ct<=GameHeader.NumItems)
				{
					if(Items[ct].Location==CARRIED)
					{
						if(f==1)
						{
							if (Options & TRS80_STYLE)
								Output(". ");
							else
								Output(" - ");
						}
						f=1;
						Output(Items[ct].Text);
					}
					ct++;
				}
				if(f==0)
					Output("Nothing");
				Output(".\n");
				break;
			}
			case 67:
				BitFlags|=(1<<0);
				break;
			case 68:
				BitFlags&=~(1<<0);
				break;
			case 69:
				GameHeader.LightTime=LightRefill;
				if(Items[LIGHT_SOURCE].Location==MyLoc)
					Redraw=1;
				Items[LIGHT_SOURCE].Location=CARRIED;
				BitFlags&=~(1<<LIGHTOUTBIT);
				break;
			case 70:
				ClearScreen(); /* pdd. */
				OutReset();
				break;
			case 71:
				SaveGame();
				break;
			case 72:
			{
				int i1=param[pptr++];
				int i2=param[pptr++];
				int t=Items[i1].Location;
				if(t==MyLoc || Items[i2].Location==MyLoc)
					Redraw=1;
				Items[i1].Location=Items[i2].Location;
				Items[i2].Location=t;
				break;
			}
			case 73:
				continuation=1;
				break;
			case 74:
				if(Items[param[pptr]].Location==MyLoc)
					Redraw=1;
				Items[param[pptr++]].Location= CARRIED;
				break;
			case 75:
			{
				int i1,i2;
				i1=param[pptr++];
				i2=param[pptr++];
				if(Items[i1].Location==MyLoc)
					Redraw=1;
				Items[i1].Location=Items[i2].Location;
				if(Items[i2].Location==MyLoc)
					Redraw=1;
				break;
			}
			case 76:	/* Looking at adventure .. */
				Look();
				break;
			case 77:
				if(CurrentCounter>=0)
					CurrentCounter--;
				break;
			case 78:
				OutputNumber(CurrentCounter);
				break;
			case 79:
				CurrentCounter=param[pptr++];
				break;
			case 80:
			{
				int t=MyLoc;
				MyLoc=SavedRoom;
				SavedRoom=t;
				Redraw=1;
				break;
			}
			case 81:
			{
				/* This is somewhat guessed. Claymorgue always
				   seems to do select counter n, thing, select counter n,
				   but uses one value that always seems to exist. Trying
				   a few options I found this gave sane results on ageing */
				int t=param[pptr++];
				int c1=CurrentCounter;
				CurrentCounter=Counters[t];
				Counters[t]=c1;
				break;
			}
			case 82:
				CurrentCounter+=param[pptr++];
				break;
			case 83:
				CurrentCounter-=param[pptr++];
				if(CurrentCounter< -1)
					CurrentCounter= -1;
				/* Note: This seems to be needed. I don't yet
				   know if there is a maximum value to limit too */
				break;
			case 84:
				Output(NounText);
				break;
			case 85:
				Output(NounText);
				Output("\n");
				break;
			case 86:
				Output("\n");
				break;
			case 87:
			{
				/* Changed this to swap location<->roomflag[x]
				   not roomflag 0 and x */
				int p=param[pptr++];
				int sr=MyLoc;
				MyLoc=RoomSaved[p];
				RoomSaved[p]=sr;
				Redraw=1;
				break;
			}
			case 88:
/*				wrefresh(Top);
				wrefresh(Bottom);*/
				sleep(2);	/* DOC's say 2 seconds. Spectrum times at 1.5 */
				break;
			case 89:
				pptr++;
				/* SAGA draw picture n */
				/* Spectrum Seas of Blood - start combat ? */
				/* Poking this into older spectrum games causes a crash */
				break;
			default:
				fprintf(stderr,"Unknown action %d [Param begins %d %d]\n",
					act[cc],param[pptr],param[pptr+1]);
				break;
		}
		cc++;
	}
	return(1+continuation);		
}


static int PerformActions(int vb,int no)
{
	static int disable_sysfunc=0;	/* Recursion lock */
	int d=BitFlags&(1<<DARKBIT);
	
	int ct=0;
	int fl;
	int doagain=0;
	if(vb==1 && no == -1 )
	{
		Output("Give me a direction too.");
		return(0);
	}
	if(vb==1 && no>=1 && no<=6)
	{
		int nl;
		if(Items[LIGHT_SOURCE].Location==MyLoc ||
		   Items[LIGHT_SOURCE].Location==CARRIED)
		   	d=0;
		if(d)
			Output("Dangerous to move in the dark! ");
		nl=Rooms[MyLoc].Exits[no-1];
		if(nl!=0)
		{
			MyLoc=nl;
			Look();
			return(0);
		}
		if(d)
		{
			if(Options&YOUARE)
				Output("You fell down and broke your neck. ");
			else
				Output("I fell down and broke my neck. ");
			MyLoc=GameHeader.NumRooms;/* It seems to be what the code says! */
			BitFlags&=~(1<<15);
			return(0);
		}
		if(Options&YOUARE)
			Output("You can't go in that direction. ");
		else
			Output("I can't go in that direction. ");
		return(0);
	}
	fl= -1;
	while(ct<=GameHeader.NumActions)
	{
		int vv,nv;
		vv=Actions[ct].Vocab;
		/* Think this is now right. If a line we run has an action73
		   run all following lines with vocab of 0,0 */
		if(vb!=0 && (doagain&&vv!=0))
			break;
		/* Oops.. added this minor cockup fix 1.11 */
		if(vb!=0 && !doagain && fl== 0)
			break;
		nv=vv%150;
		vv/=150;
		if((vv==vb)||(doagain&&Actions[ct].Vocab==0))
		{
			if((vv==0 && RandomPercent(nv))||doagain||
				(vv!=0 && (nv==no||nv==0)))
			{
				int f2;
				if(fl== -1)
					fl= -2;
				if((f2=PerformLine(ct))>0)
				{
					/* ahah finally figured it out ! */
					fl=0;
					if(f2==2)
						doagain=1;
					if(vb!=0 && doagain==0)
						return 0;
				}
			}
		}
		ct++;
		if(Actions[ct].Vocab!=0)
			doagain=0;
	}
	if(fl!=0 && disable_sysfunc==0)
	{
		int i;
		if(Items[LIGHT_SOURCE].Location==MyLoc ||
		   Items[LIGHT_SOURCE].Location==CARRIED)
		   	d=0;
		if(vb==10 || vb==18)
		{
			/* Yes they really _are_ hardcoded values */
			if(vb==10)
			{
				if(g_strcasecmp(NounText,"ALL")==0)
				{
					int ct=0;
					int f=0;
					
					if(d)
					{
						Output("It is dark.\n");
						return 0;
					}
					while(ct<=GameHeader.NumItems)
					{
						if(Items[ct].Location==MyLoc && Items[ct].AutoGet!=NULL && Items[ct].AutoGet[0]!='*')
						{
							no=WhichWord(Items[ct].AutoGet,Nouns);
							disable_sysfunc=1;	/* Don't recurse into auto get ! */
							PerformActions(vb,no);	/* Recursively check each items table code */
							disable_sysfunc=0;
							if(CountCarried()==GameHeader.MaxCarry)
							{
								if(Options&YOUARE)
									Output("You are carrying too much. ");
								else
									Output("I've too much to carry. ");
								return(0);
							}
						 	Items[ct].Location= CARRIED;
						 	Redraw=1;
						 	OutBuf(Items[ct].Text);
						 	Output(": O.K.\n");
						 	f=1;
						 }
						 ct++;
					}
					if(f==0)
						Output("Nothing taken.");
					return(0);
				}
				if(no==-1)
				{
					Output("What ? ");
					return(0);
				}
				if(CountCarried()==GameHeader.MaxCarry)
				{
					if(Options&YOUARE)
						Output("You are carrying too much. ");
					else
						Output("I've too much to carry. ");
					return(0);
				}
				i=MatchUpItem(NounText,MyLoc);
				if(i==-1)
				{
					if(Options&YOUARE)
						Output("It is beyond your power to do that. ");
					else
						Output("It's beyond my power to do that. ");
					return(0);
				}
				Items[i].Location= CARRIED;
				Output("O.K. ");
				Redraw=1;
				return(0);
			}
			if(vb==18)
			{
				if(g_strcasecmp(NounText,"ALL")==0)
				{
					int ct=0;
					int f=0;
					while(ct<=GameHeader.NumItems)
					{
						if(Items[ct].Location==CARRIED && Items[ct].AutoGet && Items[ct].AutoGet[0]!='*')
						{
							no=WhichWord(Items[ct].AutoGet,Nouns);
							disable_sysfunc=1;
							PerformActions(vb,no);
							disable_sysfunc=0;
							Items[ct].Location=MyLoc;
							OutBuf(Items[ct].Text);
							Output(": O.K.\n");
							Redraw=1;
							f=1;
						}
						ct++;
					}
					if(f==0)
						Output(N_("Nothing dropped.\n"));
					return(0);
				}
				if(no==-1)
				{
					Output(N_("What ? "));
					return(0);
				}
				i=MatchUpItem(NounText,CARRIED);
				if(i==-1)
				{
					if(Options&YOUARE)
						Output(N_("It's beyond your power to do that.\n"));
					else
						Output(N_("It's beyond my power to do that.\n"));
					return(0);
				}
				Items[i].Location=MyLoc;
				Output(N_("OK"));
				Redraw=1;
				return(0);
			}
		}
	}
	return(fl);
}

static void Input_Complete(void)
{
	char verb[10],noun[10];
	int vc,nc;
	int num;
	
	OutReset();
	num=sscanf(Input_Buffer,"%9s %9s",verb,noun);

	if(num<1)
	{
		Input_Begin(0);
		return;
	}
	
	if(num==1)
		*noun=0;
	if(*noun==0 && strlen(verb)==1)
	{
		switch(isupper(*verb)?tolower(*verb):*verb)
		{
			case 'n':strcpy(verb,"NORTH");break;
			case 'e':strcpy(verb,"EAST");break;
			case 's':strcpy(verb,"SOUTH");break;
			case 'w':strcpy(verb,"WEST");break;
			case 'u':strcpy(verb,"UP");break;
			case 'd':strcpy(verb,"DOWN");break;
			/* Brian Howarth interpreter also supports this */
			case 'i':strcpy(verb,"INVENTORY");break;
		}
	}
	nc=WhichWord(verb,Nouns);
	/* The Scott Adams system has a hack to avoid typing 'go' */
	if(nc>=1 && nc <=6)
	{
		vc=1;
	}
	else
	{
		vc=WhichWord(verb,Verbs);
		nc=WhichWord(noun,Nouns);
	}
	if(vc==-1)
	{
		Output(N_("You use word(s) I don't know! "));
		Input_Begin(0);
		return;
	}
	strcpy(NounText,noun);	/* Needed by GET/DROP hack */
	switch(PerformActions(vc,nc))
	{
		case -1:Output(N_("I don't understand your command. "));
			break;
		case -2:Output(N_("I can't do that yet. "));
			break;
	}
	/* Brian Howarth games seem to use -1 for forever */
	if(Items[LIGHT_SOURCE].Location/*==-1*/!=DESTROYED && GameHeader.LightTime!= -1)
	{
		GameHeader.LightTime--;
		if(GameHeader.LightTime<1)
		{
			BitFlags|=(1<<LIGHTOUTBIT);
			if(Items[LIGHT_SOURCE].Location==CARRIED ||
				Items[LIGHT_SOURCE].Location==MyLoc)
			{
				if(Options&SCOTTLIGHT)
					Output(N_("Light has run out! "));
				else
					Output(N_("Your light has run out. "));
			}
			if(Options&PREHISTORIC_LAMP)
				Items[LIGHT_SOURCE].Location=DESTROYED;
		}
		else if(GameHeader.LightTime<25)
		{
			if(Items[LIGHT_SOURCE].Location==CARRIED ||
			Items[LIGHT_SOURCE].Location==MyLoc)
			{
	
				if(Options&SCOTTLIGHT)
				{
					Output(N_("Light runs out in "));
					OutputNumber(GameHeader.LightTime);
					Output(N_(" turns. "));
				}
				else
				{
					if(GameHeader.LightTime%5==0)
						Output(N_("Your light is growing dim. "));
				}
			}
		}
	}
	Input_Begin(1);
}

static void Key_Press(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	int length = event->length;
	int keyval     = event->keyval;

	if(length==0 || file_selector)
		return;
		
	switch(keyval)
	{
		case GDK_Return:
			Input_Buffer[Position]=0;
			tty_put_in(Bottom, "\r\n", 2);
			Input_Complete();
			Position=0;
			return;
		case GDK_Delete:
		case GDK_BackSpace:
			if(Position>0)
			{
				tty_put_in(Bottom, "\b \b",3);
				Position--;
			}
			break;
		default:
			if(keyval>=' '&&keyval<=126)
			{
				if(Position<256)
				{
					char c=keyval;
					Input_Buffer[Position++]=c;
					tty_put_in(Bottom, &c, 1);
				}
			}
			break;
	}
}

static void Dont_Type(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	return;
}

static void Scroll_Bottom(GtkAdjustment *adj, gpointer *tty)
{
#if 0
	gtk_term_set_scroll_offset(ZVT_TERM(tty),
        	(gint)adj->value-SCROLLBACK);
#endif
}


/*
 *	Command input helper
 */	
 
void TypeString(char *s)
{
	tty_put_in(Bottom, "\n\r", 2);
	strcpy(Input_Buffer,s);
	Position=strlen(s);
	Input_Complete();
	Position=0;
}
	

/*
 *	Toolbar items
 */
 
static void TypeNorth(GtkWidget *w, gpointer *spare)
{
	TypeString("north");
}

static void TypeEast(GtkWidget *w, gpointer *spare)
{
	TypeString("east");
}

static void TypeSouth(GtkWidget *w, gpointer *spare)
{
	TypeString("south");
}

static void TypeWest(GtkWidget *w, gpointer *spare)
{
	TypeString("west");
}

static void TypeUp(GtkWidget *w, gpointer *spare)
{
	TypeString("up");
}

static void TypeDown(GtkWidget *w, gpointer *spare)
{
	TypeString("down");
}
 
static void TypeInventory(GtkWidget *w, gpointer *spare)
{
	TypeString("inventory");
}

static void TypeSave(GtkWidget *w, gpointer *spare)
{
	TypeString("save");
}

static void LoadThis(GtkWidget *w, gpointer data)
{
	gint load_this=GPOINTER_TO_INT (data);
	
	if(load_this)
	{
		LoadGame(gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selector)));
	}
	gtk_widget_destroy(file_selector);
	file_selector=NULL;
	Input_Begin(0);
}

static void LoadAGame(GtkWidget *w, gpointer *spare)
{
	if(file_selector)
		return;
		
	tty_put_in(Bottom, "\n\r", 2);
	Position=0;
	
	file_selector=gtk_file_selection_new("Load ...");


        gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->cancel_button),
		"clicked", (GtkSignalFunc) LoadThis, (void *)0);
                        
	gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button),
		"clicked", (GtkSignalFunc) LoadThis, (void *)1);
                                                
	gtk_widget_show(file_selector);
}

static void NewGame(GtkWidget *w, gpointer *spare)
{
	int i;
	LightRefill=GameHeader.LightTime;
	BitFlags=0;
	memset(Counters, '\0', sizeof(Counters));
	for(i=0;i<=GameHeader.NumItems;i++)
		Items[i].Location=Items[i].InitialLoc;

	tty_put_in(Bottom, "\033[0;0H\033[J", 9);
	Position=0;
	
	Look();
	Input_Begin(1);
}

static void AboutScottFree(GtkWidget *w, gpointer *spare)
{
	GtkWidget *about;
	const gchar *authors[] = { "Alan Cox", NULL };

	about = gnome_about_new("ScottFree", "1.15",
				_("Copyright (C) 1997 Alan Cox"),
				(const char **)authors, NULL, NULL);
	gtk_widget_show(about);
}

static GnomeUIInfo toolbar[]=
{
        GNOMEUIINFO_ITEM(N_("North"), NULL, TypeNorth, north_xpm),
        GNOMEUIINFO_ITEM(N_("East"), NULL, TypeEast, east_xpm),
        GNOMEUIINFO_ITEM(N_("South"), NULL, TypeSouth, south_xpm),
        GNOMEUIINFO_ITEM(N_("West"), NULL, TypeWest, west_xpm),
        GNOMEUIINFO_ITEM(N_("Up"), NULL, TypeUp, up_xpm),
        GNOMEUIINFO_ITEM(N_("Down"), NULL, TypeDown, down_xpm),
        GNOMEUIINFO_ITEM(N_("List"), N_("What am I carrying"), TypeInventory, inventory_xpm),
	GNOMEUIINFO_END
};

static GnomeUIInfo file_menu[]={
        GNOMEUIINFO_MENU_NEW_ITEM( N_("_New"), NULL, NewGame, NULL),
	GNOMEUIINFO_MENU_OPEN_ITEM(LoadAGame, NULL),
	GNOMEUIINFO_MENU_SAVE_AS_ITEM(TypeSave, NULL),
        GNOMEUIINFO_MENU_EXIT_ITEM(gtk_main_quit, NULL),
	GNOMEUIINFO_END
};

static GnomeUIInfo help_menu[]={
	GNOMEUIINFO_HELP("scottfree"),
	GNOMEUIINFO_MENU_ABOUT_ITEM(AboutScottFree, NULL),
	GNOMEUIINFO_END
};

static GnomeUIInfo menu[]={
        GNOMEUIINFO_MENU_FILE_TREE(file_menu),
	GNOMEUIINFO_MENU_HELP_TREE(help_menu),
	GNOMEUIINFO_END
};

int main(int argc, char *argv[])
{
	FILE *f;
	GtkWidget *divider;
	GdkFont *font;
	GtkWidget *hbox;
	GtkObject *adj;
	GtkWidget *scrollbar;
		
	gnome_score_init ("GnomeScott");

	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

	gnome_init("GnomeScott", VERSION, argc, argv);
	
	while(argv[1])
	{
		if(*argv[1]!='-')
			break;
		switch(argv[1][1])
		{
			case 'y':
				Options|=YOUARE;
				break;
			case 'i':
				Options&=~YOUARE;
				break;
			case 'd':
				Options|=DEBUGGING;
				break;
			case 's':
				Options|=SCOTTLIGHT;
				break;
			case 't':
				Options|=TRS80_STYLE;
				break;
			case 'p':
				Options|=PREHISTORIC_LAMP;
				break;
			case 'h':
			default:
				fprintf(stderr,"%s: [-h] [-y] [-s] [-i] [-t] [-d] [-p] <gamename> [savedgame].\n",
						argv[0]);
				exit(1);
		}
		if(argv[1][2]!=0)
		{
			fprintf(stderr,"%s: option -%c does not take a parameter.\n",
				argv[0],argv[1][1]);
			exit(1);
		}
		argv++;
		argc--;
	}			

	if(argc!=2 && argc!=3)
	{
		fprintf(stderr,"%s <database> <savefile>.\n",argv[0]);
		exit(1);
	}
	f=fopen(argv[1],"r");
	if(f==NULL)
	{
		perror(argv[1]);
		exit(1);
	}
	signal(SIGINT,Aborted);		/* For BSD curses */
	signal(SIGQUIT,SIG_IGN);
	signal(SIGTSTP,SIG_IGN);
	
	if (Options & TRS80_STYLE)
	{
		Width = 64;
		TopHeight = 11;
		BottomHeight = 13;
	}
	else
	{
		Width = 80;
		TopHeight = 10;
		BottomHeight = 14;
	}
	
	font=gdk_font_load("8x8");

	App=gnome_app_new(argv[0],"ScottFree 1.15");
	
	gtk_signal_connect(GTK_OBJECT(App), "destroy",
			   GTK_SIGNAL_FUNC(gtk_main_quit), NULL);
			   
	divider=gtk_vbox_new(FALSE, 1);
	
	gnome_app_set_contents(GNOME_APP(App), divider);
	
	gnome_app_create_toolbar(GNOME_APP(App), toolbar);
	gnome_app_create_menus(GNOME_APP(App), menu);
	gtk_menu_item_right_justify(GTK_MENU_ITEM(menu[1].widget));
	
	Top = (ZvtTerm *)zvt_term_new();
	Bottom=(ZvtTerm *)zvt_term_new();

	hbox = gtk_hbox_new(FALSE, 1);
	
	adj = gtk_adjustment_new(SCROLLBACK, 0, SCROLLBACK, 1, 1, 1);
	gtk_signal_connect(GTK_OBJECT(adj), "value_changed",
			   GTK_SIGNAL_FUNC(Scroll_Bottom), Bottom);
	scrollbar = gtk_vscrollbar_new(GTK_ADJUSTMENT(adj));

	gtk_signal_connect(GTK_OBJECT(Bottom), "key_press_event",
		(GtkSignalFunc)Key_Press, NULL);
		
	gtk_signal_connect(GTK_OBJECT(Top), "key_press_event",
		(GtkSignalFunc)Dont_Type, NULL);
		
	gtk_widget_grab_focus(GTK_WIDGET(Bottom));

	gtk_widget_show(GTK_WIDGET(scrollbar));	
	gtk_widget_show(GTK_WIDGET(Top));
	gtk_widget_show(GTK_WIDGET(Bottom));
	
	gtk_box_pack_start(GTK_BOX(divider), GTK_WIDGET(Top), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(Bottom), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(scrollbar), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(divider), GTK_WIDGET(hbox), FALSE, FALSE, 0);
	
	gtk_widget_show(hbox);
	gtk_widget_show(divider);
	
	gtk_widget_show(App);
	
	OutReset();
	OutBuf("\
Scott Free, A Scott Adams game driver in C.\n\
Release 1.15, (c) 1993,1994,1995,1997 Swansea University Computer Society.\n\
Distributed under the GNU software license\n\n");
	LoadDatabase(f,(Options&DEBUGGING)?1:0);
	if(argc==3)
		LoadGame(argv[2]);
	srand(time(NULL)^getpid()^getuid());
	Look();
	Input_Begin(1);	
	gtk_main();
	return 0;
}
	
