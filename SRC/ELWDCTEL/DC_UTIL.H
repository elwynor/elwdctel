/* Build 26.10.27.3 08:21PM */
/*****************************************************************************
 *   DC_UTIL.H                     Dice for Teleconference - helpers         *
 *                                                                           *
 *   Copyright (C) 2026 Elwynor Technologies.                                *
 *                                                                           *
 *   Small routines with no BBS dependencies, so the unit tests in TESTS\    *
 *   can build and run them on their own: reading what the user typed after  *
 *   ROLL, throwing dice, laying out the results, and the settings editor's  *
 *   text parsing.                                                           *
 *                                                                           *
 *   Licensed under the GNU Affero General Public License v3.0.              *
 *****************************************************************************/

#ifndef DC_UTIL_H
#define DC_UTIL_H

#include <stddef.h>

/* Results of dc_parsedice().                                               */
enum {
     DC_OK = 0,                      /* n and sides are set                  */
     DC_BADFMT,                      /* not "[n][Ds]" at all                 */
     DC_BADN,                        /* number of dice out of range          */
     DC_BADSIDES                     /* number of sides out of range         */
};

/* Read a dice spec: an optional count, then optionally "D" and a number of */
/* sides - "3", "d20", "2D8".  Case and spaces don't matter ("2 d 8").      */
/* Defaults are one die and defsides sides.  The whole text must be used:   */
/* "3D8x" is DC_BADFMT.  Counts are checked against 1..maxn and sides       */
/* against 2..maxsides.                                                      */
int dc_parsedice(const char *s, int maxn, int maxsides, int defsides,
                 int *n, int *sides);

/* A fair random number from 0 to bound-1 (no modulo bias).  Uses the       */
/* operating system's secure generator.  bound < 1 gives 0.                  */
int dc_rand(int bound);

/* Write rolls as "4, 2 and 6" into dst.  Returns 1 if it all fit, else 0   */
/* and dst is empty (the caller then shows just the total).                 */
int dc_fmtrolls(char *dst, size_t size, const int *rolls, int n);

/* Copy src to dst (size bytes, always terminated), keeping only printable  */
/* characters, so typed text can't carry ANSI codes or bells onto other     */
/* users' screens.  Runs of spaces collapse to one, and leading and         */
/* trailing spaces are dropped.  At most maxlen characters are kept.        */
void dc_cleantext(char *dst, size_t size, const char *src, size_t maxlen);

/* Is the channel allowed by the list?  An empty list or "*" allows every   */
/* channel.  Otherwise the list is channel names separated by commas,       */
/* compared without regard to case or surrounding spaces.                   */
int dc_chanok(const char *list, const char *channel);

/* Read a whole number from s.  Returns 1 and sets *val if s is only an     */
/* optional sign and digits (surrounding spaces allowed), else returns 0.   */
int dc_parsenum(const char *s, long *val);

/* Read a yes/no answer: YES, Y, ON, 1 give 1; NO, N, OFF, 0 give 0;       */
/* anything else gives -1.                                                   */
int dc_parsebool(const char *s);

#endif /* DC_UTIL_H */
