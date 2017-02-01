#include <gnome.h>
#include <libgnome/gnome-fileconvert.h>
#include <glib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

/* Do whatever you need to do... You should put the "expected" output of
   the program in ../expected/testname.out, which will be compared against
   the actual output of the test when it is run.

   A non-zero exit code also indicates failure.
*/

int main(int argc, char *argv[])
{
	gint outfd, readlen = 0;
	gchar abuf[128];
	gchar *fn;
	
	gnomelib_init("fileconvert", "0.0");
	fn = g_strconcat(getenv("srcdir"), "/tests", NULL);
	chdir(fn);
	chmod("lynxdump.sh", S_IXGRP|S_IXOTH|S_IXUSR|S_IRGRP|S_IROTH|S_IRUSR|S_IWUSR);
	outfd = gnome_file_convert("fileconvert.in",
				   "text/html",
				   "text/ascii");
	if(outfd >= 0) return 1;
	outfd = gnome_file_convert("fileconvert.in",
				   "text/html",
				   "text/plain");
	if(outfd < 0) return 1;
	while((readlen = read(outfd, abuf, sizeof(abuf) - 1))) {
		printf("%.*s", readlen, abuf); fflush(stdout);
	} 

	printf("\n");
	fflush(stdout); /* Make sure to fflush things after you do
			   any output, so what you see on your terminal
			   is the same as what goes into the output file */
	return 0;
}
