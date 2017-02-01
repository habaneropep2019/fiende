/* $Id: dict.c,v 1.14 2001/06/22 22:15:42 kmaraas Exp $ */
/* -*- mode: c; style: k&r; c-basic-offset: 4 -*- */

/*
 *  Bradford Hovinen <hovinen@udel.edu>
 *  Papadimitriou Spiros <spapadim+@cs.cmu.edu> (original version)
 *
 *  This code released under the GNU GPL.
 *  Read the file COPYING for more information.
 *
 *  MIT dictionary library
 *
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>

#include <unistd.h>

#include <glib.h>
#include <gnome.h>

#include "dict.h"

static gint dict_context_do_dns_lookup (dict_context_t *context);

/* dict_res_new (name, desc)
 *
 * Constructs a new result structure
 */

dict_res_t *dict_res_new (gchar *name, gchar *desc) 
{
    dict_res_t *res;
    
    res = g_new0 (dict_res_t, 1);
    res->name = name;
    res->desc = desc;
    
    return res;
}

/* dict_res_destroy (res)
 *
 * Destroys a result structure
 */

void dict_res_destroy (dict_res_t *res) 
{
    g_return_if_fail (res != NULL);
    
    if (res->free_name) g_free (res->name);
    if (res->free_desc) g_free (res->desc);
    g_free (res);
}

/* dict_write (str)
 *
 * Writes data to the DICT server
 * 
 *   str - The string to write
 *
 * Returns 0 on success, -1 on socket error
 */

static int dict_write (dict_context_t *context, GString *str) 
{
    int res, amt;
    
    res = g_io_channel_write(context->channel, str->str, str->len, &amt);
    
    if (res != G_IO_ERROR_NONE)
	return -1;
    else
	return 0;
}

/* dict_command_done ()
 *
 * Performs cleanup when done communicating with the server
 */

static void
dict_command_done (dict_command_t *command) 
{
    dict_context_t *context = command->context;

    command->context = NULL;
    command->state = S_DONE;
    
    if (context) {
        context->command = NULL;
        if (command->temp_context) {
            if (context->channel && command->cmd != C_DISCONNECT) {
                dict_command_t *sec_cmd = dict_disconnect_command_new ();
                sec_cmd->temp_context = TRUE;
                dict_command_invoke (sec_cmd, context);
            } else {
                dict_context_destroy (context);
            }
	}
    }
}

/* dict_cycle_buffer ()
 *
 * Reads additional data from the socket if it can
 *
 * Returns 0 on success, 1 if there is no more data to read, -1 on socket error
 */

static int 
cycle_buffer (dict_context_t *context) 
{
    int amt_read, res;
    
    if (!context->read_cycle_done) {
        if (context->write_ptr > context->read_ptr)
	    memmove (context->read_buf, context->read_ptr,
		     context->write_ptr - context->read_ptr);
        context->write_ptr -= context->read_ptr - context->read_buf;
        context->read_ptr = context->read_buf;
        
        res = g_io_channel_read (context->channel, context->write_ptr,
                                 context->read_buf + READ_BUF_LEN 
				 - context->write_ptr, 
                                 &amt_read);
        
        if (res == G_IO_ERROR_AGAIN)
	    context->read_cycle_done = TRUE;
        else if (res != G_IO_ERROR_NONE)
	    return -1;
        
        context->write_ptr += amt_read;
        return 0;
    }
    else {
        return 1;
    }
}

/* dict_read_line ()
 *
 * Reads a line of text from the socket and performs a CR-LF translation
 *
 * Returns a pointer to the string on success
 */

static char *dict_read_line (dict_context_t *context) {
    char *start_ptr, *end_ptr;
    
    if (!context->channel) return NULL;
    
    while (1) {
        end_ptr = strchr (context->read_ptr, '\r');
        
        if (end_ptr == NULL || end_ptr > context->write_ptr) {
            if (cycle_buffer (context)) return NULL;
        }
        else {
            break;
        }
    }
    
    end_ptr[0] = '\n';
    end_ptr[1] = '\0';
    
    start_ptr = context->read_ptr;
    context->read_ptr = end_ptr + 2;
    
    return g_strdup (start_ptr);
}

