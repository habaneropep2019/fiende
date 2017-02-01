/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* test-async-cancel.c - Test program for the GNOME Virtual File System.

   Copyright (C) 1999 Free Software Foundation

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Author: Darin Adler <darin@eazel.com>
*/

#include <config.h>

#include <glib.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <libgnorba/gnorba.h>
#include <unistd.h>

#include "gnome-vfs.h"
#include "gnome-vfs-backend.h"

#define TEST_ASSERT(expression, message) \
	G_STMT_START { if (!(expression)) test_failed message; } G_STMT_END

static GnomeVFSAsyncHandle *test_handle;
static gpointer test_callback_data;
static gboolean test_done;

#define MAX_THREAD_WAIT 100

static void
stop_after_log (const char *domain, GLogLevelFlags level, 
	const char *message, gpointer data)
{
	void (* saved_handler)(int);
	
	g_log_default_handler(domain, level, message, data);

	saved_handler = signal (SIGINT, SIG_IGN);
	raise(SIGINT);
	signal(SIGINT, saved_handler);
}

static void
make_asserts_break (const char *domain)
{
	g_log_set_handler (domain, 
		(GLogLevelFlags) (G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_WARNING),
		stop_after_log, NULL);
}

static int
get_free_file_descriptor_count (void)
{
	int count;
	GList *list, *p;
	int fd;

	list = NULL;
	for (count = 0; ; count++) {
		fd = open ("/dev/null", O_RDONLY);
		if (fd == -1) {
			break;
		}
		list = g_list_prepend (list, GINT_TO_POINTER (fd));
	}

	for (p = list; p != NULL; p = p->next) {
		close (GPOINTER_TO_INT (p->data));
	}
	g_list_free (list);

	return count;
}

static int free_at_start;

static int
get_used_file_descriptor_count (void)
{
	return free_at_start - get_free_file_descriptor_count ();
}

static gboolean
wait_for_boolean (gboolean *wait_for_it)
{
	int i;

	if (*wait_for_it) {
		return TRUE;
	}

	for (i = 0; i < MAX_THREAD_WAIT; i++) {
		usleep (1);
		gtk_main_iteration_do (FALSE);
		if (*wait_for_it) {
			return TRUE;
		}
	}
	return FALSE;
}

static gboolean
wait_until_vfs_threads_gone (void)
{
	int i;

	if (gnome_vfs_backend_get_job_count () == 0) {
		return TRUE;
	}

	for (i = 0; i < MAX_THREAD_WAIT; i++) {
		usleep (1);
		gtk_main_iteration_do (FALSE);
		if (gnome_vfs_backend_get_job_count () == 0) {
			return TRUE;
		}
	}
	return FALSE;
}

static gboolean
wait_until_vfs_threads_gone_no_main (void)
{
	int i;

	if (gnome_vfs_backend_get_job_count () == 0) {
		return TRUE;
	}

	for (i = 0; i < MAX_THREAD_WAIT; i++) {
		usleep (1);
		if (gnome_vfs_backend_get_job_count () == 0) {
			return TRUE;
		}
	}
	return FALSE;
}

static gboolean
wait_until_file_descriptors_gone (void)
{
	int i;

	if (get_used_file_descriptor_count () == 0) {
		return TRUE;
	}

	for (i = 0; i < MAX_THREAD_WAIT; i++) {
		usleep (1);
		gtk_main_iteration_do (FALSE);
		if (get_used_file_descriptor_count () == 0) {
			return TRUE;
		}
	}
	return FALSE;
}

static gboolean at_least_one_test_failed = FALSE;

static void
test_failed (const char *format, ...)
{
	va_list arguments;
	char *message;

	va_start (arguments, format);
	message = g_strdup_vprintf (format, arguments);
	va_end (arguments);

	g_message ("test failed: %s", message);
	at_least_one_test_failed = TRUE;
}

