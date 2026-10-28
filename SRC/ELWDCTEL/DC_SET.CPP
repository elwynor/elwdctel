/* Build 26.10.27.3 08:21PM */
/*****************************************************************************
 *   DC_SET.CPP                    Dice for Teleconference - live settings  *
 *                                                                           *
 *   Copyright (C) 2026 Elwynor Technologies.                                *
 *                                                                           *
 *   Loads, seeds and saves the settings record, and describes each setting  *
 *   (its label, allowed values and default) so the "DICE SET" editor in    *
 *   ELWDCTEL.CPP can list and change them.  See DC_SET.H.                   *
 *                                                                           *
 *   Licensed under the GNU Affero General Public License v3.0.              *
 *****************************************************************************/

#include <stddef.h>
#include <stdio.h>
#include "DC_SET.H"
#include "DC_UTIL.H"

/* The record must fit the stored size, which never changes.               */
typedef char dcs_fits[(sizeof(struct dcsetdat) + 16 <= DCS_RECLEN) ? 1 : -1];

struct dcsetdat dccfg;               /* the live settings                    */
static DFAFILE *dcsdat = NULL;       /* ELWDCTEL.DAT                         */

enum { T_BOOL, T_NUM, T_KEY, T_CHAN };

struct setdef {
     INT type;
     size_t offset;                  /* where it lives in struct dcsetdat    */
     INT size;                       /* text buffer size (incl. terminator)  */
     LONG min, max;                  /* number range                         */
     LONG defnum;                    /* default for numbers and yes/no       */
     const CHAR *defstr;             /* default for text                     */
     const CHAR *label;              /* what the sysop sees                  */
};

/* The defaults live here, not in ELWDCTEL.MSG: settings are changed only  */
/* online, so there is one place for them.  They seed a new install and   */
/* back "DICE SET DEFAULTS".                                               */
#define OFS(f) offsetof(struct dcsetdat, f)
static const struct setdef setdefs[] = {
     { T_BOOL, OFS(enabled),  0, 0, 1, 1, NULL,
       "Dice commands work" },
     { T_BOOL, OFS(autoon),   0, 0, 1, 0, NULL,
       "ROLL is already on when a user signs on (otherwise DICE ON)" },
     { T_NUM,  OFS(maxdice),  0, 1, 100, 50, NULL,
       "Most dice in one roll" },
     { T_NUM,  OFS(maxsides), 0, 2, 1000, 100, NULL,
       "Most sides on one die" },
     { T_KEY,  OFS(usekey),   LOKSIZ, 0, 0, 0, "",
       "Key to use dice (blank = everyone)" },
     { T_CHAN, OFS(channels), DCS_CHNSIZ, 0, 0, 0, "",
       "Channels it works in (blank = all; names separated by commas)" },
};
#define NSET ((INT)(sizeof(setdefs) / sizeof(setdefs[0])))

static VOID *
where(INT i)
{
     return((CHAR *)&dccfg + setdefs[i].offset);
}

static VOID
loadone(INT i)                       /* one setting to its default           */
{
const struct setdef *d = &setdefs[i];

     if (d->type == T_BOOL || d->type == T_NUM) {
          *(SHORT *)where(i) = (SHORT)d->defnum;
     }
     else {
          stzcpy((CHAR *)where(i), d->defstr, d->size);
     }
}

static VOID
sanity(VOID)                         /* keep loaded values inside limits     */
{
INT i;
const struct setdef *d;
SHORT *s;

     for (i = 0; i < NSET; i++) {
          d = &setdefs[i];
          if (d->type == T_BOOL || d->type == T_NUM) {
               s = (SHORT *)where(i);
               if (*s < d->min || *s > d->max) {
                    *s = (SHORT)d->defnum;
               }
          }
          else {
               ((CHAR *)where(i))[d->size - 1] = '\0';
          }
     }
}

static VOID
save(VOID)                           /* write dccfg to ELWDCTEL.DAT          */
{
struct dcsetrec rec;
GBOOL ok;

     setmem((CHAR *)&rec, sizeof(rec), 0);
     /* The key must be in writable memory (a stack copy, never the string  */
     /* constant): the data-file code reads keys through goodblk(), which   */
     /* treats read-only memory as empty (WG33 DFAAPI.C, ISGOODPT.C).        */
     stzcpy(rec.recid, DCS_RECID, sizeof(rec.recid));
     dccfg.layout = 1;
     rec.d = dccfg;
     dfaSetBlk(dcsdat);
     if (dfaAcqEQ(NULL, rec.recid, 0)) {
          ok = dfaUpdateDup(&rec);
     }
     else {
          ok = dfaInsertDup(&rec);
     }
     dfaRstBlk();
     if (!ok) {
          shocst("DICE SETTINGS NOT SAVED", "%s", "Could not write ELWDCTEL.DAT");
     }
}