/* data_notify_match (line)
 *
 * Notification function called when receiving response from a MATCH command
 */

static void data_notify_match (dict_context_t *context, char *line) 
{
    DictStatusCode status_code;
    int num_matches;
    char *word, *desc;
    dict_res_t *new_res;
    dict_command_t *command;
    
    command = context->command;
    
    switch (command->state) {
    case S_GROUND:      /* Uh oh... problem */
    case S_DONE:
        g_error ("In ground state when invoking MATCH command");
        break;
        
    case S_STATUS:
        status_code = (DictStatusCode) atoi (line);
        while (isdigit (*line)) line++;
        while (isspace (*line)) line++;
        
        num_matches = 0;
        
        switch (status_code) {
	case DICT_STATUS_MATCHES_FOUND:
            num_matches = atoi (line);
            command->state = S_DATA;
            if (command->status_notify_cb)
		command->status_notify_cb (command, status_code, num_matches,
					   command->user_data);
            break;
            
	case DICT_STATUS_BAD_DB:
	case DICT_STATUS_BAD_STRAT:
	case DICT_STATUS_BAD_COMMAND:
	case DICT_STATUS_BAD_PARAMS:
	case DICT_STATUS_CMD_NO_IMPL:
	case DICT_STATUS_PARAM_NO_IMPL:
	case DICT_STATUS_NO_ACCESS:
            dict_command_done (command);
            if (command->error_notify_cb)
		command->error_notify_cb (command, status_code, line,
					  command->user_data);
            break;
            
	case DICT_STATUS_NO_MATCH:
	case DICT_STATUS_OK:
            dict_command_done (command);
            if (command->status_notify_cb)
		command->status_notify_cb (command, status_code, 0,
					   command->user_data);
            break;
            
	default:
            g_error ("Invalid status code given");
        }
        break;
        
    case S_DATA:
        if (line != NULL && strcmp (line, ".\n") == 0) {
            command->state = S_STATUS;
        }
        else {
            word = line;
            
            line = strchr (line, ' ');
            *line = '\0'; line++;
            if (*line == '\"') line++;
            desc = line;
            while (*line && *line != '\n' && *line != '\"') line++;
            *line = '\0';
            
            new_res = dict_res_new (g_strdup (word), g_strdup (desc));
            new_res->free_name = TRUE;
            new_res->free_desc = TRUE;
            
            command->res_tail = 
		g_list_append (command->res_tail, new_res);
            if (!command->res_list) 
		command->res_list = command->res_tail;
            
            if (command->data_notify_cb)
		command->data_notify_cb (command, new_res,
					 command->user_data);
        }
    }
}

/* data_notify_define (line)
 *
 * Notification function called when receiving response from a DEFINE command
 */

