#include <config.h>
#include <gnome.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <dirent.h>
#include <signal.h>
#include <errno.h>
#include <pwd.h>

#include "socket.h"
#include "cddb.h"

#ifdef WITH_LIBGHTTP
#  include <ghttp.h>
int do_httprequest(char *req);
#endif

void read_query(char *s, int fd, int len);
int check_response(char *buf);
void disconnect(int fd);
static char *if_strtok(char *p, const char *p2);
char *get_file_name(char *discid);
void getusername_safe(char *buf, int len);
void gethostname_safe(char *buf, int len);
void die(int signal);
int do_request(char *req, int fd);
void my_exit(int errorcode);

int check_cache(char *req);
int create_cache(char *req);
int remove_cache(char *req);
char *cache_name(char *req);

char hostname[512], username[512];
char client_ver[64], client_name[64];
char *g_req;
int pid;

gchar *server;
gint port;

#ifdef WITH_LIBGHTTP
gboolean use_http;
gchar *proxy = NULL;
gboolean need_proxy_auth;
gchar *proxy_auth_name = NULL;
gchar *proxy_auth_passwd = NULL;
#endif

const gchar *status_msg[] = {
    N_("Ready for command: %s\n"),
    N_("Connecting [%s]\n"),
    N_("Querying server [%s]\n"),
    N_("Waiting for reply [%s]\n"),
    N_("Reading reply [%s]\n"),
    N_("Disconnecting [%s]\n"),
    N_("Error connecting [%s]\n"),
    N_("Error querying [%s]\n"),
    N_("Error reading [%s]\n"),
    N_("No match found for %s\n"),
    N_("Timeout: %s\n"),
    N_("Inactive. %s\n"),
};

void my_exit(int errorcode)
{
    remove(gnome_util_home_file(".cddbstatus_lock"));
    exit(errorcode);
}