static void
get_file_info_callback (GnomeVFSAsyncHandle *handle,
			GList *results,
			gpointer callback_data)
{
	TEST_ASSERT (handle == test_handle, ("get_file_info, bad handle"));
	TEST_ASSERT (g_list_length (results) == 1, ("get_file_info, bad list length"));
	TEST_ASSERT (callback_data == test_callback_data, ("get_file_info, bad handle"));

	test_handle = NULL;
	g_free (callback_data);

	test_done = TRUE;
}

static void
first_get_file_info (void)
{
	GList *uri_list;

	/* Start a get_file_info call. */
	test_done = FALSE;
	test_callback_data = g_malloc (1);
	uri_list = g_list_prepend (NULL, gnome_vfs_uri_new ("file:///dev/null"));
	gnome_vfs_async_get_file_info (&test_handle,
				       uri_list,
				       GNOME_VFS_FILE_INFO_DEFAULT,
				       get_file_info_callback,
				       test_callback_data);
	g_list_free (uri_list);

	/* Wait until it is done. */
	TEST_ASSERT (wait_for_boolean (&test_done), ("first_get_file_info: callback was not called"));
	TEST_ASSERT (wait_until_vfs_threads_gone (), ("first_get_file_info: thread never went away"));

	/* For some reason, this consumes file descriptors.
	 * I don't know why.
	 */
}

static void
test_get_file_info (void)
{
	GList *uri_list;

	/* Start a get_file_info call. */
	test_done = FALSE;
	test_callback_data = g_malloc (1);
	uri_list = g_list_prepend (NULL, gnome_vfs_uri_new ("file:///dev/null"));
	gnome_vfs_async_get_file_info (&test_handle,
				       uri_list,
				       GNOME_VFS_FILE_INFO_DEFAULT,
				       get_file_info_callback,
				       test_callback_data);
	g_list_free (uri_list);

	/* Wait until it is done. */
	TEST_ASSERT (wait_for_boolean (&test_done), ("get_file_info 1: callback was not called"));
	TEST_ASSERT (wait_until_vfs_threads_gone (), ("get_file_info 1: thread never went away"));
	TEST_ASSERT (get_used_file_descriptor_count () == 0,
		     ("get_file_info 1: %d file descriptors leaked", get_used_file_descriptor_count ()));
	free_at_start = get_free_file_descriptor_count ();

	/* Cancel one right after starting it. */
	test_done = FALSE;
	test_callback_data = g_malloc (1);
	uri_list = g_list_prepend (NULL, gnome_vfs_uri_new ("file:///dev/null"));
	gnome_vfs_async_get_file_info (&test_handle,
				       uri_list,
				       GNOME_VFS_FILE_INFO_DEFAULT,
				       get_file_info_callback,
				       test_callback_data);
	g_list_free (uri_list);
	gnome_vfs_async_cancel (test_handle);
	g_free (test_callback_data);

	/* Wait until it is done. */
	TEST_ASSERT (wait_until_vfs_threads_gone (), ("get_file_info 2: thread never went away"));
	TEST_ASSERT (get_used_file_descriptor_count () == 0,
		     ("get_file_info 2: %d file descriptors leaked", get_used_file_descriptor_count ()));
	TEST_ASSERT (test_done == FALSE, ("get_file_info 2: callback was called"));
	free_at_start = get_free_file_descriptor_count ();
}

static gboolean file_open_flag;

static void
file_open_callback (GnomeVFSAsyncHandle *handle,
		    GnomeVFSResult result,
		    gpointer callback_data)
{
	TEST_ASSERT (handle == test_handle, ("open callback, bad handle"));
	TEST_ASSERT (result == GNOME_VFS_OK, ("open callback, bad result"));
	TEST_ASSERT (callback_data == test_callback_data, ("open callback, bad callback data"));

	file_open_flag = TRUE;
}

static gboolean file_closed_flag;

static void
file_close_callback (GnomeVFSAsyncHandle *handle,
		     GnomeVFSResult result,
		     gpointer callback_data)
{
	TEST_ASSERT (handle == test_handle, ("close callback, bad handle"));
	TEST_ASSERT (result == GNOME_VFS_OK, ("close callback, bad result"));
	TEST_ASSERT (callback_data == test_callback_data, ("close callback, bad callback data"));

	file_closed_flag = TRUE;
}

