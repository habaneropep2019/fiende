#include <gnome.h>
#include <glib.h>
#include <stdio.h>

/* Do whatever you need to do... You should put the "expected" output of
   the program in ../expected/testname.out, which will be compared against
   the actual output of the test when it is run.

   A non-zero exit code also indicates failure.
*/

/* This should probably be moved to glib/testglib.c.  */
int main(int argc, char *argv[])
{
	char **output;
	int i;
	gnomelib_init("split", "0.0");
	output = g_strsplit("foo:bar:baz:qaz", ":", -1);
	for(i = 0; output[i]; i++) {
		g_print("%s ", output[i]);
		g_free(output[i]);
	}
	g_print("\n"); g_free(output);
	output = g_strsplit("foo:bar:baz:qaz", ":", 2);
	for(i = 0; output[i]; i++) {
		g_print("%s ", output[i]);
		g_free(output[i]);
	}
	g_print("\n"); g_free(output);	
	return 0;
}