int main(int argc, char *argv[])
{
    char req[512], client[512];
    int cddev;
    int i;
    FILE *tf;

    tf = fopen(gnome_util_home_file(".cddbstatus_lock"), "w");
    fprintf(tf, "%p\n", tf);
    if(tf)
	fclose(tf);


    fgets(req, 511, stdin);	/* grab the cddb request */
    fgets(client, 511, stdin);	/* and then the client info */
    i = sscanf(client, "client %s %s %d %d\n", client_name, client_ver, &pid, &cddev);
    gnomelib_init("cddbslave", VERSION);
    server = gnome_config_get_string("/cddbslave/server/address=freedb.freedb.org");
    port = gnome_config_get_int("/cddbslave/server/port=888");
#ifdef WITH_LIBGHTTP
    use_http = gnome_config_get_bool("/cddbslave/server/use_http=false");
    proxy = gnome_config_get_string("/cddbslave/server/http_proxy=");
    if (proxy && proxy[0] == '\0') {
      g_free(proxy);
      proxy = NULL;
    }
    need_proxy_auth = gnome_config_get_bool("/cddbslave/server/need_http_proxy_auth=false");
    proxy_auth_name = gnome_config_private_get_string("/cddbslave/server/http_proxy_auth_name=");
    if (proxy_auth_name && proxy_auth_name[0] == '\0') {
      g_free(proxy_auth_name);
      proxy_auth_name = NULL;
    }
    proxy_auth_passwd = gnome_config_private_get_string("/cddbslave/server/http_proxy_auth_passwd=");
    if (proxy_auth_passwd && proxy_auth_passwd[0] == '\0') {
      g_free(proxy_auth_passwd);
      proxy_auth_passwd = NULL;
    }
#endif

    gethostname_safe(hostname, 512);
    getusername_safe(username, 512);
    close(cddev);

    if(check_cache(req))
	my_exit(0);
    create_cache(req);

    if(fork()==0)
    {
	int fd;
	char *dname;
	DIR *d;
	struct dirent *de;

#ifdef WITH_LIBGHTTP
	if (!use_http) {
#endif
	/* connect to cddb server */
	g_req = req;
	set_status(STATUS_CONNECTING, server);
	fd = opensocket(server, port);
	if(fd < 0)
	{
	    set_status(ERR_CONNECTING, server);
	    sleep(5);
	    set_status(STATUS_NONE, "");
	    remove_cache(req);
	    return(-1);
	}
#ifdef WITH_LIBGHTTP
	}
#endif
	set_status(STATUS_QUERYING, server);

	/* grab directory name */
	dname = get_file_name("");
	if(!dname)
	{
#ifdef WITH_LIBGHTTP
	  if (!use_http) {
#endif
	    if(do_request(req, fd) < 0)
		sleep(5);
	    set_status(STATUS_NONE, "");
	    remove_cache(req);
	    disconnect(fd);
#ifdef WITH_LIBGHTTP
	  } else {
	    if (do_httprequest(req) < 0)
	      sleep(5);
	    set_status(STATUS_NONE, "");
	    remove_cache(req);
	  }
#endif
	    my_exit(-1);
	}

	/* open directory for scanning */
	d = opendir(dname);
	if(!d)
	{
#ifdef WITH_LIBGHTTP
	  if (!use_http) {
#endif
	    if(do_request(req, fd) < 0)
		sleep(5);
	    set_status(STATUS_NONE, "");
	    remove_cache(req);
	    disconnect(fd);
#ifdef WITH_LIBGHTTP
	  } else {
	    if (do_httprequest(req) < 0)
	      sleep(5);
	    set_status(STATUS_NONE, "");
	    remove_cache(req);
	  }
#endif
	    my_exit(-1);
	}

	/* scan for cached queries */
	while((de=readdir(d)))
	{
	    if(strstr(de->d_name, ".query"))
	    {
		FILE *fp;
		char *fname;
		char buf[512];
		
		fname = get_file_name(de->d_name);

		fp = fopen(fname, "r");
		if(!fp)
		    break;
		
		fgets(buf, 512, fp);
		fclose(fp);
#ifdef WITH_LIBGHTTP
		if (use_http) {
		  if (do_httprequest(buf) < 0)
		    sleep(5);
		} else
#endif
		if(do_request(buf, fd) < 0)
		    sleep(5);	/* sleep for a bit so client can get error code */
		remove_cache(buf);
		g_free(fname);
	    }
	}
	/* clean up */
	set_status(STATUS_NONE, "");
	g_free(dname);
#ifdef WITH_LIBGHTTP
	if (!use_http)
#endif
	  disconnect(fd);
	my_exit(EXIT_SUCCESS);
    }
    exit(EXIT_SUCCESS);
}

#ifdef WITH_LIBGHTTP
char *uri_encode(char *string) {
  GString *str = g_string_sized_new(strlen(string));
  gchar *ret;

  while (*string) {
    if (*string == ' ')
      g_string_append_c(str, '+');
    else if ((*string >= 'A' && *string <= 'Z') ||
	     (*string >= 'a' && *string <= 'z') ||
	     (*string >= '0' && *string <= '9') ||
	     *string == '-' || *string == '_' || *string == '.')
      g_string_append_c(str, *string);
    else {
      gint8 a = *string >> 4, b = *string & 0xf;
      gchar hex[] = "%xx";

      if (a < 10) hex[1] = a + '0';
      else hex[1] = a - 10 + 'a';
      if (b < 10) hex[2] = b + '0';
      else hex[2] = b - 10 + 'b';
      g_string_append(str, hex);
    }
    string++;
  }
  ret = str->str;
  g_string_free(str, FALSE);
  return ret;
}