VOID
dcset_defaults(VOID)
{
INT i;

     setmem((CHAR *)&dccfg, sizeof(dccfg), 0);
     for (i = 0; i < NSET; i++) {
          loadone(i);
     }
     sanity();
     save();
}

VOID
dcset_open(VOID)
{
struct dcsetrec rec;

     if (GetFileAttributesA(DC_SETFN) == INVALID_FILE_ATTRIBUTES) {
          struct dfaSegSpec seg;
          struct dfaKeySpec key;

          setmem((CHAR *)&seg, sizeof(seg), 0);
          seg.position = offsetof(struct dcsetrec, recid);
          seg.length = sizeof(rec.recid);
          seg.type = DFAST_ZSTRING;
          setmem((CHAR *)&key, sizeof(key), 0);
          key.nSegments = 1;
          key.segs = &seg;
          /* The page must be bigger than the record (Btrieve error 28).    */
          dfaCreateSpec(DC_SETFN, FALSE, sizeof(struct dcsetrec), 1024,
                        0, 0, 1, &key, NULL);
     }
     dcsdat = dfaOpen(DC_SETFN, sizeof(struct dcsetrec), NULL);

     setmem((CHAR *)&rec, sizeof(rec), 0);
     stzcpy(rec.recid, DCS_RECID, sizeof(rec.recid));   /* writable key    */
     dfaSetBlk(dcsdat);
     if (dfaAcqEQ(&rec, rec.recid, 0)) {
          dfaRstBlk();
          dccfg = rec.d;
          sanity();
          return;
     }
     dfaRstBlk();
     dcset_defaults();               /* a new install                        */
     shocst("DICE SETTINGS CREATED", "%s", "New settings file, default values");
}

VOID
dcset_close(VOID)
{
     if (dcsdat != NULL) {
          dfaClose(dcsdat);
          dcsdat = NULL;
     }
}

INT
dcset_count(VOID)
{
     return(NSET);
}

const CHAR *
dcset_label(INT n)
{
     return(n >= 1 && n <= NSET ? setdefs[n - 1].label : "");
}

const CHAR *
dcset_value(INT n)
{
static CHAR buf[DCS_CHNSIZ + 16];
const struct setdef *d;

     if (n < 1 || n > NSET) {
          return("");
     }
     d = &setdefs[n - 1];
     switch (d->type) {
     case T_BOOL:
          return(*(SHORT *)where(n - 1) ? "Yes" : "No");
     case T_NUM:
          sprintf(buf, "%d", *(SHORT *)where(n - 1));
          return(buf);
     default:
          if (*(CHAR *)where(n - 1) == '\0') {
               return(d->type == T_CHAN ? "(all channels)" : "(everyone)");
          }
          return((CHAR *)where(n - 1));
     }
}

const CHAR *
dcset_range(INT n)
{
static CHAR buf[64];
const struct setdef *d;

     if (n < 1 || n > NSET) {
          return("");
     }
     d = &setdefs[n - 1];
     switch (d->type) {
     case T_BOOL:
          return("YES or NO");
     case T_NUM:
          sprintf(buf, "a number from %ld to %ld", d->min, d->max);
          return(buf);
     case T_KEY:
          sprintf(buf, "a key name up to %d characters, or - for blank",
                  d->size - 1);
          return(buf);
     default:
          sprintf(buf, "channel names separated by commas (up to %d"
                  " characters), or - for all", d->size - 1);
          return(buf);
     }
}

GBOOL
dcset_change(INT n, const CHAR *text)
{
const struct setdef *d;
LONG v;
INT b;
CHAR tmp[DCS_CHNSIZ];

     if (n < 1 || n > NSET) {
          return(FALSE);
     }
     d = &setdefs[n - 1];
     switch (d->type) {
     case T_BOOL:
          if ((b = dc_parsebool(text)) < 0) {
               return(FALSE);
          }
          *(SHORT *)where(n - 1) = (SHORT)b;
          break;
     case T_NUM:
          if (!dc_parsenum(text, &v) || v < d->min || v > d->max) {
               return(FALSE);
          }
          *(SHORT *)where(n - 1) = (SHORT)v;
          break;
     default:
          dc_cleantext(tmp, sizeof(tmp), text, sizeof(tmp) - 1);
          if (strlen(tmp) > (size_t)(d->size - 1)) {
               return(FALSE);
          }
          if (strcmp(tmp, "-") == 0) {
               tmp[0] = '\0';
          }
          if (d->type == T_KEY) {
               strupr(tmp);          /* a key or a lock expression           */
          }
          stzcpy((CHAR *)where(n - 1), tmp, d->size);
          break;
     }
     save();
     return(TRUE);
}