static gboolean file_read_flag;
char read_buffer[1];

static void
file_read_callback (GnomeVFSAsyncHandle *handle,
		    GnomeVFSResult result,
		    gpointer buffer,
		    GnomeVFSFileSize bytes_requested,
		    GnomeVFSFileSize bytes_read,
		    gpointer callback_data)
{
	TEST_ASSERT (handle == test_handle, ("read callback, bad handle"));
	TEST_ASSERT (result == GNOME_VFS_OK, ("read callback, bad result"));
	TEST_ASSERT (buffer == read_buffer, ("read callback, bad buffer"));
	TEST_ASSERT (bytes_requested == 1, ("read callback, bad bytes_requested"));
	TEST_ASSERT (bytes_read == 1, ("read callback, bad bytes_read"));
	TEST_ASSERT (callback_data == test_callback_data, ("read callback, bad callback data"));

	file_read_flag = TRUE;
}

static gboolean directory_load_flag;

static void
directory_load_callback (GnomeVFSAsyncHandle *handle,
			 GnomeVFSResult result,
			 GList *list,
			 guint entries_read,
			 gpointer callback_data)
{
	GList *element;
	GnomeVFSFileInfo *info;

	for (element = list; element != NULL; element = element->next) {
		info = element->data;
		gnome_vfs_file_info_ref (info);
	}
	
	for (element = list; element != NULL; element = element->next) {
		info = element->data;
		gnome_vfs_file_info_unref (info);
	}
	
	directory_load_flag = TRUE;
}

static gboolean directory_load_failed_flag;

static void
directory_load_failed_callback (GnomeVFSAsyncHandle *handle,
				GnomeVFSResult result,
				GList *list,
				guint entries_read,
				gpointer callback_data)
{
	g_assert (result != GNOME_VFS_OK);
	directory_load_failed_flag = TRUE;
}

static void
test_open_read_close (void)
{
	file_open_flag = FALSE;
	gnome_vfs_async_open (&test_handle,
			      "file:///etc/passwd",
			      GNOME_VFS_OPEN_READ,
			      file_open_callback,
			      test_callback_data);
	TEST_ASSERT (wait_for_boolean (&file_open_flag), ("open: callback was not called"));

	file_read_flag = FALSE;
	gnome_vfs_async_read (test_handle,
			      read_buffer,
			      1,
			      file_read_callback,
			      test_callback_data);

	TEST_ASSERT (wait_for_boolean (&file_read_flag), ("open read close: read callback was not called"));
	file_closed_flag = FALSE;
	gnome_vfs_async_close (test_handle,
			       file_close_callback,
			       test_callback_data);

	TEST_ASSERT (wait_for_boolean (&file_closed_flag), ("open read close: close callback was not called"));

	TEST_ASSERT (wait_until_vfs_threads_gone (), ("open read cancel close: thread never went away"));
	TEST_ASSERT (get_used_file_descriptor_count () == 0,
		     ("open read cancel close: %d file descriptors leaked", get_used_file_descriptor_count ()));
	free_at_start = get_free_file_descriptor_count ();
}

static void
test_open_read_cancel_close (void)
{
	file_open_flag = FALSE;
	gnome_vfs_async_open (&test_handle,
			      "file:///etc/passwd",
			      GNOME_VFS_OPEN_READ,
			      file_open_callback,
			      test_callback_data);
	TEST_ASSERT (wait_for_boolean (&file_open_flag), ("open: callback was not called"));

	file_read_flag = FALSE;
	gnome_vfs_async_read (test_handle,
			      read_buffer,
			      1,
			      file_read_callback,
			      test_callback_data);
	gnome_vfs_async_cancel (test_handle);

	file_closed_flag = FALSE;
	gnome_vfs_async_close (test_handle,
			       file_close_callback,
			       test_callback_data);

	TEST_ASSERT (wait_for_boolean (&file_closed_flag), ("open read cancel close: callback was not called"));
	TEST_ASSERT (!file_read_flag, ("open read cancel close: read callback was called"));

	TEST_ASSERT (wait_until_vfs_threads_gone (), ("open read cancel close: thread never went away"));
	TEST_ASSERT (get_used_file_descriptor_count () == 0,
		     ("open read cancel close: %d file descriptors leaked", get_used_file_descriptor_count ()));
	free_at_start = get_free_file_descriptor_count ();
}

