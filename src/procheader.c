/*
 * Sylpheed -- a GTK+ based, lightweight, and fast e-mail client
 * Copyright (C) 1999-2001 Hiroyuki Yamamoto
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "intl.h"
#include "procheader.h"
#include "procmsg.h"
#include "codeconv.h"
#include "prefs_common.h"
#include "utils.h"

#define BUFFSIZE	8192

/*
  procheader_get_one_field
  - reads fp and puts the header and the corresponding content into buf
    if one of these is one of hentry table.
  - if hentry is NULL, ignores no headers
 */

gint procheader_get_one_field(gchar *buf, gint len, FILE *fp,
			      HeaderEntry hentry[])
{
	gint nexthead;
	gint hnum = 0;
	HeaderEntry *hp = NULL;

	if (hentry != NULL) {
		/* skip non-required headers */
		do {
			do {
				if (fgets(buf, len, fp) == NULL)
					return -1;
				if (buf[0] == '\r' || buf[0] == '\n')
					return -1;
			} while (buf[0] == ' ' || buf[0] == '\t');

			for (hp = hentry, hnum = 0; hp->name != NULL;
			     hp++, hnum++) {
				if (!strncasecmp(hp->name, buf,
						 strlen(hp->name)))
					break;
			}
		} while (hp->name == NULL);
	} else {
		if (fgets(buf, len, fp) == NULL) return -1;
		if (buf[0] == '\r' || buf[0] == '\n') return -1;
	}

	/* unfold the specified folded line */
	if (hp && hp->unfold) {
		gboolean folded = FALSE;
		gchar *bufp = buf + strlen(buf);

		while (1) {
			nexthead = fgetc(fp);

			/* folded */
			if (nexthead == ' ' || nexthead == '\t')
				folded = TRUE;
			else if (nexthead == EOF)
				break;
			else if (folded == TRUE) {
				/* concatenate next line */
				if ((len - (bufp - buf)) <= 2) break;

				/* replace return code on the tail end
				   with space */
				*(bufp - 1) = ' ';
				*bufp++ = nexthead;
				*bufp = '\0';
				if (nexthead == '\r' || nexthead == '\n') {
					folded = FALSE;
					continue;
				}
				if (fgets(bufp, len - (bufp - buf), fp)
				    == NULL) break;
				bufp += strlen(bufp);

				folded = FALSE;
			} else {
				ungetc(nexthead, fp);
				break;
			}
		}

		/* remove trailing return code */
		strretchomp(buf);

		return hnum;
	}

	while (1) {
		nexthead = fgetc(fp);
		if (nexthead == ' ' || nexthead == '\t') {
			size_t buflen = strlen(buf);

			/* concatenate next line */
			if ((len - buflen) > 2) {
				gchar *p = buf + buflen;

				*p++ = nexthead;
				*p = '\0';
				buflen++;
				if (fgets(p, len - buflen, fp) == NULL)
					break;
			} else
				break;
		} else {
			if (nexthead != EOF)
				ungetc(nexthead, fp);
			break;
		}
	}

	/* remove trailing return code */
	strretchomp(buf);

	return hnum;
}

gchar *procheader_get_unfolded_line(gchar *buf, gint len, FILE *fp)
{
	gboolean folded = FALSE;
	gint nexthead;
	gchar *bufp;

	if (fgets(buf, len, fp) == NULL) return NULL;
	if (buf[0] == '\r' || buf[0] == '\n') return NULL;
	bufp = buf + strlen(buf);

	while (1) {
		nexthead = fgetc(fp);

		/* folded */
		if (nexthead == ' ' || nexthead == '\t')
			folded = TRUE;
		else if (nexthead == EOF)
			break;
		else if (folded == TRUE) {
			/* concatenate next line */
			if ((len - (bufp - buf)) <= 2) break;

			/* replace return code on the tail end
			   with space */
			*(bufp - 1) = ' ';
			*bufp++ = nexthead;
			*bufp = '\0';
			if (nexthead == '\r' || nexthead == '\n') {
				folded = FALSE;
				continue;
			}
			if (fgets(bufp, len - (bufp - buf), fp)
			    == NULL) break;
			bufp += strlen(bufp);

			folded = FALSE;
		} else {
			ungetc(nexthead, fp);
			break;
		}
	}

	/* remove trailing return code */
	strretchomp(buf);

	return buf;
}

GSList *procheader_get_header_list(const gchar *file)
{
	FILE *fp;
	gchar buf[BUFFSIZE], tmp[BUFFSIZE];
	gchar *p;
	GSList *hlist = NULL;
	Header *header;

	if ((fp = fopen(file, "r")) == NULL) {
		FILE_OP_ERROR(file, "fopen");
		return NULL;
	}

	while (procheader_get_unfolded_line(buf, sizeof(buf), fp) != NULL) {
		if (*buf == ':') continue;
		for (p = buf; *p && *p != ' '; p++) {
			if (*p == ':') {
				header = g_new(Header, 1);
				header->name = g_strndup(buf, p - buf);
				p++;
				while (*p == ' ' || *p == '\t') p++;
				conv_unmime_header(tmp, sizeof(tmp), p, NULL);
				header->body = g_strdup(tmp);

				hlist = g_slist_append(hlist, header);
				break;
			}
		}
	}

	fclose(fp);
	return hlist;
}

