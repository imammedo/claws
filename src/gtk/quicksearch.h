/*
 * Claws Mail -- a GTK+ based, lightweight, and fast e-mail client
 * Copyright (C) 1999-2007 Colin Leroy <colin@colino.net> 
 * and the Claws Mail team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef QUICKSEARCH_H
#define QUICKSEARCH_H 1

typedef enum
{
	QUICK_SEARCH_SUBJECT,
	QUICK_SEARCH_FROM,
	QUICK_SEARCH_TO,
	QUICK_SEARCH_EXTENDED,
	QUICK_SEARCH_MIXED
} QuickSearchType;

typedef struct _QuickSearch QuickSearch;
typedef void (*QuickSearchExecuteCallback) (QuickSearch *quicksearch, gpointer data);

#include "procmsg.h"

QuickSearch *quicksearch_new();
GtkWidget *quicksearch_get_widget(QuickSearch *quicksearch);
void quicksearch_show(QuickSearch *quicksearch);
void quicksearch_hide(QuickSearch *quicksearch);
void quicksearch_set(QuickSearch *quicksearch, QuickSearchType type, const gchar *matchstring);
gboolean quicksearch_is_active(QuickSearch *quicksearch);
void quicksearch_set_execute_callback(QuickSearch *quicksearch,
				      QuickSearchExecuteCallback callback,
				      gpointer data);
gboolean quicksearch_match(QuickSearch *quicksearch, MsgInfo *msginfo);
gboolean quicksearch_is_running(QuickSearch *quicksearch);
gboolean quicksearch_has_focus(QuickSearch *quicksearch);
void quicksearch_pass_key(QuickSearch *quicksearch, guint val, GdkModifierType mod);
void quicksearch_reset_cur_folder_item(QuickSearch *quicksearch);
void quicksearch_search_subfolders(QuickSearch *quicksearch, 
				   FolderView  *folderview,
				   FolderItem  *folder_item);
gboolean quicksearch_is_fast(QuickSearch *quicksearch);
gboolean quicksearch_is_in_typing(QuickSearch *quicksearch);
void quicksearch_relayout(QuickSearch *quicksearch);
#endif /* QUICKSEARCH_H */