static void data_notify_define (dict_context_t *context, char *line) 
{
    DictStatusCode status_code;
    int num_found;
    char *buf;
    dict_res_t *new_res;
    dict_command_t *command;
    
    command = context->command;
    
    switch (command->state) {
    case S_GROUND:      /* Uh oh... problem */
    case S_DONE:
        g_error ("In ground state when invoking DEFINE command");
        break;
        
    case S_STATUS:
        status_code = (DictStatusCode) atoi (line);
        while (isdigit (*line)) line++;
        while (isspace (*line)) line++;
        
        num_found = 0;
        
        switch (status_code) {
	case DICT_STATUS_DEF_RETRIEVED:
            num_found = atoi (line);
            if (command->status_notify_cb)
		command->status_notify_cb (command, status_code, num_found,
					   command->user_data);
            break;
            
	case DICT_STATUS_WORD_DB_NAME:
            while (*line != '\"') line++; line++;
            while (*line != '\"') line++; line++;
            while (isspace (*line)) line++;
            
            buf = line;
            while (!isspace (*buf)) buf++;
            *buf = '\0';
            
            command->db_name = g_strdup (line);
            command->def = g_string_new (NULL);
            command->state = S_DATA;
            break;
            
	case DICT_STATUS_BAD_DB:
	case DICT_STATUS_BAD_COMMAND:
	case DICT_STATUS_BAD_PARAMS:
	case DICT_STATUS_CMD_NO_IMPL:
	case DICT_STATUS_PARAM_NO_IMPL:
	case DICT_STATUS_NO_ACCESS:
            dict_command_done (command);
            if (command->error_notify_cb)
		command->error_notify_cb (command, status_code, line,
					  command->user_data);
            break;
            
	case DICT_STATUS_NO_MATCH:
	case DICT_STATUS_OK:
            dict_command_done (command);
            if (command->status_notify_cb)
		command->status_notify_cb (command, status_code, 0,
					   command->user_data);
            break;
            
	default:
            g_error ("Invalid status code given");
        }
        break;
        
    case S_DATA:
        if (!strcmp (line, ".\n")) {
            new_res = dict_res_new (command->db_name, command->def->str);
            new_res->free_name = new_res->free_desc = TRUE;
            
            g_string_free (command->def, FALSE);
	    command->def = NULL;
            
            command->res_tail = g_list_append (command->res_tail, new_res);
            if (!command->res_list) command->res_list = command->res_tail;
            
            command->state = S_STATUS;
            if (command->data_notify_cb)
		command->data_notify_cb (command, new_res,
					 command->user_data);
        }
        else {
            g_string_append (command->def, line);
        }
    }
}

/* data_notify_show_db (line)
 *
 * Notification function called when receiving response from a SHOW DB command
 */

static void data_notify_show_db (dict_context_t *context, char *line) 
{
    DictStatusCode status_code;
    int num_found;
    char *name, *desc;
    dict_res_t *new_res;
    dict_command_t *command;
    
    command = context->command;
    
    switch (command->state) {
    case S_GROUND:      /* Uh oh... problem */
    case S_DONE:
        g_error ("In ground state when invoking SHOW DB command");
        break;
        
    case S_STATUS:
        status_code = (DictStatusCode) atoi (line);
        while (isdigit (*line)) line++;
        while (isspace (*line)) line++;
        
        num_found = 0;
        
        switch (status_code) {
	case DICT_STATUS_DB_PRESENT:
            num_found = atoi (line);
            command->state = S_DATA;
            if (command->status_notify_cb)
		command->status_notify_cb (command, status_code, num_found,
					   command->user_data);
            break;
            
	case DICT_STATUS_NO_DB:
	case DICT_STATUS_OK:
            dict_command_done (command);
            if (command->status_notify_cb)
		command->status_notify_cb (command, status_code, 0,
					   command->user_data);
            break;
            
	default:
            g_error ("Invalid status code given");
        }
        break;
        
    case S_DATA:
        if (!strcmp (line, ".\n")) {
            command->state = S_STATUS;
        }
        else {
            name = line;
            line = strchr (line, ' ');
            *line = '\0'; line++;
            if (*line == '\"') line++;
            desc = line;
            while (*line && *line != '\n' && *line != '\"') line++;
            *line = '\0';
            
            new_res = dict_res_new (g_strdup (name), g_strdup (desc));
            new_res->free_name = TRUE;
            new_res->free_desc = TRUE;
            
            command->res_tail = 
		g_list_append (command->res_tail, new_res);
            if (!command->res_list) 
		command->res_list = command->res_tail;
            
            if (command->data_notify_cb)
		command->data_notify_cb (command, new_res,
					 command->user_data);
        }
    }
}

/* data_notify_show_strat (line)
 *
 * Notification function called when receiving response from a SHOW STRAT
 * command
 */

