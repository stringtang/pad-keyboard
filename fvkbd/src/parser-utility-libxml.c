/*
 * parser-utility-libxml.c: parser utilities using libxml
 *
 * Copyright (C) 2009, Intel Corporation.
 *
 * Author: Raymond Liu <raymond.liu@intel.com>
 * 
 * 
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


#include "fvkbd.h"

#include <libxml/parser.h>
#include <libxml/tree.h>

#include "parser-utility-libxml.h"

#define FVKBD_PARSER_GET_PRIVATE(object)\
	(G_TYPE_INSTANCE_GET_PRIVATE((object), FVKBD_TYPE_PARSER, FvkbdParserPrivate))

struct _FvkbdParserPrivate
{
	xmlDocPtr doc;
	xmlNodePtr cur;
};


G_DEFINE_TYPE (FvkbdParser, fvkbd_parser, G_TYPE_OBJECT)


static void
fvkbd_parser_finalize(GObject *object)
{
	FvkbdParser *parser = FVKBD_PARSER(object);

	g_free(parser->err_msg);
	G_OBJECT_CLASS(fvkbd_parser_parent_class)->finalize(object);
}


static void
fvkbd_parser_class_init (FvkbdParserClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass, sizeof(FvkbdParserPrivate));

	gobject_class->finalize = fvkbd_parser_finalize;
}


static void
fvkbd_parser_init (FvkbdParser *self)
{
	FvkbdParserPrivate *priv = FVKBD_PARSER_GET_PRIVATE(self);

	priv->doc = NULL;
	priv->cur = NULL;

	self->err = FALSE;
	self->err_msg = NULL;
}


FvkbdParser *
fvkbd_parser_new (void)
{
	static gboolean xmlParserInitialized = FALSE;

	if (!xmlParserInitialized)
		xmlInitParser();

	return g_object_new(FVKBD_TYPE_PARSER, NULL);
}


void
fvkbd_parser_cleanup (FvkbdParser *parser)
{
	xmlCleanupParser();
}


#if 0

static char *ElementType[] = {
	"NULL",
	"XML_ELEMENT_NODE",
	"XML_ATTRIBUTE_NODE",
	"XML_TEXT_NODE",
	"XML_CDATA_SECTION_NODE",
	"XML_ENTITY_REF_NODE",
	"XML_ENTITY_NODE",
	"XML_PI_NODE",
	"XML_COMMENT_NODE",
	"XML_DOCUMENT_NODE",
	"XML_DOCUMENT_TYPE_NODE",
	"XML_DOCUMENT_FRAG_NODE",
	"XML_NOTATION_NODE",
	"XML_HTML_DOCUMENT_NODE",
	"XML_DTD_NODE",
	"XML_ELEMENT_DECL",
	"XML_ATTRIBUTE_DECL",
	"XML_ENTITY_DECL",
	"XML_NAMESPACE_DECL",
	"XML_XINCLUDE_START",
	"XML_XINCLUDE_END",
	"XML_DOCB_DOCUMENT_NODE"
	"NULL"
};

static void
fvkbd_parser_print_elements(xmlNodePtr node)
{
	xmlNodePtr cur_node = NULL;

	for (cur_node = node; cur_node; cur_node = cur_node->next) {
		if (cur_node->type == XML_ELEMENT_NODE) {
			g_printf("node type: Element, name: %s\n", cur_node->name);
		} else {
			g_printf("node type: %s, content: %s\n", ElementType[cur_node->type], XML_GET_CONTENT(cur_node));
		}

		fvkbd_parser_print_elements(cur_node->children);
	}
}
#endif


gint
fvkbd_parser_load_file (FvkbdParser *parser, const gchar *filename)
{
	xmlDocPtr doc = NULL;
	FvkbdParserPrivate *priv = FVKBD_PARSER_GET_PRIVATE(parser);

	if (filename == NULL)
		goto fail;

	doc = xmlReadFile(filename, NULL, 0);
	if (doc == NULL)
		goto fail;

	priv->cur = xmlDocGetRootElement(doc);
	if (priv->cur == NULL)
		goto fail;

	if (priv->doc)
		xmlFreeDoc(priv->doc);

	priv->doc = doc;

	return 0;

fail:
	return -1;
}


void
fvkbd_parser_free_file (FvkbdParser *parser)
{
	FvkbdParserPrivate *priv = FVKBD_PARSER_GET_PRIVATE(parser);

	if (priv->doc)
		xmlFreeDoc(priv->doc);

	priv->doc = NULL;
	priv->cur = NULL;
}


inline gboolean
fvkbd_parser_element_next (FvkbdParser *parser)
{
	FvkbdParserPrivate *priv = FVKBD_PARSER_GET_PRIVATE(parser);

	if (priv->cur == NULL)
		return FALSE;


	while (priv->cur->next != NULL) {
		priv->cur = priv->cur->next;
		if (priv->cur->type == XML_ELEMENT_NODE)
			return TRUE;
	}

	return FALSE;
}


inline gboolean
fvkbd_parser_element_enter (FvkbdParser *parser)
{
	FvkbdParserPrivate *priv = FVKBD_PARSER_GET_PRIVATE(parser);

	if ((priv->cur == NULL) || (priv->cur->children == NULL))
		return FALSE;

	priv->cur = priv->cur->children;
	return TRUE;
}


inline gboolean
fvkbd_parser_go_child_element (FvkbdParser *parser)
{
	FvkbdParserPrivate *priv = FVKBD_PARSER_GET_PRIVATE(parser);

	if ((priv->cur == NULL) || (priv->cur->children == NULL))
		return FALSE;

	priv->cur = priv->cur->children;

	while (priv->cur->type != XML_ELEMENT_NODE) {
		if (priv->cur->next == NULL)
			return FALSE;
		priv->cur = priv->cur->next;
	}

	return TRUE;
}


inline gboolean
fvkbd_parser_go_parent_element (FvkbdParser *parser)
{
	FvkbdParserPrivate *priv = FVKBD_PARSER_GET_PRIVATE(parser);

	if ((priv->cur == NULL) || (priv->cur->parent == NULL))
		return FALSE;

	priv->cur = priv->cur->parent;
	return TRUE;
}


inline gboolean
fvkbd_parser_element_match (FvkbdParser *parser, gchar *name)
{
	FvkbdParserPrivate *priv = FVKBD_PARSER_GET_PRIVATE(parser);

	if (priv->cur == NULL)
		return FALSE;

	DBG("element_name = %s, match_name = %s", priv->cur->name, name);
	return (xmlStrEqual(priv->cur->name, (const xmlChar *)name) == 1);
}


gchar *
fvkbd_parser_element_get_content (FvkbdParser *parser)
{
	FvkbdParserPrivate *priv = FVKBD_PARSER_GET_PRIVATE(parser);
	gchar *value = NULL;

	if (priv->cur == NULL)
		return NULL;

	value = (gchar *)(xmlNodeGetContent(priv->cur));

	DBG("element_name = %s, content = %s", priv->cur->name, value);
	return value;
}


gchar *
fvkbd_parser_get_attribute (FvkbdParser *parser, gchar *name)
{
	FvkbdParserPrivate *priv = FVKBD_PARSER_GET_PRIVATE(parser);
	gchar *value = NULL;

	if (priv->cur == NULL || name == NULL)
		return NULL;

	value = (gchar *)(xmlGetProp(priv->cur, (const xmlChar *)name));

	DBG("attr_name = %s, value = %s", name, value);
	return value;
}


void fvkbd_parser_set_error (FvkbdParser *parser, gchar *err_msg)
{
	FvkbdParserPrivate *priv = FVKBD_PARSER_GET_PRIVATE(parser);

	if (priv->cur == NULL)
		return;

	if (parser->err_msg)
		g_free(parser->err_msg);

	parser->err_msg = g_strdup_printf("Element(%s), Err:%s\n", priv->cur->name, err_msg);

	parser->err = TRUE;
}