void procheader_header_list_destroy(GSList *hlist)
{
	Header *header;

	while (hlist != NULL) {
		header = hlist->data;

		g_free(header->name);
		g_free(header->body);
		g_free(header);
		hlist = g_slist_remove(hlist, header);
	}
}

void procheader_get_header_fields(FILE *fp, HeaderEntry hentry[])
{
	gchar buf[BUFFSIZE];
	HeaderEntry *hp;
	gint hnum;
	gchar *p;

	if (hentry == NULL) return;

	while ((hnum = procheader_get_one_field(buf, sizeof(buf), fp, hentry))
	       != -1) {
		hp = hentry + hnum;

		p = buf + strlen(hp->name);
		while (*p == ' ' || *p == '\t') p++;

		if (hp->body == NULL)
			hp->body = g_strdup(p);
		else if (!strcasecmp(hp->name, "To:") ||
			 !strcasecmp(hp->name, "Cc:")) {
			gchar *tp = hp->body;
			hp->body = g_strconcat(tp, ", ", p, NULL);
			g_free(tp);
		}
	}
}

enum
{
	H_DATE		= 0,
	H_FROM		= 1,
	H_TO		= 2,
	H_NEWSGROUPS	= 3,
	H_SUBJECT	= 4,
	H_MSG_ID	= 5,
	H_REFERENCES	= 6,
	H_IN_REPLY_TO	= 7,
	H_CONTENT_TYPE	= 8,
	H_SEEN		= 9,
	H_X_FACE	= 10,
};

MsgInfo *procheader_parse(const gchar *file, MsgFlags flags, gboolean full)
{
	static HeaderEntry hentry_full[] = {{"Date:",		NULL, FALSE},
					   {"From:",		NULL, TRUE},
					   {"To:",		NULL, TRUE},
					   {"Newsgroups:",	NULL, TRUE},
					   {"Subject:",		NULL, TRUE},
					   {"Message-Id:",	NULL, FALSE},
					   {"References:",	NULL, FALSE},
					   {"In-Reply-To:",	NULL, FALSE},
					   {"Content-Type:",	NULL, FALSE},
					   {"Seen:",		NULL, FALSE},
					   {"X-Face:",		NULL, FALSE},
					   {NULL,		NULL, FALSE}};

	static HeaderEntry hentry_short[] = {{"Date:",		NULL, FALSE},
					    {"From:",		NULL, TRUE},
					    {"To:",		NULL, TRUE},
					    {"Newsgroups:",	NULL, TRUE},
					    {"Subject:",	NULL, TRUE},
					    {"Message-Id:",	NULL, FALSE},
					    {"References:",	NULL, FALSE},
					    {"In-Reply-To:",	NULL, FALSE},
					    {"Content-Type:",	NULL, FALSE},
					    {"Seen:",		NULL, FALSE},
					    {NULL,		NULL, FALSE}};

	FILE *fp;
	MsgInfo *msginfo;
	gchar buf[BUFFSIZE], tmp[BUFFSIZE];
	gchar *reference = NULL;
	gchar *p;
	gchar *hp;
	HeaderEntry *hentry;
	gint hnum;

	hentry = full ? hentry_full : hentry_short;

	if ((fp = fopen(file, "r")) == NULL) {
		FILE_OP_ERROR(file, "fopen");
		return NULL;
	}
	if (MSG_IS_QUEUED(flags)) {
		while (fgets(buf, sizeof(buf), fp) != NULL)
			if (buf[0] == '\r' || buf[0] == '\n') break;
	}

	msginfo = g_new0(MsgInfo, 1);
	msginfo->flags = flags != 0 ? flags : MSG_NEW|MSG_UNREAD;
	msginfo->inreplyto = NULL;

	while ((hnum = procheader_get_one_field(buf, sizeof(buf), fp, hentry))
	       != -1) {
		hp = buf + strlen(hentry[hnum].name);
		while (*hp == ' ' || *hp == '\t') hp++;

		switch (hnum) {
		case H_DATE:
			if (msginfo->date) break;
			msginfo->date_t =
				procheader_date_parse(NULL, hp, 0);
			msginfo->date = g_strdup(hp);
			break;
		case H_FROM:
			if (msginfo->from) break;
			conv_unmime_header(tmp, sizeof(tmp), hp, NULL);
			msginfo->from = g_strdup(tmp);
			msginfo->fromname = procheader_get_fromname(tmp);
			break;
		case H_TO:
			conv_unmime_header(tmp, sizeof(tmp), hp, NULL);
			if (msginfo->to) {
				p = msginfo->to;
				msginfo->to =
					g_strconcat(p, ", ", tmp, NULL);
				g_free(p);
			} else
				msginfo->to = g_strdup(tmp);
			break;
		case H_NEWSGROUPS:
			if (msginfo->newsgroups) {
				p = msginfo->newsgroups;
				msginfo->newsgroups =
					g_strconcat(p, ",", hp, NULL);
				g_free(p);
			} else
				msginfo->newsgroups = g_strdup(buf + 12);
			break;
		case H_SUBJECT:
			if (msginfo->subject) break;
			conv_unmime_header(tmp, sizeof(tmp), hp, NULL);
			msginfo->subject = g_strdup(tmp);
			break;
		case H_MSG_ID:
			if (msginfo->msgid) break;

			extract_parenthesis(hp, '<', '>');
			remove_space(hp);
			msginfo->msgid = g_strdup(hp);
			break;
		case H_REFERENCES:
			if (!reference) {
				eliminate_parenthesis(hp, '(', ')');
				if ((p = strrchr(hp, '<')) != NULL &&
				    strchr(p + 1, '>') != NULL) {
					extract_parenthesis(p, '<', '>');
					remove_space(p);
					if (*p != '\0')
						reference = g_strdup(p);
				}
			}
			break;
		case H_IN_REPLY_TO:
			if (!reference) {
				eliminate_parenthesis(hp, '(', ')');
				extract_parenthesis(hp, '<', '>');
				remove_space(hp);
				if (*hp != '\0')
					reference = g_strdup(hp);
			}
			break;
		case H_CONTENT_TYPE:
			if (!strncasecmp(hp, "multipart", 9))
				msginfo->flags |= MSG_MIME;
			break;
		case H_SEEN:
			/* mnews Seen header */
			MSG_UNSET_FLAGS(msginfo->flags, MSG_NEW|MSG_UNREAD);
			break;
		case H_X_FACE:
			if (msginfo->xface) break;
			msginfo->xface = g_strdup(hp);
			break;
		default:
		}
	}
	msginfo->inreplyto = reference;

	fclose(fp);

	return msginfo;
}