static void
test_open_close (void)
{
	file_open_flag = FALSE;
	gnome_vfs_async_open (&test_handle,
			      "file:///etc/passwd",
			      GNOME_VFS_OPEN_READ,
			      file_open_callback,
			      test_callback_data);
	TEST_ASSERT (wait_for_boolean (&file_open_flag), ("open: open callback was not called"));
	
	file_closed_flag = FALSE;
	gnome_vfs_async_close (test_handle,
			       file_close_callback,
			       test_callback_data);


	TEST_ASSERT (wait_for_boolean (&file_closed_flag), ("open close: close callback was not called"));
	
	TEST_ASSERT (wait_until_vfs_threads_gone (), ("open cancel 1: thread never went away"));
	TEST_ASSERT (get_used_file_descriptor_count () == 0,
		     ("open cancel 1: %d file descriptors leaked", get_used_file_descriptor_count ()));
	free_at_start = get_free_file_descriptor_count ();
}

static void
test_open_cancel (void)
{
	file_open_flag = FALSE;
	gnome_vfs_async_open (&test_handle,
			      "file:///etc/passwd",
			      GNOME_VFS_OPEN_READ,
			      file_open_callback,
			      test_callback_data);
	gnome_vfs_async_cancel (test_handle);

	TEST_ASSERT (wait_until_vfs_threads_gone (), ("open cancel 1: thread never went away"));
	TEST_ASSERT (!file_open_flag, ("open cancel 1: open callback was called"));
	TEST_ASSERT (get_used_file_descriptor_count () == 0,
		     ("open cancel 1: %d file descriptors leaked", get_used_file_descriptor_count ()));
	free_at_start = get_free_file_descriptor_count ();

	file_open_flag = FALSE;
	gnome_vfs_async_open (&test_handle,
			      "file:///etc/passwd",
			      GNOME_VFS_OPEN_READ,
			      file_open_callback,
			      test_callback_data);
	wait_until_vfs_threads_gone_no_main ();
	gnome_vfs_async_cancel (test_handle);
	TEST_ASSERT (wait_until_file_descriptors_gone (),
		     ("open cancel 2: %d file descriptors leaked", get_used_file_descriptor_count ()));
	free_at_start = get_free_file_descriptor_count ();
	TEST_ASSERT (wait_until_vfs_threads_gone (), ("open cancel 2: later thread never went away"));
	TEST_ASSERT (!file_open_flag, ("open cancel 2: open callback was called"));
}

static void
file_open_fail_callback (GnomeVFSAsyncHandle *handle,
			 GnomeVFSResult result,
			 gpointer callback_data)
{
	TEST_ASSERT (handle == test_handle, ("open callback, bad handle"));
	TEST_ASSERT (result == GNOME_VFS_ERROR_NOT_FOUND, ("open callback, bad result"));
	TEST_ASSERT (callback_data == test_callback_data, ("open callback, bad callback data"));

	file_open_flag = TRUE;
}

static void
test_open_fail (void)
{
	file_open_flag = FALSE;
	gnome_vfs_async_open (&test_handle,
			      "file:///etc/mugwump-xxx",
			      GNOME_VFS_OPEN_READ,
			      file_open_fail_callback,
			      test_callback_data);
	TEST_ASSERT (wait_for_boolean (&file_open_flag), ("open fail 1: callback was not called"));
	TEST_ASSERT (wait_until_vfs_threads_gone (), ("open fail 1: thread never went away"));
	TEST_ASSERT (get_used_file_descriptor_count () == 0,
		     ("open fail 1: %d file descriptors leaked", get_used_file_descriptor_count ()));
	free_at_start = get_free_file_descriptor_count ();
}

