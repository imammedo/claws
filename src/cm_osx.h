#ifndef CM_OSX_H

#include <gtk/gtk.h>

#ifdef MAC_INTEGRATION
void claws_mail_osx_init(GtkWidget *window);
void claws_mail_osx_set_menu(GtkWidget *menu);
#else
static inline void claws_mail_osx_init(GtkWidget *window) {}
static inline void claws_mail_osx_set_menu(GtkWidget *menu) {}
#endif

#endif
