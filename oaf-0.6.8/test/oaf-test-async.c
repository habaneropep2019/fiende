/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
#include <liboaf/liboaf.h>
#include <liboaf/oaf-async.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define DEBUG_TIME 0

typedef struct {
        gboolean callback_called;
        gboolean succeeded;
} callback_data;


static 
void test_callback (CORBA_Object   activated_object, 
                    const char    *error_reason, 
                    gpointer       user_data)
{
        callback_data *data;

        data = (callback_data *) user_data;

        if (activated_object == CORBA_OBJECT_NIL) {
                data->succeeded = FALSE;
        } else {
                data->succeeded = TRUE;
        }
                
        data->callback_called = TRUE;
}


/* returns TRUE in case of success. FALSE otherwise. 
   -1 if answer timeouted.... */
static int
test_activate (char *requirements)
{
        CORBA_Environment ev;
        callback_data data;
#if DEBUG_TIME        
        time_t beg_time;
#endif

        CORBA_exception_init (&ev);

        data.callback_called = FALSE;
        oaf_activate_async (requirements, NULL, 0, test_callback, &data, &ev);

#if DEBUG_TIME
        beg_time = time (NULL);
#endif

        while (data.callback_called == FALSE) {
                g_main_iteration (TRUE);
#if DEBUG_TIME
                if (time (NULL) > (beg_time + 10)) {
                        return -1;
                }
#endif
        }

        
        if (data.succeeded == TRUE) {
                return TRUE;
        } else {
                return FALSE;
        }
}

/* returns TRUE in case of success. FALSE otherwise. 
   -1 if answer timeouted.... */
static int
test_activate_from_id (char *aid)
{
        CORBA_Environment ev;
        callback_data data;
#if DEBUG_TIME        
        time_t beg_time;
#endif

        CORBA_exception_init (&ev);

        data.callback_called = FALSE;
        oaf_activate_from_id_async (aid, 0, test_callback, &data, &ev);

#if DEBUG_TIME
        beg_time = time (NULL);
#endif

        while (data.callback_called == FALSE) {
                g_main_iteration (TRUE);
#if DEBUG_TIME
                if (time (NULL) > (beg_time + 10)) {
                        return -1;
                }
#endif
        }

        
        if (data.succeeded == TRUE) {
                return TRUE;
        } else {
                return FALSE;
        }
}


#define TOTAL_TESTS 4
int
main (int argc, char *argv[])
{
        int test_status;
        int test_passed;

        test_passed = 0;

	oaf_init (argc, argv);
        printf ("testing async interfaces\n");

        printf ("testing activate_async... ");
        /* this should fail */
        test_status = test_activate ("");
        if (test_status == FALSE) {
                test_passed++;
                printf (" passed\n");
        } else if (test_status == TRUE
                   || test_status == -1) {
                printf (" failed\n");
        }

        printf ("testing activate_async... ");
        test_status = test_activate ("has (repo_ids, 'IDL:Empty:1.0')");
        if (test_status == TRUE) {
                test_passed++;
                printf (" passed\n");
        } else if (test_status == FALSE
                   || test_status == -1) {
                printf (" failed\n");
        }

        printf ("testing activate_from_id_async... ");
        test_status = test_activate_from_id ("");
        if (test_status == FALSE) {
                test_passed++;
                printf (" passed\n");
        } else if (test_status == TRUE
                   || test_status == -1) {
                printf (" failed\n");
        }

        printf ("testing activate_from_id_async... ");
        test_status = test_activate_from_id ("OAFIID:Empty:19991025");
        if (test_status == TRUE) {
                test_passed++;
                printf (" passed\n");
        } else if (test_status == FALSE
                   || test_status == -1) {
                printf (" failed\n");
        }

        printf ("Async Test Results: %d passed upon %d \n", 
                test_passed, TOTAL_TESTS);

        if (test_passed != TOTAL_TESTS) {
                return 1;
        }

        return 0;
}



