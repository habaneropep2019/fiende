/* Cddbsubmit - a helper program for submitting entries to CDDB servers
 * Copyright (C) 1999  James Henstridge
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <libgnome/libgnome.h>
#ifdef WITH_LIBGHTTP
#  include <ghttp.h>
#endif

char *email = NULL, *filename = NULL;

/* the gnome_config prefix should be set so that lookups will get info about
 * the specified submit method */
int submitByMail(char *category, char *ID) {
  FILE *pipe, *fd;
  char *to;
  char buf[1024];

  fflush(stdout);
  pipe = popen(MAIL_TRANSFER_AGENT, "w");
  to = gnome_config_get_string("email_address");
  if (!to) {
    g_printerr("cddbsubmit: error in service description - no email address\n");
    return -1;
  }
  fprintf(pipe, "To: %s\n", to);
  g_free(to);
  fprintf(pipe, "Subject: cddb %s %s\n", category, ID);
  fprintf(pipe, "From: %s\n\n", email);

  fd = fopen(filename, "r");
  while (fgets(buf, sizeof(buf), fd) != NULL)
    fputs(buf, pipe);
  fclose(fd);
  return pclose(pipe);
}

#ifdef WITH_LIBGHTTP
int submitByHTTP(char *category, char *ID) {
  ghttp_request *request;
  char *url, *proxy, buf[1024];
  gchar *proxy_auth_name = NULL, *proxy_auth_passwd = NULL;
  GString *data;
  FILE *fd;
  
  url = gnome_config_get_string("url");
  if (!url) {
    g_printerr("cddbsubmit: error in service description - no url\n");
    return -1;
  }
  request = ghttp_request_new();
  if (!request)
    return -1;
  if (gnome_config_get_bool("/cddbslave/server/use_http"))
    proxy = gnome_config_get_string("/cddbslave/server/http_proxy");
  else
    proxy = NULL;
  if (proxy && proxy[0] == '\0') g_free(proxy);
  if (proxy && ghttp_set_proxy(request, proxy) != 0) {
    ghttp_request_destroy(request);
    g_printerr("cddbsubmit: could not set proxy for HTTP request\n");
    g_free(proxy);
    return -1;
  }
  if (proxy) g_free(proxy);

  if (gnome_config_get_bool("/cddbslave/server/need_http_proxy_auth=false")) {
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
    if (ghttp_set_proxy_authinfo(request, proxy_auth_name, proxy_auth_passwd) != 0) {
      ghttp_request_destroy(request);
      g_printerr("cddbsubmit: could not set proxy authorization for HTTP request\n");
      g_free(proxy_auth_name);
      g_free(proxy_auth_passwd);
      return -1;
    }
    g_free(proxy_auth_name);
    g_free(proxy_auth_passwd);
  }

  if (ghttp_set_uri(request, url) != 0) {
    ghttp_request_destroy(request);
    g_printerr("cddbsubmit: could not set uri for HTTP request\n");
    g_free(url);
    return -1;
  }
  g_free(url);
  if (ghttp_set_type(request, ghttp_type_post) != 0) {
    ghttp_request_destroy(request);
    g_printerr("cddbsubmit: could not set HTTP request type to POST\n");
    return -1;
  }

  /* set the headers for the request */
  ghttp_set_header(request, http_hdr_Connection, "close");
  ghttp_set_header(request, "Category", category);
  ghttp_set_header(request, "Discid", ID);
  ghttp_set_header(request, "User-Email", email);
  if (gnome_config_get_bool("testing=false"))
    ghttp_set_header(request, "Submit-Mode", "test");
  else
    ghttp_set_header(request, "Submit-Mode", "submit");
  ghttp_set_header(request, "X-Cddbd-Note", "Problems with submission?  Tell me (James Henstridge)");

  /* set the body for the post request */
  data = g_string_sized_new(2048);
  fd = fopen(filename, "r");
  while (fgets(buf, sizeof(buf), fd) != NULL)
    g_string_append(data, buf);
  fclose(fd);
  if (ghttp_set_body(request, data->str, data->len) != 0) {
    g_printerr("cddbsubmit: couldn't set the body for the post request\n");
    ghttp_request_destroy(request);
    g_string_free(data, TRUE);
    return -1;
  }

  if (ghttp_prepare(request) != 0) {
    ghttp_request_destroy(request);
    g_printerr("cddbsubmit: an error occured preparing the HTTP request\n");
    g_string_free(data, TRUE);
    return -1;
  }
  if (ghttp_process(request) == ghttp_error) {
    g_printerr("cddbsubmit: HTTP processing error: %s\n",
	       ghttp_get_error(request));
    ghttp_request_destroy(request);
    g_string_free(data, TRUE);
    return -1;
  }
  if (ghttp_status_code(request) != 200) {
    g_printerr("cddbsubmit: the server returned the error code %d (%s)\n",
	       ghttp_status_code(request), ghttp_reason_phrase(request));
    g_print(ghttp_get_body(request));
    ghttp_request_destroy(request);
    g_string_free(data, TRUE);
    return -1;
  }
  ghttp_request_destroy(request);
  g_string_free(data, TRUE);
  return 0;
}
#endif

