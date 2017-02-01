/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
#include <liboaf/liboaf.h>
#include <stdio.h>
#include <stdlib.h>
#include "empty.h"

int
main (int argc, char *argv[])
{
	OAF_ServerInfoList *result;
	CORBA_Environment ev;
	char *query;
	char **sort_criteria;
	int i;

	CORBA_exception_init (&ev);
	oaf_init (argc, argv);

	sort_criteria = NULL;

	if (argc > 1) {
		query = argv[1];
		if (argc > 2) {
			int i;
			int num_conditions;

			num_conditions = argc - 2;

			printf ("Number of sort criteria: %d\n",
				num_conditions);

			sort_criteria =
				g_malloc (sizeof (char *) *
					  (num_conditions + 1));

			for (i = 0; i < num_conditions; i++) {
				sort_criteria[i] = g_strdup (argv[i + 2]);
				puts (sort_criteria[i]);
			}

			sort_criteria[num_conditions] = NULL;
		}
	} else {
		query = "repo_ids.has('IDL:Empty:1.0')";
	}

	/* putenv("OAF_BARRIER_INIT=1"); */
	result = oaf_query (query, sort_criteria, &ev);

	/* result = oaf_query ("iid == 'OAFIID:Empty:19991025'", NULL, &ev); */

	if (result == NULL) {
		puts ("query failed");
	} else {
		printf ("number of results: %d\n", result->_length);

		for (i = 0; i < result->_length; i++) {
			puts ((result->_buffer[i]).iid);
		}

	}

	CORBA_exception_free (&ev);

	return 0;
}