gchar *assemble_url(char *req) {
  gchar *enc_req = uri_encode(req);
  gchar *enc_username = uri_encode(username);
  gchar *enc_hostname = uri_encode(hostname);
  gchar *enc_clientname = uri_encode(client_name);
  gchar *enc_clientver = uri_encode(client_ver);
  /* we use protocol version 3 because I haven't written code to handle
   * multiple exact matches.  This means we don't have to deal with it */
  gchar *url = g_strdup_printf(
	"http://%s:%d/~cddb/cddb.cgi?cmd=%s&hello=%s+%s+%s+%s&proto=3",
	server, port, enc_req, enc_username, enc_hostname,
	enc_clientname, enc_clientver);
  g_free(enc_req);
  g_free(enc_username);
  g_free(enc_hostname);
  g_free(enc_clientname);
  g_free(enc_clientver);
  return url;
}

int do_httprequest(char *req)
{
  ghttp_request *request;
  gchar *url, *s;
  gchar buf[512];
  gchar categ[CATEG_MAX];
  gchar discid[DISCID_MAX];
  gchar dtitle[DTITLE_MAX];
  gchar *fname;
  gint code, len;
  FILE *fp;

  request = ghttp_request_new();
  if (!request)
    return -1;
  if (proxy && ghttp_set_proxy(request, proxy) != 0) {
    ghttp_request_destroy(request);
    set_status(ERR_CONNECTING, server);
    return -1;
  }
  if (need_proxy_auth && ghttp_set_proxy_authinfo(request, proxy_auth_name, proxy_auth_passwd) != 0) {
    ghttp_request_destroy(request);
    set_status(ERR_CONNECTING, server);
    return -1;
  }
  url = assemble_url(req);
  if (ghttp_set_uri(request, url) != 0) {
    ghttp_request_destroy(request);
    g_free(url);
    set_status(ERR_CONNECTING, server);
    return -1;
  }
  g_free(url);
  ghttp_set_header(request, http_hdr_Connection, "close");
  if (ghttp_prepare(request) != 0) {
    ghttp_request_destroy(request);
    set_status(ERR_CONNECTING, server);
    return -1;
  }
  set_status(STATUS_READING, server);
  if (ghttp_process(request) == ghttp_error) {
    g_message("processing error: %s", ghttp_get_error(request));
    set_status(ERR_QUERYING, server);
    ghttp_request_destroy(request);
    return -1;
  }
  if ((code = ghttp_status_code(request)) != 200) {
    g_message("failed due to bad return code %d", code);
    set_status(ERR_QUERYING, "disc");
    ghttp_request_destroy(request);
    return -code;
  }

  s = ghttp_get_body(request);
  len = ghttp_get_body_len(request);
  
  code = atoi(strtok(s, " " ));         
  strncpy(categ, if_strtok(NULL, " ") , CATEG_MAX);
  categ[CATEG_MAX-1]=0;
  strncpy(discid, if_strtok( NULL, " "), DISCID_MAX);
  discid[DISCID_MAX-1]=0;
  strncpy(dtitle, if_strtok( NULL, "\0"), DTITLE_MAX);
  dtitle[DTITLE_MAX-1]=0;

  if (code != 200) {
    /* not found */
    set_status(ERR_NOMATCH, "disc");
    ghttp_request_destroy(request);
    return -code;
  }

  ghttp_request_destroy(request);

  request = ghttp_request_new();
  if (!request)
    return -1;
  if (proxy && ghttp_set_proxy(request, proxy) != 0) {
    ghttp_request_destroy(request);
    set_status(ERR_CONNECTING, server);
    return -1;
  }
  if (need_proxy_auth && ghttp_set_proxy_authinfo(request, proxy_auth_name, proxy_auth_passwd) != 0) {
    ghttp_request_destroy(request);
    set_status(ERR_CONNECTING, server);
    return -1;
  }

  g_snprintf(buf, 511, "cddb read %s %s\n", categ, discid);
  url = assemble_url(buf);
  
  if (ghttp_set_uri(request, url) != 0) {
    g_free(url);
    set_status(ERR_CONNECTING, server);
    ghttp_request_destroy(request);
    return;
  }
  g_free(url);
  ghttp_set_header(request, http_hdr_Connection, "close");
  if (ghttp_prepare(request) != 0) {
    set_status(ERR_CONNECTING, server);
    ghttp_request_destroy(request);
    return -1;
  }
  set_status(STATUS_READING, server);
  if (ghttp_process(request) == ghttp_error) {
    g_print("processing error: %s", ghttp_get_error(request));
    set_status(ERR_READING, server);
    ghttp_request_destroy(request);
    return -1;
  }
  if ((code = ghttp_status_code(request)) != 200) {
    set_status(ERR_READING, server);
    ghttp_request_destroy(request);
    return -code;
  }
  s = ghttp_get_body(request);
  if ((code = check_response(s)) != 210) {
    set_status(ERR_NOMATCH, "disc");
    ghttp_request_destroy(request);
    return -code;
  }
  /* skip first line ... */
  while (*s != '\r' && *s != '\n' && *s != '\0') s++;
  while (*s == '\r' || *s == '\n') s++;

  fname = get_file_name(discid);
  if (!fname) {
    ghttp_request_destroy(request);
    return -1;
  }

  fp = fopen(fname, "w");
  g_free(fname);

  if (fp == NULL) {
    set_status(ERR_READING, "disc");
    ghttp_request_destroy(request);
    return -1;
  }
  /* write body to file, skip \r's, and stop before the final dot */
  while (*s) {
    if (*s != '\r') {
      fprintf(fp, "%c", *s);
      if (s[0] == '\n' && s[1] == '.') break;
    }
    s++;
  }
  fclose(fp);

  kill(pid, SIGUSR2);
  ghttp_request_destroy(request);
  set_status(STATUS_NONE, "");
}
#endif