gchar *procheader_get_fromname(const gchar *str)
{
	gchar *tmp, *name;

	Xalloca(tmp, strlen(str) + 1, return NULL);
	strcpy(tmp, str);

	if (*tmp == '\"') {
		extract_quote(tmp, '\"');
		g_strstrip(tmp);
	} else if (strchr(tmp, '<')) {
		eliminate_parenthesis(tmp, '<', '>');
		g_strstrip(tmp);
		if (*tmp == '\0') {
			strcpy(tmp, str);
			extract_parenthesis(tmp, '<', '>');
			g_strstrip(tmp);
		}
	} else if (strchr(tmp, '(')) {
		extract_parenthesis(tmp, '(', ')');
		g_strstrip(tmp);
	}

	if (*tmp == '\0')
		name = g_strdup(str);
	else
		name = g_strdup(tmp);

	return name;
}

time_t procheader_date_parse(gchar *dest, const gchar *src, gint len)
{
	static gchar monthstr[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
	gchar weekday[4];
	gint day;
	gchar month[4];
	gint year;
	gint hh, mm, ss;
	gchar zone[6];
	gint result;
	GDateMonth dmonth;
	struct tm t;
	gchar *p;
	time_t timer;

	/* parsing date field... */
	result = sscanf(src, "%3s, %d %3s %d %2d:%2d:%2d %5s",
			weekday, &day, month, &year, &hh, &mm, &ss, zone);
	if (result != 8) {
		result = sscanf(src, "%d %3s %d %2d:%2d:%2d %5s",
				&day, month, &year, &hh, &mm, &ss, zone);
		if (result != 7) {
			ss = 0;
			result = sscanf(src, "%3s, %d %3s %d %2d:%2d %5s",
					weekday, &day, month, &year, &hh, &mm, zone);
			if (result != 7) {
				result = sscanf(src, "%d %3s %d %2d:%2d %5s",
						&day, month, &year, &hh, &mm,
						zone);
				if (result != 6) {
					g_warning("Invalid date: %s\n", src);
					if (dest && len > 0)
						strncpy2(dest, src, len);
					return 0;
				}
			}
		}
	}

	/* Y2K compliant :) */
	if (year < 100) {
		if (year < 70)
			year += 2000;
		else
			year += 1900;
	}

	if ((p = strstr(monthstr, month)) != NULL)
		dmonth = (gint)(p - monthstr) / 3 + 1;
	else {
		g_warning("Invalid month: %s\n", month);
		dmonth = G_DATE_BAD_MONTH;
	}

	t.tm_sec = ss;
	t.tm_min = mm;
	t.tm_hour = hh;
	t.tm_mday = day;
	t.tm_mon = dmonth - 1;
	t.tm_year = year - 1900;
	t.tm_wday = 0;
	t.tm_yday = 0;
	t.tm_isdst = -1;

	timer = mktime(&t);
	timer += tzoffset_sec(&timer) - remote_tzoffset_sec(zone);

	if (dest)
		procheader_date_get_localtime(dest, len, timer);

	return timer;
}

void procheader_date_get_localtime(gchar *dest, gint len, const time_t timer)
{
	struct tm *lt;
	gchar *default_format = "%y/%m/%d(%a) %H:%M";

	lt = localtime(&timer);

	if (prefs_common.date_format)
		strftime(dest, len, prefs_common.date_format, lt);
	else
		strftime(dest, len, default_format, lt);
}