static void
my_yield (int count)
{
	for (; count > 0; count--) {
		usleep (100);
		gtk_main_iteration_do (FALSE);
	}
}

static void
test_load_directory_cancel (int delay_till_cancel, int chunk_count)
{
	GnomeVFSAsyncHandle *handle;
	guint num_entries;
	
	
	gnome_vfs_async_load_directory (&handle,
					"file:///etc",
					GNOME_VFS_FILE_INFO_GET_MIME_TYPE
		 			 | GNOME_VFS_FILE_INFO_FORCE_FAST_MIME_TYPE
		 			 | GNOME_VFS_FILE_INFO_FOLLOW_LINKS,
					GNOME_VFS_DIRECTORY_FILTER_NONE,
					0,
					NULL,
					chunk_count,
					directory_load_callback,
					&num_entries);
	
	usleep (delay_till_cancel * 100);
	
	directory_load_flag = FALSE;
	gnome_vfs_async_cancel (handle);
	TEST_ASSERT (wait_until_vfs_threads_gone (), ("open cancel 1: thread never went away"));
	TEST_ASSERT (!directory_load_flag, ("load directory cancel 1: load callback was called"));

	gnome_vfs_async_load_directory (&handle,
					"file:///etc",
					GNOME_VFS_FILE_INFO_GET_MIME_TYPE
		 			 | GNOME_VFS_FILE_INFO_FORCE_FAST_MIME_TYPE
		 			 | GNOME_VFS_FILE_INFO_FOLLOW_LINKS,
					GNOME_VFS_DIRECTORY_FILTER_NONE,
					0,
					NULL,
					chunk_count,
					directory_load_callback,
					&num_entries);
	
	my_yield (delay_till_cancel);
	
	directory_load_flag = FALSE;
	gnome_vfs_async_cancel (handle);
	TEST_ASSERT (wait_until_vfs_threads_gone (), ("open cancel 2: thread never went away"));
	TEST_ASSERT (!directory_load_flag, ("load directory cancel 2: load callback was called"));
}

static void
test_load_directory_fail (void)
{
	GnomeVFSAsyncHandle *handle;
	guint num_entries;
	
	directory_load_failed_flag = FALSE;
	gnome_vfs_async_load_directory (&handle,
					"file:///strcprstskrzkrk",
					GNOME_VFS_FILE_INFO_GET_MIME_TYPE
		 			 | GNOME_VFS_FILE_INFO_FORCE_FAST_MIME_TYPE
		 			 | GNOME_VFS_FILE_INFO_FOLLOW_LINKS,
					GNOME_VFS_DIRECTORY_FILTER_NONE,
					0,
					NULL,
					32,
					directory_load_failed_callback,
					&num_entries);
		
	TEST_ASSERT (wait_for_boolean (&directory_load_failed_flag), ("load directory cancel 1: load callback was not called"));
	TEST_ASSERT (wait_until_vfs_threads_gone (), ("open cancel 1: thread never went away"));
}

static gboolean find_directory_flag;

static void
test_find_directory_callback (GnomeVFSAsyncHandle *handle,
			      GList *results,
			      gpointer callback_data)
{
	GList *element;

	find_directory_flag = TRUE;

	for (element = results; element != NULL; element = element->next) {
		GnomeVFSFindDirectoryResult *result_item = (GnomeVFSFindDirectoryResult *)element->data;
		
		if (result_item->result == GNOME_VFS_OK) {
			gnome_vfs_uri_ref (result_item->uri);
			gnome_vfs_uri_unref (result_item->uri);
		}
	}
	
	g_assert (callback_data == &find_directory_flag);
}

