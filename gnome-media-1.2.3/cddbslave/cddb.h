#ifndef CDDB_SLAVE_CDDB_H
#define CDDB_SLAVE_CDDB_H

#include <config.h>
#include <gnome.h>

void set_status(int status, gchar *info);

#define HOSTNAME_MAX    256
#define VERSION_MAX     10
#define CATEG_MAX       20      
#define DISCID_MAX      10
#define DTITLE_MAX      80

#define STATUS_READY         0
#define STATUS_CONNECTING    1
#define STATUS_QUERYING      2
#define STATUS_WAITING       3
#define STATUS_READING       4
#define STATUS_DISCONNECTING 5

#define ERR_CONNECTING       6
#define ERR_QUERYING         7
#define ERR_READING          8
#define ERR_NOMATCH          9
#define ERR_TIMEOUT          10

#define STATUS_NONE          11

#endif