int do_request(char *req, int fd)
{
    int i;
    char buf[512];

    g_req = req;		/* make the request available to die() */
    
    /* receive login banner */
    fgetsock(buf, 511, fd);
    
    if((i=check_response(buf)) > 400)
	return -i;

    /* send handshake */
    g_snprintf(buf, 511, "cddb hello %s %s %s %s\n", 
	       username, hostname, 
	       client_name, client_ver);
    send(fd, buf, strlen(buf), 0);
    
    /* receive handshake result */
    fgetsock(buf, 511, fd);
    
    if((i=check_response(buf)) != 200)
    {
	set_status(ERR_QUERYING, server);
	return -i;
    }
    /* we're connected and identified. */
    /* now we send our query */
    g_snprintf(buf, 511, "%s\n", req);
    send(fd, buf, strlen(buf), 0);
    set_status(STATUS_READING, server);
    /* receive query result */
    fgetsock(buf, 511, fd);
    
    if((i=check_response(buf)) != 200)
    {
	set_status(ERR_NOMATCH, "disc");
	return -i;
    }
    /* alright, our query response was positive, now send the read request. */
    read_query(buf, fd, 512);
    set_status(STATUS_NONE, "");
    return 0;
}

int check_response(char *buf)
{
    int code;

    if(sscanf(buf, "%d", &code) != 1)
	return -1;
    
    return code;
}

void disconnect(int fd)
{
    char buf[512];

    /* send quit message */
    g_snprintf(buf, 511, "quit\n");
    send(fd, buf, strlen(buf), 0);
    
    /* receive quit result */
    fgetsock(buf, 511, fd);

    close(fd);
    return;
}

static char *if_strtok(char *p, const char *p2)
{
        p=strtok(p,p2);
        if(p==NULL)
                return("????");
        return p;
}

