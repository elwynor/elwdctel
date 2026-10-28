/* Build 26.10.27.3 08:21PM */
/*****************************************************************************
 *   DC_SET.H                      Dice for Teleconference - live settings  *
 *                                                                           *
 *   Copyright (C) 2026 Elwynor Technologies.                                *
 *                                                                           *
 *   The settings are kept in one record in ELWDCTEL.DAT and are changed     *
 *   online, while the board runs, with "DICE SET" in Teleconference.       *
 *   There are no CNF options: the defaults are in DC_SET.CPP, and they set  *
 *   up a new install and back "DICE SET DEFAULTS".                         *
 *                                                                           *
 *   Adding a setting: add a field at the END of struct dcsetdat, taking its *
 *   room out of spare[] so the record size and earlier fields never move,   *
 *   make an all-zero value mean "as before", and add a row to setdefs[] in  *
 *   DC_SET.CPP.                                                             *
 *                                                                           *
 *   Licensed under the GNU Affero General Public License v3.0.              *
 *****************************************************************************/

#ifndef DC_SET_H
#define DC_SET_H

#include "DC_DEFS.H"

#define DC_SETFN     "ELWDCTEL.DAT"  /* the settings file                    */
#define DCS_RECID    "SETTINGS"      /* key of the one settings record       */
#define DCS_RECLEN   512             /* stored record size; never changes    */
#define DCS_CHNSIZ   200             /* channel list, with its terminator    */

/* The record is stored on disk, so its layout must not depend on compiler */
/* padding: pack it to single bytes.                                        */
#pragma pack(push, 1)
struct dcsetdat {
     SHORT layout;                   /* 1                                    */
     SHORT enabled;                  /* 1 = dice commands work               */
     SHORT autoon;                   /* 1 = ROLL is on at sign-on            */
     SHORT maxdice;                  /* most dice in one roll                */
     SHORT maxsides;                 /* most sides on one die                */
     CHAR  usekey[LOKSIZ];           /* key to use dice; "" = everyone       */
     CHAR  channels[DCS_CHNSIZ];     /* allowed channels; "" = all           */
};

struct dcsetrec {                    /* the record as stored                 */
     CHAR recid[16];                 /* DCS_RECID (key 0)                    */
     struct dcsetdat d;
     CHAR spare[DCS_RECLEN - 16 - sizeof(struct dcsetdat)];
};
#pragma pack(pop)

extern struct dcsetdat dccfg;        /* the live settings                    */

VOID dcset_open(VOID);               /* open the file; load or seed          */
VOID dcset_close(VOID);
VOID dcset_defaults(VOID);           /* every setting back to its default    */

/* For the editor.  Settings are numbered from 1.                          */
INT dcset_count(VOID);
const CHAR *dcset_label(INT n);
const CHAR *dcset_value(INT n);      /* current value, ready to show         */
const CHAR *dcset_range(INT n);      /* what may be entered                  */

/* Change setting n from the text the sysop typed, and save.  "-" clears   */
/* a text setting.  Returns TRUE if the value was accepted.                */
GBOOL dcset_change(INT n, const CHAR *text);

#endif /* DC_SET_H */
