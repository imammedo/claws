#include <gdk/gdkquartz.h>
#include "gtkosxapplication.h"
#include "cm_osx.h"

void claws_mail_osx_init(GtkWidget *window)
{
	GtkosxApplication *theApp = g_object_new(GTKOSX_TYPE_APPLICATION, NULL);
	NSWindow *native = gdk_quartz_window_get_nswindow (GTK_WIDGET (window)->window);

	[native setCollectionBehavior: NSWindowCollectionBehaviorFullScreenPrimary];

	gtkosx_application_ready(theApp);
}

void claws_mail_osx_set_menu(GtkWidget *menu)
{
	GtkosxApplication *theApp = g_object_new(GTKOSX_TYPE_APPLICATION, NULL);

	gtk_widget_hide (menu);
	gtkosx_application_set_menu_bar(theApp, GTK_MENU_SHELL(menu));
}

void claws_mail_osx_set_empty_menu(GtkWidget *window)
{
	GtkWidget *empty_menu = gtk_menu_bar_new();
	gtk_widget_set_parent(empty_menu, window);

	claws_mail_osx_set_menu(empty_menu);
}