void read_query(char *s, int fd, int len)
{
    char buf[512];
    char categ[CATEG_MAX];
    char discid[DISCID_MAX];
    char dtitle[DTITLE_MAX];
    char *fname;
    int code;
    FILE *fp;

    s[len-1] = 0;
 
    code = atoi(strtok(s, " " ));         
    strncpy(categ, if_strtok(NULL, " ") , CATEG_MAX);
    categ[CATEG_MAX-1]=0;
    strncpy(discid, if_strtok( NULL, " "), DISCID_MAX);
    discid[DISCID_MAX-1]=0;
    strncpy(dtitle, if_strtok( NULL, "\0"), DTITLE_MAX);
    dtitle[DTITLE_MAX-1]=0;
    
    g_snprintf(buf, 511, "cddb read %s %s\n", categ, discid);
    send(fd, buf, strlen(buf), 0);
    
    fgetsock(buf, 511, fd);
    
    if(check_response(buf) != 210)
	return;
    
    fname = get_file_name(discid);
    if(fname == NULL)
	return;

    fp = fopen(fname, "w");

    if(fp == NULL)
	return;			/* blah, can't open file */

    do
    {
	fgetsock(buf, 511, fd);
	fprintf(fp, "%s", buf);
    } while(strncmp(".", buf, 1));
   
    fclose(fp);
    g_free(fname);

    kill(pid, SIGUSR2);
    return;
}

char *get_file_name(char *discid)
{
    char *fname;
    char *homedir=NULL;
    struct stat fs;

    homedir = g_get_home_dir();
    fname = g_malloc(512);

    /* Make our directory if it doesn't exist. */
    g_snprintf(fname, 511, "%s/.cddbslave", homedir);
    if(stat(fname, &fs) < 0)
    {
	if(errno == ENOENT)
	{
	    if(mkdir(fname, S_IRWXU))
	    {
		return NULL;
	    }
	}
	else
	    return NULL;
    }

    g_snprintf(fname, 511, "%s/.cddbslave/%s", homedir, discid);
    return fname;
}

void gethostname_safe(char *buf, int len)
{
    gethostname(buf, 512);
    if(!buf || (strcmp(buf, "")==0))
	strncpy(buf, "generic", len);
}

void getusername_safe(char *buf, int len)
{
    strncpy(buf, g_get_user_name(), len);
}

char *cache_name(char *req)
{
    char discid[64];
    char *fname;
    
    if(sscanf(req, "cddb query %[0-9a-fA-F]", discid) != 1)
	return FALSE;
    
    strncat(discid, ".query", 63);
    
    fname = get_file_name(discid);
    return fname;
}
   
/* Deal with cached requests. */
int check_cache(char *req)
{
    char *fname;
    FILE *fp;

    fname = cache_name(req);
    if(!fname)
	return FALSE;

    fp = fopen(fname, "rw");
    if(!fp)
	return FALSE;		/* query already has been submitted */
    
    fclose(fp);
    g_free(fname);

    return TRUE;
}

/* Create `lock' file. */
int create_cache(char *req)
{
    char *fname;
    FILE *fp;

    fname = cache_name(req);
    if(!fname)
	return FALSE;
    
    fp = fopen(fname, "w");
    if(!fp)
	return FALSE;		/* can't open file */

    fputs(req, fp);

    fclose(fp);
    g_free(fname);

    return TRUE;
}

/* Remove fulfulled queries */
int remove_cache(char *req)
{
    char *fname;
    
    fname = cache_name(req);
    if(!fname)
	return FALSE;
    
    if(remove(fname))
	return FALSE;		/* can't remove file */

    g_free(fname);

    return TRUE;
}

void set_status(int status, gchar *info)
{
    FILE *fp;
    static char *status_file;

    if (!status_file){
	    status_file = gnome_util_home_file (".cddbstatus");
    }
	    
    if(status == STATUS_NONE)
	truncate(status_file, 0);
    fp = fopen(status_file, "a+");
    if(!fp)
    {
	perror("fopen");
	return;
    }
    fprintf(fp, "%03d ", status);
    fprintf(fp, status_msg[status], info);
    fclose(fp);
}