static void data_notify_show_strat (dict_context_t *context,
				    char *line) 
{
    DictStatusCode status_code;
    int num_found;
    char *newline;
    char *name, *desc;
    dict_res_t *new_res;
    dict_command_t *command;
    
    command = context->command;
    
    switch (command->state) {
    case S_GROUND:      /* Uh oh... problem */
    case S_DONE:
        g_error ("In ground state when invoking SHOW STRAT command");
        break;
        
    case S_STATUS:
        status_code = (DictStatusCode) atoi (line);
        while (isdigit (*line)) line++;
        while (isspace (*line)) line++;
        
        num_found = 0;
        
        switch (status_code) {
	case DICT_STATUS_STRAT_PRESENT:
            num_found = atoi (line);
            command->state = S_DATA;
            if (command->status_notify_cb)
		command->status_notify_cb (command, status_code, num_found,
					   command->user_data);
            break;
            
	case DICT_STATUS_NO_STRAT:
	case DICT_STATUS_OK:
            dict_command_done (command);
            if (command->status_notify_cb)
		command->status_notify_cb (command, status_code, 0,
					   command->user_data);
            break;
            
	default:
            g_error ("Invalid status code given");
        }
        break;
        
    case S_DATA:
        if (!strcmp (line, ".\n")) {
            command->state = S_STATUS;
        }
        else {
	    newline = line = g_strdup (line);
            name = line;
            line = strchr (line, ' ');
            *line = '\0'; line++;
            if (*line == '\"') line++;
            desc = line;
            while (*line && *line != '\n' && *line != '\"') line++;
            *line = '\0';
            
            new_res = dict_res_new (g_strdup (name), g_strdup (desc));
            new_res->free_name = TRUE;
            new_res->free_desc = TRUE;

	    g_free (newline);
            
            command->res_tail = 
		g_list_append (command->res_tail, new_res);
            if (!command->res_list) 
		command->res_list = command->res_tail;
            
            if (command->data_notify_cb)
		command->data_notify_cb (command, new_res,
					 command->user_data);
        }
    }
}

/* data_notify_connect (context)
 *
 * Closes the connection to the DICT server
 */

static void data_notify_connect (dict_context_t *context, char *line) 
{
    dict_command_t *command;
    
    command = context->command;
    
    switch (command->state) {
    case S_GROUND:      /* Uh oh... problem */
    case S_DATA:
    case S_DONE:
        g_error ("Bad command state");
        break;
        
    case S_STATUS:
        if (command->trigger_cmd) {
            if (command->data_notify_cb)
		command->data_notify_cb (command, 
					 (dict_res_t *) command->trigger_cmd,
					 command->user_data);
        }
        else {
            dict_command_destroy (command);
            command = NULL;
        }
        
        break;
    }
}

/* data_notify_quit (context)
 *
 * Closes the connection to the DICT server
 */

static void data_notify_quit (dict_context_t *context, char *line) 
{
    gboolean reset_command_field;
    dict_command_t *command;
    
    command = context->command;
    
    switch (command->state) {
    case S_GROUND:      /* Uh oh... problem */
    case S_DATA:
    case S_DONE:
        g_error ("In ground state when invoking SHOW STRAT command");
        break;
        
    case S_STATUS:
        dict_disconnect (context);
        reset_command_field = !command->temp_context;
        dict_command_destroy (command);
        if (reset_command_field) command = NULL;
        break;
    }
}

/* dict_data_notify (channel, cond, data)
 *
 * Callback invoked when data comes in from the server
 * This is the main engine of the DICT implementation; it parses out responses
 * from the server and invokes the necessary callbacks
 */

