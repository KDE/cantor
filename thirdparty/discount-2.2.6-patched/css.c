/* markdown: a C implementation of John Gruber's Markdown markup language.
 *
 * Copyright (C) 2009 David L Parsons.
 * The redistribution terms are provided in the COPYRIGHT file that must
 * be distributed with this source code.
 */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

#include "config.h"

#include "cstring.h"
#include "markdown.h"
#include "amalloc.h"


/*
 * dump out stylesheet sections.
 */
static void
stylesheets(Paragraph *p, Cstring *f)
{
    Line* q;

    for ( ; p ; p = p->next ) {
	if ( p->typ == STYLE ) {
	    for ( q = p->text; q ; q = q->next ) {
		Cswrite(f, T(q->text), S(q->text));
		Csputc('\n', f);
	    }
	}
	if ( p->down )
	    stylesheets(p->down, f);
    }
}


/* dump any embedded styles to a string
 */
int
mkd_css(Document *d, char **res)
{
    Cstring f;
    int size;

    if ( res && d && d->compiled ) {
	*res = 0;
	CREATE(f);
	RESERVE(f, 100);
	stylesheets(d->code, &f);
			
	if ( (size = S(f)) > 0 ) {
	    /* null-terminate, then strdup() into a free()able memory
	     * chunk
	     */
	    EXPAND(f) = 0;
	    *res = strdup(T(f));
	}
	DELETE(f);
	return size;
    }
    return EOF;
}


/* dump any embedded styles to a file
 */
int
mkd_generatecss(Document *d, FILE *f)
{
    char *res;
    int written;
    int size = mkd_css(d, &res);

    written = (size > 0) ? fwrite(res,1,size,f) : 0;
    
    if ( res )
	free(res);
    
    return (written == size) ? size : EOF;
}
