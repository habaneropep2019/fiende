#include <gnome.h>
#include <glib.h>
#include <stdio.h>

int global_cntr = 0;

static void sample_trigger_function(char *msg, char *level, char *supinfo[])
{
	int i;
	for(i = 0; supinfo[i]; i++)
		g_print("%s ", supinfo[i]);
	g_print("[%s] %s\n", level, msg);
	fflush(stdout);
	
	global_cntr++;
}

int main(int argc, char *argv[])
{
	struct _GnomeTrigger trig;
	gchar *fn;

	trig.type = GTRIG_FUNCTION;
	trig.u.function = sample_trigger_function;
	trig.level = NULL;

	gnomelib_init("trigs", "0.0");
	fn = g_strconcat(getenv("srcdir"), "/tests/trigs.in", NULL);
	gnome_triggers_init();
	gnome_triggers_add_trigger(&trig, "test", "one", NULL);
	gnome_triggers_do("Test of direct hit", "warning", "test", "one", NULL);
	fflush(stdout);
	g_print("Global counter is now %d\n", global_cntr);
	fflush(stdout);
	gnome_triggers_do("Test of indirect hit", "warning", "test", "one", "two", "three", NULL);
	fflush(stdout);
	g_print("Global counter is now %d\n", global_cntr);
	fflush(stdout);
	return 0;
}