static gboolean dict_data_notify (GIOChannel *channel, GIOCondition cond, 
                                  gpointer data)
{
    char *line;
    dict_context_t *context;
    
    context = (dict_context_t *) data;

    /* HAAAAAAAAAAACK, we never free context because there is
     * some sort of corruption or what not and this gets called,
     * even after connection was closed */
    if (context->channel == NULL)
	    return FALSE;
    
    if (cond == G_IO_IN) {
        context->read_cycle_done = FALSE;
        
        while ((line = dict_read_line (context))) {
            if (!context->command)
		g_error ("Data received in ground state");
            else
		context->command->raw_data_notify_cb (context, line);
	    g_free (line);
        }
    }
    else if (cond == G_IO_ERR) {
        if (context->command->error_notify_cb)
	    context->command->error_notify_cb (context->command, 
					       DICT_SOCKET_ERROR,
					       _("Cannot connect to server"),
					       context->command->user_data);
        dict_disconnect (context);
	return FALSE;
    }
    
    return TRUE;
}

/* dict_context_new (server, port)
 * 
 * Constructs a new context object for a given server and port
 *
 *   server - Hostname of the server to use
 *   port - Port number
 *
 * Returns pointer on success, NULL on lookup error
 */

dict_context_t *dict_context_new (gchar *server, gushort port) 
{
    dict_context_t *context;

    context = g_new0 (dict_context_t, 1);
    context->hostname = g_strdup
	    ((server != NULL) ? server : DICT_DEFAULT_SERVER);
    context->hostinfo = NULL;
    context->sockaddr.sin_family = AF_INET;
    context->sockaddr.sin_port =
	htons ((port > 0) ? port : DICT_DEFAULT_PORT);
    context->read_ptr = context->write_ptr = context->read_buf;

    dict_context_do_dns_lookup (context);

    return context;
}

/* dict_context_clone (context)
 * 
 * Clones an existing context object
 */

dict_context_t *dict_context_clone (dict_context_t *context) 
{
    dict_context_t *new_context;

    new_context = g_new0 (dict_context_t, 1);
    
    bcopy (&context->sockaddr, &new_context->sockaddr,
	   sizeof (context->sockaddr));
    new_context->hostname = g_strdup (context->hostname);
    new_context->hostinfo = context->hostinfo;
    new_context->read_ptr = new_context->write_ptr =
	new_context->read_buf;
    
    return new_context;
}

/* dict_context_destroy (context)
 *
 * Destroys a context object
 */

void dict_context_destroy (dict_context_t *context) 
{
    g_return_if_fail (context != NULL);

    dict_disconnect (context);
    g_free (context->hostname);
    context->hostname = NULL;
    /* FOOOOOOOO, HAAAAAAAAAAACK, the notify handler still gets called,
     * I do not understand this, this keeps it from dieing */
    /*g_free (context);*/
}

/* dict_connect (context)
 *
 * Opens a connection to the DICT server.
 *
 * Returns 0 on success, -1 on connection error
 */

int dict_connect (dict_context_t *context) 
{
    int sock_fd;
    int old_flags;

    g_return_val_if_fail (context != NULL, 0);

    if (context->channel) return 0;
    
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	return -1;
    
    if ((old_flags = fcntl (sock_fd, F_GETFL, 0)) == -1)
	return -1;
    
    if (fcntl (sock_fd, F_SETFL, old_flags | O_NONBLOCK) == -1)
	return -1;
    
    if (connect(sock_fd, (struct sockaddr*)&context->sockaddr, sizeof(context->sockaddr)) != 0
	&& errno != EINPROGRESS)
	return -1;
    
    context->channel = g_io_channel_unix_new (sock_fd);
    context->source_id = 
	g_io_add_watch (context->channel, G_IO_IN | G_IO_ERR, 
			dict_data_notify, context);
    
    return 0;
}

/* dict_disconnect (context)
 *
 * Opens a connection to the DICT server.
 *
 * Returns 0 on success, -1 on connection error
 */

void dict_disconnect (dict_context_t *context) 
{
    g_return_if_fail (context != NULL);

    if (context->source_id > 0) {
	    g_source_remove (context->source_id);
	    context->source_id = 0;
    }

    if (context->channel != NULL) {
	    g_io_channel_close (context->channel);
	    g_io_channel_unref (context->channel);
	    context->channel = NULL;
    }
}

/* dict_command_new ()
 *
 * Create a new oommand structure
 */

