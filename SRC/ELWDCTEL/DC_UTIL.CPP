/* Build 26.10.27.3 08:21PM */
/*****************************************************************************
 *   DC_UTIL.CPP                   Dice for Teleconference - helpers         *
 *                                                                           *
 *   Copyright (C) 2026 Elwynor Technologies.                                *
 *                                                                           *
 *   See DC_UTIL.H.  Nothing here may use the BBS SDK: the unit tests build  *
 *   this file on its own.                                                   *
 *                                                                           *
 *   Licensed under the GNU Affero General Public License v3.0.              *
 *****************************************************************************/

#define _CRT_RAND_S                  /* for rand_s(), before <stdlib.h>      */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "DC_UTIL.H"

/* Read digits at *pp into *val.  Returns 0 if there are none or the number */
/* is longer than four digits (far past any sane limit; stops overflow).    */
static int
digits(const char **pp, int *val)
{
const char *p = *pp;
int v = 0, len = 0;

     while (*p >= '0' && *p <= '9') {
          if (++len > 4) {
               return(0);
          }
          v = v * 10 + (*p++ - '0');
     }
     if (len == 0) {
          return(0);
     }
     *pp = p;
     *val = v;
     return(1);
}

static const char *
skipsp(const char *p)
{
     while (*p == ' ' || *p == '\t') {
          p++;
     }
     return(p);
}

int
dc_parsedice(const char *s, int maxn, int maxsides, int defsides,
             int *n, int *sides)
{
const char *p = skipsp(s);
int count = 1, sd = defsides;

     if (*p == '\0') {
          return(DC_BADFMT);
     }
     if (*p >= '0' && *p <= '9') {
          if (!digits(&p, &count)) {
               return(DC_BADN);          /* more than four digits            */
          }
          p = skipsp(p);
     }
     if (*p == 'd' || *p == 'D') {
          p = skipsp(p + 1);
          if (!digits(&p, &sd)) {
               return(*p >= '0' && *p <= '9' ? DC_BADSIDES : DC_BADFMT);
          }
          p = skipsp(p);
     }
     if (*p != '\0') {
          return(DC_BADFMT);
     }
     if (count < 1 || count > maxn) {
          return(DC_BADN);
     }
     if (sd < 2 || sd > maxsides) {
          return(DC_BADSIDES);
     }
     *n = count;
     *sides = sd;
     return(DC_OK);
}

int
dc_rand(int bound)
{
unsigned int u, range, limit;

     if (bound < 1) {
          return(0);
     }
     range = (unsigned int)bound;
     /* Throw away the top sliver of values that would make some results    */
     /* more likely than others (modulo bias).                              */
     limit = (0xFFFFFFFFu / range) * range;
     do {
          if (rand_s(&u) != 0) {
               u = (unsigned int)rand() * 32768u + (unsigned int)rand();
          }
     } while (u >= limit);
     return((int)(u % range));
}

int
dc_fmtrolls(char *dst, size_t size, const int *rolls, int n)
{
size_t used = 0;
char one[16];
int i, len;

     if (size == 0) {
          return(0);
     }
     dst[0] = '\0';
     for (i = 0; i < n; i++) {
          sprintf(one, "%d", rolls[i]);
          len = (int)strlen(one);
          /* ", " or " and " between entries (5 bytes at most), then the    */
          /* number itself, and room for the terminator.                    */
          if (used + len + (i > 0 ? 5 : 0) >= size) {
               dst[0] = '\0';
               return(0);
          }
          if (i > 0) {
               strcpy(dst + used, i == n - 1 ? " and " : ", ");
               used += strlen(dst + used);
          }
          strcpy(dst + used, one);
          used += len;
     }
     return(1);
}

void
dc_cleantext(char *dst, size_t size, const char *src, size_t maxlen)
{
size_t n = 0;
int space = 0;
unsigned char c;

     if (size == 0) {
          return;
     }
     while ((c = (unsigned char)*src++) != '\0') {
          if (c == ' ' || c == '\t') {
               space = (n > 0);      /* remember it; leading ones are dropped*/
               continue;
          }
          if (c < 32 || c == 127) {  /* ESC, bell, other control codes       */
               continue;
          }
          if (space) {
               if (n + 1 >= size - 1 || n + 1 >= maxlen) {
                    break;
               }
               dst[n++] = ' ';
               space = 0;
          }
          if (n >= size - 1 || n >= maxlen) {
               break;
          }
          dst[n++] = (char)c;
     }
     dst[n] = '\0';
}

int
dc_chanok(const char *list, const char *channel)
{
const char *p, *b, *e;
size_t len;

     for (p = list; *p == ' '; p++) {
     }
     if (*p == '\0' || strcmp(p, "*") == 0) {
          return(1);
     }
     len = strlen(channel);
     while (*p != '\0') {
          for (b = p; *b == ' '; b++) {      /* start of this name           */
          }
          for (e = b; *e != '\0' && *e != ','; e++) {
          }
          p = (*e == ',') ? e + 1 : e;       /* where the next name starts   */
          while (e > b && e[-1] == ' ') {    /* trim the end of this name    */
               e--;
          }
          if ((size_t)(e - b) == len && len > 0 && _strnicmp(b, channel, len) == 0) {
               return(1);
          }
          if (e - b == 1 && *b == '*') {
               return(1);
          }
     }
     return(0);
}

int
dc_parsenum(const char *s, long *val)
{
char *end;
long v;

     while (*s == ' ') {
          s++;
     }
     if (*s == '\0') {
          return(0);
     }
     v = strtol(s, &end, 10);
     if (end == s) {
          return(0);
     }
     while (*end == ' ') {
          end++;
     }
     if (*end != '\0') {
          return(0);
     }
     *val = v;
     return(1);
}

int
dc_parsebool(const char *s)
{
static const char *yes[] = { "YES", "Y", "ON", "1", NULL };
static const char *no[] = { "NO", "N", "OFF", "0", NULL };
int i;

     while (*s == ' ') {
          s++;
     }
     for (i = 0; yes[i] != NULL; i++) {
          if (_stricmp(s, yes[i]) == 0) {
               return(1);
          }
     }
     for (i = 0; no[i] != NULL; i++) {
          if (_stricmp(s, no[i]) == 0) {
               return(0);
          }
     }
     return(-1);
}
