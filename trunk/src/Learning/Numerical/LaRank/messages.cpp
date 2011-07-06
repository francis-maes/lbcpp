/***********************************************************************
 * 
 *  LUSH Lisp Universal Shell
 *    Copyright (C) 2002 Leon Bottou, Yann Le Cun, AT&T Corp, NECI.
 *  Includes parts of TL3:
 *    Copyright (C) 1987-1999 Leon Bottou and Neuristique.
 *  Includes selected parts of SN3.2:
 *    Copyright (C) 1991-2001 AT&T Corp.
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA
 * 
 ***********************************************************************/

/***********************************************************************
 * $Id: messages.c,v 1.4 2005/11/16 00:10:01 agbs Exp $
 **********************************************************************/

#include <stdlib.h>
#include <stdio.h>

#include "messages.h"

larank_message_t larank_message_level = larank_INFO;

larank_message_proc_t *larank_message_proc = 0;


static void
defaultproc (larank_message_t level, const char *fmt, va_list ap)
{
  if (level <= larank_message_level)
    vprintf (fmt, ap);
#ifdef LUSH
  if (level <= larank_ERROR)
    {
      extern void run_time_error (const char *s);
      run_time_error ("larank error");
    }
#endif
  if (level <= larank_ERROR)
    abort ();
}

void
larank_error (const char *fmt, ...)
{
  larank_message_proc_t *f = larank_message_proc;
  va_list ap;
  va_start (ap, fmt);
  if (!f)
    f = defaultproc;
  (*f) (larank_ERROR, fmt, ap);
  va_end (ap);
  abort ();
}

void
larank_warning (const char *fmt, ...)
{
  larank_message_proc_t *f = larank_message_proc;
  va_list ap;
  va_start (ap, fmt);
  if (!f)
    f = defaultproc;
  (*f) (larank_WARNING, fmt, ap);
  va_end (ap);
}

void
larank_info (const char *fmt, ...)
{
  larank_message_proc_t *f = larank_message_proc;
  va_list ap;
  va_start (ap, fmt);
  if (!f)
    f = defaultproc;
  (*f) (larank_INFO, fmt, ap);
  va_end (ap);
}

void
larank_debug (const char *fmt, ...)
{
  larank_message_proc_t *f = larank_message_proc;
  va_list ap;
  va_start (ap, fmt);
  if (!f)
    f = defaultproc;
  (*f) (larank_DEBUG, fmt, ap);
  va_end (ap);
}

void
larank_assertfail (const char *file, int line)
{
  larank_error ("Assertion failed: %s:%d\n", file, line);
}
