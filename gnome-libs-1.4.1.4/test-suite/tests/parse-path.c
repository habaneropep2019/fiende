#include <gnome.h>
#include <glib.h>
#include <stdio.h>

#include <config.h>

/* Do whatever you need to do... You should put the "expected" output of
   the program in ../expected/testname.out, which will be compared against
   the actual output of the test when it is run.

   A non-zero exit code also indicates failure.
*/

/* This is a #define in the real code, but this should work for us.  */
char *prefix;

/* Get the definition of parse_path and release_path from here.  */
#include "libgnome/parse-path.cP"


static void test_parse_path (const char *path, gint priv)
{
	ParsedPath *p = parse_path (path, priv); 

#if 0
	printf ("%s|%s|%d - %s|%s|%s|%s|%s|%s|\n", (char*) prefix, path, priv,
		p->file, p->section, p->key, p->def, p->path, p->opath);
#else
/* Make 'x' null safe. */
#define X(x) ((x)?(x):"(null)")

	printf ("parse_path (%d):\n\n", priv);
	printf ("   Prefix:\t%s\n", X(prefix));
	printf ("   Path:\t%s\n\n", X(path));
	printf ("   File:\t%s\n", X(p->file));
	printf ("   Section:\t%s\n", X(p->section));
	printf ("   Key:\t\t%s\n", X(p->key));
	printf ("   Def:\t\t%s\n", X(p->def));
	printf ("   Path:\t%s\n", X(p->path));
	printf ("   OPath:\t%s\n\n", X(p->opath));

#undef X

#endif
	fflush (stdout);

	release_path (p);
}

static void
my_parse_path (const char *path)
{
	test_parse_path (path, 0);
	test_parse_path (path, 1);
}

static void
run_tests (void)
{
	my_parse_path ("section/key");
	my_parse_path ("section/key=default");

	my_parse_path ("/section/key");
	my_parse_path ("/section/key=default");

	my_parse_path ("file/section/key");
	my_parse_path ("file/section/key=default");

	my_parse_path ("/file/section/key");
	my_parse_path ("/file/section/key=default");

	my_parse_path ("=/file=/section/key");
	my_parse_path ("=/file=/section/key=default");

	my_parse_path ("=/file/section/key");
	my_parse_path ("=/file/section/key=default");

}

static void
run_tests_with_prefix (const char *pfx)
{
        char *old_prefix = prefix;

	/*gnome_config_push_prefix (prefix);*/
        prefix = (char *)pfx;

	run_tests ();

	/*gnome_config_pop_prefix ();*/
	prefix = old_prefix;
}

int
main (int argc, char *argv[])
{
	putenv ("HOME=/tmp");
	gnomelib_init("parse-path", "0.0");

	gnome_user_dir = "/tmp/.gnome";
	gnome_user_private_dir = "/tmp/.gnome_private";

	run_tests ();

	run_tests_with_prefix ("prefix");
	run_tests_with_prefix ("prefix/");
	run_tests_with_prefix ("/prefix");
	run_tests_with_prefix ("/prefix/");

	run_tests_with_prefix ("prefix/a/b/c");
	run_tests_with_prefix ("prefix/a/b/c/");
	run_tests_with_prefix ("/prefix/a/b/c");
	run_tests_with_prefix ("/prefix/a/b/c/");

	run_tests_with_prefix ("=/prefix");
	run_tests_with_prefix ("=/prefix/");
	run_tests_with_prefix ("=/prefix/a/b/c");
	run_tests_with_prefix ("=/prefix/a/b/c/");

	return 0;
}