dict_command_t *dict_command_new (void) 
{
    dict_command_t *command;
    
    command = g_new0 (dict_command_t, 1);

    return command;
}

/* dict_connect_command_new ()
 *
 * Sends the command to disconnect from the server
 */

dict_command_t *dict_connect_command_new (void) 
{
    dict_command_t *command;
    
    command = dict_command_new ();
    
    command->cmd = C_CONNECT;
    command->state = S_GROUND;
    command->raw_data_notify_cb = data_notify_connect;
    
    return command;
}

/* dict_disconnect_command_new ()
 *
 * Sends the command to the server to retrieve the names of the strategies
 * that the server supports
 */

dict_command_t *dict_disconnect_command_new (void) 
{
    dict_command_t *command;
    
    command = dict_command_new ();
    
    command->cmd_string = g_string_new ("QUIT\r\n");
    
    command->cmd = C_DISCONNECT;
    command->state = S_GROUND;
    command->raw_data_notify_cb = data_notify_quit;
    
    return command;
}

/* dict_match_command_new (database, strategy, word)
 *
 * Sends the command to the server to match a word to the contents of a
 * database using the given strategy
 *
 *   database: Database to check; a valid list can be obtained with
 *             dict_get_db; defaults to `!' (check all databases)
 *   strategy: Strategy to use; a valid list can be obtained with
 *             dict_get_strat; defaults to `.' (server-dependent default)
 *   word: Word to check
 */

dict_command_t *dict_match_command_new (char *db, char *strategy,
					char *word) 
{
    dict_command_t *command;
    
    command = dict_command_new ();
    
    if (!db) db = "!";
    if (!strategy) strategy = ".";
    
    command->cmd_string = g_string_new (NULL);
    g_string_sprintf (command->cmd_string, "MATCH %s %s \"%s\"\r\n", 
                      db, strategy, word);
    
    command->cmd = C_MATCH;
    command->state = S_GROUND;
    command->search_term = g_strdup (word);
    command->search_db = g_strdup (db);
    command->search_strat = g_strdup (strategy);
    command->raw_data_notify_cb = data_notify_match;
    
    return command;
}

/* dict_define_command_new (database, word)
 *
 * Sends the command to the server to get the definition for a word
 *
 *   database: Database to check; a valid list can be obtained with
 *             dict_get_db; defaults to `!' (check all databases)
 *   word: Word to find
 */

dict_command_t *dict_define_command_new (char *db, char *word) 
{
    dict_command_t *command;

    command = dict_command_new ();
    
    if (!db) db = "!";
    
    command->cmd_string = g_string_new (NULL);
    g_string_sprintf(command->cmd_string, "DEFINE %s \"%s\"\r\n", db, word);
    
    command->cmd = C_DEFINE;
    command->state = S_GROUND;
    command->search_term = g_strdup (word);
    command->search_db = g_strdup (db);
    command->raw_data_notify_cb = data_notify_define;
    
    return command;
}

/* dict_show_db_command_new ()
 *
 * Sends the command to the server to retrieve the names of all the databases
 */

dict_command_t *dict_show_db_command_new (void) 
{
    dict_command_t *command;

    command = dict_command_new ();
    
    command->cmd_string = g_string_new ("SHOW DB\r\n");
    
    command->cmd = C_SHOW_DB;
    command->state = S_GROUND;
    command->raw_data_notify_cb = data_notify_show_db;
    
    return command;
}

/* dict_show_strat_command_new ()
 *
 * Sends the command to the server to retrieve the names of the strategies
 * that the server supports
 */

dict_command_t *dict_show_strat_command_new (void) 
{
    dict_command_t *command;

    command = dict_command_new ();
    
    command->cmd_string = g_string_new ("SHOW STRAT\r\n");
    
    command->cmd = C_SHOW_STRAT;
    command->state = S_GROUND;
    command->raw_data_notify_cb = data_notify_show_strat;
    
    return command;
}