static void
test_find_directory (int delay_till_cancel)
{
	GnomeVFSAsyncHandle *handle;
	GList *vfs_uri_as_list;


	vfs_uri_as_list = g_list_append (NULL, gnome_vfs_uri_new ("file://~"));
	vfs_uri_as_list = g_list_append (vfs_uri_as_list, gnome_vfs_uri_new ("file:///ace_of_spades"));
	
	find_directory_flag = FALSE;
	
	gnome_vfs_async_find_directory (&handle, vfs_uri_as_list,
		GNOME_VFS_DIRECTORY_KIND_TRASH, FALSE, TRUE, 0777,
		test_find_directory_callback, &find_directory_flag);
		
	TEST_ASSERT (wait_for_boolean (&find_directory_flag), ("find directory cancel 1: callback was not called"));
	
	find_directory_flag = FALSE;
	
	gnome_vfs_async_find_directory (&handle, vfs_uri_as_list,
		GNOME_VFS_DIRECTORY_KIND_TRASH, FALSE, TRUE, 0777,
		test_find_directory_callback, &find_directory_flag);
	
	usleep (delay_till_cancel * 100);
	
	gnome_vfs_async_cancel (handle);
	TEST_ASSERT (wait_until_vfs_threads_gone (), ("open cancel 2: thread never went away"));
	TEST_ASSERT (!find_directory_flag, ("find directory cancel 2: callback was called"));

	
	gnome_vfs_async_find_directory (&handle, vfs_uri_as_list,
		GNOME_VFS_DIRECTORY_KIND_TRASH, FALSE, TRUE, 0777,
		test_find_directory_callback, &find_directory_flag);
	
	my_yield (delay_till_cancel);
	
	find_directory_flag = FALSE;
	gnome_vfs_async_cancel (handle);
	TEST_ASSERT (wait_until_vfs_threads_gone (), ("open cancel 3: thread never went away"));
	TEST_ASSERT (!find_directory_flag, ("find directory cancel 3: callback was called"));

	gnome_vfs_uri_list_free (vfs_uri_as_list);
}

int
main (int argc, char **argv)
{
	CORBA_Environment ev;

	make_asserts_break("GnomeVFS");
	/* Initialize the libraries we use. */
	g_thread_init (NULL);
	CORBA_exception_init (&ev);
	gnome_CORBA_init ("test-async-cancel", "0.0", &argc, argv, 0, &ev);
	gnome_vfs_init ();

	
	/* Initialize our own stuff. */
	free_at_start = get_free_file_descriptor_count ();

	/* Do the basic tests of our own tools. */
	TEST_ASSERT (get_used_file_descriptor_count () == 0, ("file descriptor count"));
	TEST_ASSERT (gnome_vfs_backend_get_job_count () == 0, ("VFS thread count"));

	/* Spend those first few file descriptors. */
	first_get_file_info ();
	free_at_start = get_free_file_descriptor_count ();

	/* Test to see that a simple async. call works without leaking or anything. */
	test_get_file_info ();
	test_get_file_info ();
	test_open_close ();
	test_open_close ();
	test_open_read_close ();
	test_open_read_close ();
	test_open_cancel ();
	test_open_cancel ();

	test_open_fail ();
	test_open_fail ();
	test_open_read_cancel_close ();
	test_open_read_cancel_close ();

	test_load_directory_fail ();
	test_load_directory_cancel (0, 1);
	test_load_directory_cancel (1, 1);
	test_load_directory_cancel (10, 1);
	test_load_directory_cancel (100, 1);
	test_load_directory_cancel (0, 1);
	test_load_directory_cancel (1, 1);
	test_load_directory_cancel (10, 1);
	test_load_directory_cancel (100, 1);

	test_load_directory_cancel (0, 32);
	test_load_directory_cancel (1, 32);
	test_load_directory_cancel (10, 32);
	test_load_directory_cancel (100, 32);
	test_load_directory_cancel (0, 32);
	test_load_directory_cancel (1, 32);
	test_load_directory_cancel (10, 32);
	test_load_directory_cancel (100, 32);

	test_find_directory (0);
	test_find_directory (0);
	test_find_directory (1);
	test_find_directory (1);
	test_find_directory (10);
	test_find_directory (10);
	test_find_directory (100);
	test_find_directory (100);
		
	gnome_vfs_shutdown ();
	/* Report to "make check" on whether it all worked or not. */
	return at_least_one_test_failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
