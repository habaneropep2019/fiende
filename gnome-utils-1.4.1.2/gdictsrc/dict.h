#ifndef __DICT_H_
#define __DICT_H_

/* $Id: dict.h,v 1.7 2000/06/25 21:35:37 hovinen Exp $ */

/*
 *  Bradford Hovinen <hovinen@udel.edu>
 *  Papadimitriou Spiros <spapadim+@cs.cmu.edu> (original version)
 *
 *  MIT dictionary library
 *
 *  This code released under the GNU GPL.
 *  Read the file COPYING for more information.
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define DICT_DEFAULT_SERVER "dict.org"
#define DICT_DEFAULT_PORT   2628

#define READ_BUF_LEN 4096

typedef enum _cmd_t {
    C_CONNECT, C_DISCONNECT, C_MATCH, C_DEFINE, C_SHOW_DB, C_SHOW_STRAT
} cmd_t;

typedef enum _cmd_state_t {
    S_GROUND, S_STATUS, S_DATA, S_DONE
} cmd_state_t;

/* Status messages */

typedef enum {
    DICT_SOCKET_ERROR         = 0,
    DICT_STATUS_DB_PRESENT    = 110,
    DICT_STATUS_STRAT_PRESENT = 111,
    DICT_STATUS_DB_INFO       = 112,
    DICT_STATUS_HELP_TEXT     = 113,
    DICT_STATUS_SERVER_INFO   = 114,
    DICT_STATUS_CHALLENGE     = 130,
    DICT_STATUS_DEF_RETRIEVED = 150,
    DICT_STATUS_WORD_DB_NAME  = 151,
    DICT_STATUS_MATCHES_FOUND = 152,
    DICT_STATUS_CONNECT       = 220,
    DICT_STATUS_QUIT          = 221,
    DICT_STATUS_AUTH_OK       = 230,
    DICT_STATUS_OK            = 250,
    DICT_STATUS_SEND_RESPONSE = 330,
    DICT_STATUS_SERVER_DOWN   = 420,
    DICT_STATUS_SHUTDOWN      = 421,
    DICT_STATUS_BAD_COMMAND   = 500,
    DICT_STATUS_BAD_PARAMS    = 501,
    DICT_STATUS_CMD_NO_IMPL   = 502,
    DICT_STATUS_PARAM_NO_IMPL = 503,
    DICT_STATUS_NO_ACCESS     = 530,
    DICT_STATUS_USE_SHOW_INFO = 531,
    DICT_STATUS_UNK_MECH      = 532,
    DICT_STATUS_BAD_DB        = 550,
    DICT_STATUS_BAD_STRAT     = 551,
    DICT_STATUS_NO_MATCH      = 552,
    DICT_STATUS_NO_DB         = 554,
    DICT_STATUS_NO_STRAT      = 555
} DictStatusCode;

typedef struct _dict_res_t {
    gchar *name;
    gchar *desc;
    gboolean free_name;
    gboolean free_desc;
} dict_res_t;

struct _dict_context_t;

typedef struct _dict_command_t {
    struct _dict_context_t *context;
    
    GString      *cmd_string;
    
    cmd_t         cmd;
    cmd_state_t   state;
    void        (*raw_data_notify_cb) (struct _dict_context_t *, gchar *);
    void        (*error_notify_cb)  (struct _dict_command_t *,
				     DictStatusCode code,
				     char *message, gpointer data);
    void        (*status_notify_cb) (struct _dict_command_t *,
				     DictStatusCode code,
				     int num_found, gpointer data);
    void        (*data_notify_cb)   (struct _dict_command_t *,
				     dict_res_t *,
				     gpointer data);
    
    gchar        *db_name;
    GString      *def;
    
    gchar        *search_term;          /* Parameters used in current search */
    gchar        *search_db;
    gchar        *search_strat;
    
    GList        *res_list;                               /* List of results */
    GList        *res_tail;          /* Tail of results list for fast access */
    
    gboolean     temp_context;   /* 1 if context should be garbage collected */
    
    struct _dict_command_t *trigger_cmd;
    
    gpointer     user_data;               /* Data to pass along to callbacks */
} dict_command_t;

typedef struct _dict_context_t {
    struct sockaddr_in sockaddr;
    struct hostent *hostinfo;
    char           *hostname;
    
    GIOChannel     *channel;
    guint           source_id;
    char            read_buf[READ_BUF_LEN + 1];
    char           *read_ptr;
    char           *write_ptr;
    gboolean        read_cycle_done;
    
    dict_command_t *command;
} dict_context_t;

dict_res_t *dict_res_new (gchar *name, gchar *desc);
void dict_res_destroy (dict_res_t *res);

dict_context_t *dict_context_new (gchar *server, gushort port);
dict_context_t *dict_context_clone (dict_context_t *context);
void dict_context_destroy (dict_context_t *context);

int dict_connect (dict_context_t *context);
void dict_disconnect (dict_context_t *context);

int dict_match (dict_context_t *context, char *db, char *strategy, char *word);
int dict_define (dict_context_t *context, char *db, char *word);
int dict_get_db (dict_context_t *context);
int dict_get_strat (dict_context_t *context);

dict_command_t *dict_command_new (void);
dict_command_t *dict_connect_command_new (void);
dict_command_t *dict_disconnect_command_new (void);
dict_command_t *dict_match_command_new (char *db, char *strategy, char *word);
dict_command_t *dict_define_command_new (char *db, char *word);
dict_command_t *dict_show_db_command_new (void);
dict_command_t *dict_show_strat_command_new (void);

void dict_command_destroy (dict_command_t *command);
int dict_command_invoke (dict_command_t *command, dict_context_t *context);

#endif /* __DICT_H_ */