static char *service = "freedb", *category = NULL, *ID = NULL;

static void parseAnArg(poptContext ctx,
		       enum poptCallbackReason reason,
		       const struct poptOption *opt,
		       const char *arg, void *data) {
  if (reason != POPT_CALLBACK_REASON_POST)
    return;
  category = poptGetArg(ctx);
  if (category == NULL) {
    g_printerr("cddbsubmit: must give a music category name\n");
    poptPrintUsage(ctx, stderr, 0);
    exit(1);
  }
  ID = poptGetArg(ctx);
  if (ID == NULL) {
    g_printerr("cddbsubmit: must give a disc ID\n");
    poptPrintUsage(ctx, stderr, 0);
    exit(1);
  }
}

static struct poptOption options[] = {
  { NULL, '\0', POPT_ARG_CALLBACK|POPT_CBFLAG_POST,
    parseAnArg, 0, NULL, NULL },
  { "service", 's', POPT_ARG_STRING,
    &service, 0, "The service to submit to", "SERVICE" },
  { NULL, '\0', 0, NULL, 0}
};

int main(int argc, char *argv[]) {
  char *fname, *prefix;
  char *service_type;
  char *user, host[512];

  gnomelib_init("cddbsubmit", VERSION);
  gnomelib_register_popt_table(options, "cddbslave options");
  poptFreeContext(gnomelib_parse_args(argc, argv, 0));

  /* config prefix points to cddb submission description file */
  fname = gnome_datadir_file("gnome/cddb-submit-methods");
  prefix = g_strconcat("=", fname, "=/", service, "/", NULL);
  g_free(fname);
  gnome_config_push_prefix(prefix);
  g_free(prefix);

  if ((email = gnome_config_get_string("/cddbslave/personal/email_address"))
      == NULL) {
    user = g_get_user_name();
    gethostname(host, sizeof(host));
    if (host[0] == '\0')
      strcpy(host, "localhost");
    email = g_strconcat(user, "@", host, NULL);
    gnome_config_set_string("/cddbslave/personal/email_address", email);
    gnome_config_sync();
  }
  filename = g_strconcat(g_get_home_dir(), "/.cddbslave/", ID, NULL);

  if (!g_file_exists(filename)) {
    g_printerr("cddbslave: could not find local track listing for ID %s\n",ID);
    exit(1);
  }

  service_type = gnome_config_get_string("type");
  if (service_type == NULL) {
    g_printerr("cddbsubmit: unknown cddb service '%s'\n", service);
    exit(1);
  }
  if (!strcmp(service_type, "mail"))
    return (submitByMail(category, ID) != 0);
#if WITH_LIBGHTTP
  else if (!strcmp(service_type, "http"))
    return (submitByHTTP(category, ID) != 0);
#endif
  else {
    g_printerr("cddbsubmit: unknown service type '%s' for service '%s'\n",
	       service_type, service);
    return 1;
  }
}