/* dict_command_destroy (command)
 *
 * Destroys the data associated with a command object
 */

void dict_command_destroy (dict_command_t *command) 
{
    dict_res_t *res;
    
    if (!command)
    	return;

    /* Make sure it has disconnected from server */
    dict_command_done (command);
    
    g_free (command->search_term);
    g_free (command->search_db);
    g_free (command->search_strat);
    
    while (command->res_list) {
        res = (dict_res_t *) command->res_list->data;
        dict_res_destroy (res);
        command->res_list = 
	    g_list_remove_link (command->res_list, command->res_list);
    }
    
    g_free (command);
}

static void trigger_cmd (dict_command_t *command, dict_command_t *trigger_cmd,
                         gpointer user_data)
{
    command->context->command = NULL;
    dict_command_destroy (command);
    dict_command_invoke (trigger_cmd, trigger_cmd->context);
}

/* dict_command_invoke (command, context)
 *
 * Sends the command to the server associated with the given context
 *
 * Returns 0 on success, -1 on socket error, -2 on DNS lookup failure
 */

int dict_command_invoke (dict_command_t *command,
			 dict_context_t *context) 
{
    dict_command_t *sec_cmd;
    
    g_return_val_if_fail (command != NULL, 0);
    g_return_val_if_fail (context != NULL || command->context != NULL, 0);

    if (!context) context = command->context;

    if (context->hostinfo == NULL) {
	if (dict_context_do_dns_lookup (context) < 0) return -2;
    }

    /* If the context is already in use, clone it and set the new context
     * to temporary (unless it is a connect or disconnect commmand)
     */
     
    if (context->command
	&& command->cmd != C_CONNECT && command->cmd != C_DISCONNECT)
    {
        command->context = dict_context_clone (context);
        command->temp_context = TRUE;
    }
    
    /* If this is a disconnect command and the context is in use, we want
     * force a disconnect 
     */
     
    else if (context->command && command->cmd == C_DISCONNECT) {
        dict_disconnect (context);
        return 0;
    }
    
    /* Otherwise, the context is free, so just assign it as the context we
     * are going to use
     */
     
    else if (context->command == NULL) {
        command->context = context;
    }
    
    
    /* If this context is not already connected to the server, initiate
     * a command to connect it and set the given command to be triggered
     * when the connection is established
     */
    
    if (!command->context->channel && command->cmd != C_CONNECT) {
        sec_cmd = dict_connect_command_new ();
        sec_cmd->trigger_cmd = command;
        sec_cmd->data_notify_cb = 
            (void (*) (dict_command_t *, dict_res_t *, gpointer)) trigger_cmd;
        sec_cmd->error_notify_cb = command->error_notify_cb;
        sec_cmd->user_data = command->user_data;
        return dict_command_invoke (sec_cmd, command->context);
    }
    
    /* If this is a connect command, then go ahead and connect to the server */
    
    else if (command->cmd == C_CONNECT) {
        command->context->command = command;
        command->state = S_STATUS;
        return dict_connect (command->context);
    }
    
    /* Otherwise, send the command string to the server */
    
    else {
        command->context->command = command;
        command->state = S_STATUS;
        if (dict_write (command->context, command->cmd_string)) return -1;
    }
    
    return 0;
}

/* dict_context_do_dns_lookup (context)
 * 
 * Perform DNS lookup on a context and set the host info and server
 * address fields based on the results
 * 
 * Returns 0 on success, -1 on lookup failure
 */

gint dict_context_do_dns_lookup (dict_context_t *context) 
{
    g_return_val_if_fail (context != NULL, 0);

    if (context->hostinfo != NULL) return 0;

    if (context->hostname == NULL) return -1;

    if ((context->hostinfo = gethostbyname (context->hostname)) != NULL) {
	bcopy(context->hostinfo->h_addr, &context->sockaddr.sin_addr,
	      context->hostinfo->h_length);
	return 0;
    }
    else {
	return -1;
    }
}
