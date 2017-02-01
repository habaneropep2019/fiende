/*
 * Use this program to test your implementation of write_[login|logout]_record
 */
#include <unistd.h>
#include "gnome-pty.h"

int
main ()
    {
    void *utmp;
    /* Assume, that ttyr0 exists, and, nobody - too */
    utmp = write_login_record ("nobody", ":0", "/dev/ttyr0", 1, 1, 1);
    sleep (120);
    write_logout_record (utmp, 1, 1);
    return 0;
    }
