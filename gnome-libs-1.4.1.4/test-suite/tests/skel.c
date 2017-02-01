#include <gnome.h>
#include <glib.h>
#include <stdio.h>

/* Do whatever you need to do... You should put the "expected" output of
   the program in ../expected/testname.out, which will be compared against
   the actual output of the test when it is run.

   A non-zero exit code also indicates failure.
*/

int main(int argc, char *argv[])
{
	gnomelib_init("skel", "0.0");
	fflush(stdout); /* Make sure to fflush things after you do
			   any output, so what you see on your terminal
			   is the same as what goes into the output file */
	return 0;
}
