/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 * 
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#ifndef _PARSER_UTILITY_LIBXML_H
#define _PARSER_UTILITY_LIBXML_H

G_BEGIN_DECLS

typedef struct _FvkbdParser FvkbdParser;
typedef struct _FvkbdParserClass FvkbdParserClass;
typedef struct _FvkbdParserPrivate FvkbdParserPrivate;

#define FVKBD_TYPE_PARSER			(fvkbd_parser_get_type())
#define FVKBD_PARSER(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), FVKBD_TYPE_PARSER, FvkbdParser))
#define FVKBD_PARSER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), FVKBD_TYPE_PARSER, FvkbdParserClass))
#define FVKBD_IS_PARSER(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), FVKBD_TYPE_PARSER))
#define FVKBD_IS_PARSER_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), FVKBD_TYPE_PARSER))
#define FVKBD_PARSER_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), FVKBD_TYPE_PARSER, FvkbdParserClass))

struct _FvkbdParser {
	GObject parent;
	gboolean err;
	char *err_msg;
	FvkbdParserPrivate *private;
};

struct _FvkbdParserClass {
	GObjectClass parent;
};

GType fvkbd_parser_get_type (void);

FvkbdParser *fvkbd_parser_new (void);
void fvkbd_parser_cleanup (FvkbdParser *parser);
gint fvkbd_parser_load_file (FvkbdParser *parser, const gchar *filename);
void fvkbd_parser_free_file (FvkbdParser *parser);

inline gboolean fvkbd_parser_element_next (FvkbdParser *parser);
inline gboolean fvkbd_parser_element_enter (FvkbdParser *parser);
inline gboolean fvkbd_parser_go_child_element (FvkbdParser *parser);
inline gboolean fvkbd_parser_go_parent_element (FvkbdParser *parser);

inline gboolean fvkbd_parser_element_match (FvkbdParser *parser, gchar *name);
gchar *fvkbd_parser_element_get_content (FvkbdParser *parser);
gchar *fvkbd_parser_get_attribute (FvkbdParser *parser, gchar *name);

void fvkbd_parser_set_error (FvkbdParser *parser, gchar *err_msg);

G_END_DECLS

#endif //_PARSER_UTILITY_LIBXML_H