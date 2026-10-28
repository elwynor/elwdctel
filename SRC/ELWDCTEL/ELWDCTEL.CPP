/* Build 26.10.27.3 08:21PM */
/*****************************************************************************
 *   ELWDCTEL.CPP                  Dice for Teleconference - the module      *
 *                                                                           *
 *   Copyright (C) 2026 Elwynor Technologies.                                *
 *   Originally HOTELeconference Dice (ANXHDICE) by Randall R. Spangler,     *
 *   annex.com, 1997.                                                        *
 *   Converted for The Major BBS v10 by Mark Laudenbach -                    *
 *   https://github.com/laudenbachm                                          *
 *                                                                           *
 *   A user in Teleconference types                                          *
 *                                                                           *
 *        DICE ON          to allow dice rolling                             *
 *        ROLL 3D8         to roll three eight-sided dice                    *
 *                                                                           *
 *   and everyone in the channel sees the roll.  The result is sent the      *
 *   same way an ordinary chat line is, so it also reaches users on other    *
 *   boards in a linked channel such as MajorLink (formerly WorldLink).      *
 *                                                                           *
 *   The module has no menu entry.  It adds the DICE and ROLL commands to    *
 *   the stock Teleconference when the board starts.  Sysops change its      *
 *   settings online, in Teleconference, with DICE SET.                      *
 *                                                                           *
 *   Licensed under the GNU Affero General Public License v3.0.              *
 *****************************************************************************/

#include <stdlib.h>
#include <time.h>
#include "DC_DEFS.H"
#include "DC_TLC.H"
#include "DC_SET.H"
#include "DC_UTIL.H"
#include "ELWDCTEL.H"                /* message numbers, from ELWDCTEL.MST   */

#define DC_HARDMAXN  100             /* the most dice any setting allows     */

extern "C" {
VOID EXPORT init__elwdctel(VOID);
}

static VOID dcstart(VOID);
static GBOOL dclon(VOID);
static VOID dchup(VOID);
static VOID dcfin(VOID);
static SHORT dcdicecmd(LPCMDPARAM param);
static SHORT dcrollcmd(LPCMDPARAM param);

/* The module block.  Only the logon, hang-up and shut-down routines are   */
/* used: users never "enter" this module, they use it from Teleconference. */
static struct module dcmod = {
     "",                             /* name, filled in from ELWDCTEL.MDF    */
     dclon,                          /* logon: ROLL starts off (or on)       */
     NULL,                           /* input (not used)                     */
     NULL,                           /* status input (not used)              */
     NULL,                           /* injoth (not used)                    */
     NULL,                           /* logoff (not used)                    */
     dchup,                          /* hang-up: ROLL off for this channel   */
     NULL,                           /* nightly cleanup (not used)           */
     NULL,                           /* account deleted (not used)           */
     dcfin                           /* clean shut-down                      */
};

/* The Teleconference commands.  The engine keeps a pointer to each of     */
/* these, so they must stay put for the life of the board (tlcdyncm.cpp).  */
static CMDSTRUCT dccmdst;            /* DICE                                 */
static CMDSTRUCT dcrollst;           /* ROLL                                 */

static HMCVFILE dcmb = NULL;         /* ELWDCTEL.MCV                         */
static CHAR *rollon = NULL;          /* per channel: 1 = ROLL is switched on */

/* Values for the text variables.  Each is filled in just before the       */
/* message that shows it.                                                   */
static CHAR tvndice[8];
static CHAR tvsides[8];
static CHAR tvtotal[16];
static CHAR tvrolls[DC_LISTSIZ];     /* "4, 2 and 6"                         */
static CHAR tvchan[CHAN_MAX_SIZE];
static CHAR tvsetnum[8];
static CHAR tvsetnam[80];
static CHAR tvsetval[DCS_CHNSIZ + 16];
static CHAR tvsetrng[96];

static CHAR *tv_ndice(VOID)    { return(tvndice); }
static CHAR *tv_sides(VOID)    { return(tvsides); }
static CHAR *tv_total(VOID)    { return(tvtotal); }
static CHAR *tv_rolls(VOID)    { return(tvrolls); }
static CHAR *tv_chan(VOID)     { return(tvchan); }
static CHAR *tv_setnum(VOID)   { return(tvsetnum); }
static CHAR *tv_setnam(VOID)   { return(tvsetnam); }
static CHAR *tv_setval(VOID)   { return(tvsetval); }
static CHAR *tv_setrng(VOID)   { return(tvsetrng); }
static CHAR *tv_version(VOID)  { return((CHAR *)ELWDCTEL_VERSION); }
static CHAR *tv_dice(VOID)     { return((CHAR *)DC_CMDDICE); }
static CHAR *tv_roll(VOID)     { return((CHAR *)DC_CMDROLL); }

static CHAR *
tv_maxn(VOID)                        /* the current limit, read live         */
{
static CHAR buf[8];

     sprintf(buf, "%d", dccfg.maxdice);
     return(buf);
}

static CHAR *
tv_maxs(VOID)
{
static CHAR buf[8];

     sprintf(buf, "%d", dccfg.maxsides);
     return(buf);
}

/***************************************************************************
 *   Start-up and shut-down                                                *
 ***************************************************************************/

VOID EXPORT
init__elwdctel(VOID)
{
     stzcpy(dcmod.descrp, gmdnam("ELWDCTEL.MDF"), MNMSIZ);
     register_module(&dcmod);
     dcmb = opnmsg("ELWDCTEL.MCV");
     rollon = (CHAR *)alczer(nterms);

     /* Text variable names are shared by the whole board, so they all     */
     /* start with DCT_.                                                     */
     register_textvar("DCT_NDICE", tv_ndice);
     register_textvar("DCT_SIDES", tv_sides);
     register_textvar("DCT_TOTAL", tv_total);
     register_textvar("DCT_ROLLS", tv_rolls);
     register_textvar("DCT_CHANNEL", tv_chan);
     register_textvar("DCT_SETNUM", tv_setnum);
     register_textvar("DCT_SETNAME", tv_setnam);
     register_textvar("DCT_SETVALUE", tv_setval);
     register_textvar("DCT_SETRANGE", tv_setrng);
     register_textvar("DCT_VERSION", tv_version);
     register_textvar("DCT_DICE", tv_dice);
     register_textvar("DCT_ROLL", tv_roll);
     register_textvar("DCT_MAXN", tv_maxn);
     register_textvar("DCT_MAXS", tv_maxs);

     dcset_open();

     if (!dctlc_bind()) {
          shocst("DICE NOT STARTED", "v%s: %s", ELWDCTEL_VERSION, dctlc_error());
          return;
     }
     /* The commands are added once every module has started (the board's  */
     /* "startup vectors", MAJORBBS.C), so Teleconference and the modules  */
     /* it depends on are ready whatever order the board loaded them in.   */
     hook_startup(dcstart);
}

static GBOOL
addcmd(LPCMDSTRUCT st, const CHAR *word, SHORT (*func)(LPCMDPARAM))
{
     /* "fNSpace" = 1: the command is a word of its own, followed by a      */
     /* space and its arguments (tlcapi.cpp Parse).                         */
     setmem((CHAR *)st, sizeof(*st), 0);
     stzcpy(st->pszLong, word, MAX_CMD_SIZEL);
     st->fNSpace = 1;
     st->bTOnly = FALSE;             /* web Teleconference users too         */
     st->docmdfunc = func;
     return(dctlc_register(st));
}

static VOID
dcstart(VOID)
{
     dctlc_init();
     if (!addcmd(&dccmdst, DC_CMDDICE, dcdicecmd)
      || !addcmd(&dcrollst, DC_CMDROLL, dcrollcmd)) {
          shocst("DICE NOT STARTED", "v%s: %s", ELWDCTEL_VERSION,
                 "Teleconference refused the DICE or ROLL command");
          return;
     }
     shocst("DICE FOR TELECONFERENCE", "v%s loaded (commands %s, %s)",
            ELWDCTEL_VERSION, DC_CMDDICE, DC_CMDROLL);
}

static GBOOL
dclon(VOID)
{
     /* A new caller on this channel starts with ROLL as the Sysop set it   */
     /* up.  Done at logon as well as hang-up, because the board reuses a   */
     /* channel the moment a caller drops, and a missed hang-up must not    */
     /* carry the last caller's choice over.                                */
     rollon[usrnum] = (CHAR)(dccfg.autoon ? 1 : 0);
     return(0);                      /* carry on with the normal logon       */
}

static VOID
dchup(VOID)
{
     rollon[usrnum] = 0;
}

static VOID
dcfin(VOID)
{
     /* The Teleconference has no way to remove a command, and it shuts     */
     /* down with the board, so there is nothing to undo there.             */
     dcset_close();
     if (dcmb != NULL) {
          clsmsg(dcmb);
          dcmb = NULL;
     }
}

/***************************************************************************
 *   Talking to the user                                                   *
 ***************************************************************************/

static VOID
tell(LPUSRSTRUCT u)                  /* send prfbuf to the user, privately   */
{
     /* Going through the Teleconference (not straight to the channel)      */
     /* reaches web users too, and keeps the output in step with the        */
     /* Teleconference's own prompt.                                         */
     dctlc_usersend(u->m_pszName, u->m_pszName, NULL, RECV_TYPE_PRFBUF);
     clrprf();
}

static VOID
say(LPUSRSTRUCT u, INT msgnum)       /* one message, privately               */
{
     clrprf();
     prfmsg(msgnum);
     tell(u);
}

/***************************************************************************
 *   DICE SET - the settings editor                                        *
 ***************************************************************************/

static VOID
setlist(LPUSRSTRUCT u)
{
INT n;

     clrprf();
     prfmsg(SETHDR);
     for (n = 1; n <= dcset_count(); n++) {
          sprintf(tvsetnum, "%d", n);
          stzcpy(tvsetnam, dcset_label(n), sizeof(tvsetnam));
          stzcpy(tvsetval, dcset_value(n), sizeof(tvsetval));
          prfmsg(SETROW);
     }
     prfmsg(SETFOOT);
     tell(u);
}

static VOID
seteditor(LPUSRSTRUCT u, CHAR *args) /* args: what follows "SET"            */
{
CHAR *val;
LONG n;
CHAR audit[128];

     while (*args == ' ') {
          args++;
     }
     if (*args == '\0') {
          setlist(u);
          return;
     }
     if (sameas(args, "DEFAULTS")) {
          dcset_defaults();
          _snprintf(audit, sizeof(audit) - 1, "%s reset all settings", u->m_pszName);
          audit[sizeof(audit) - 1] = '\0';
          shocst("DICE SETTINGS RESET", "%s", audit);
          say(u, SETDEF);
          return;
     }
     /* "<number> <value>" */
     for (val = args; *val != '\0' && *val != ' '; val++) {
     }
     if (*val == ' ') {
          *val++ = '\0';
     }
     if (!dc_parsenum(args, &n) || n < 1 || n > dcset_count()) {
          setlist(u);
          return;
     }
     sprintf(tvsetnum, "%ld", n);
     stzcpy(tvsetnam, dcset_label((INT)n), sizeof(tvsetnam));
     while (*val == ' ') {
          val++;
     }
     if (*val == '\0' || !dcset_change((INT)n, val)) {
          stzcpy(tvsetval, dcset_value((INT)n), sizeof(tvsetval));
          stzcpy(tvsetrng, dcset_range((INT)n), sizeof(tvsetrng));
          say(u, SETBAD);
          return;
     }
     stzcpy(tvsetval, dcset_value((INT)n), sizeof(tvsetval));
     /* Audit Trail details must stay under 128 characters (AUDAPI.H).     */
     _snprintf(audit, sizeof(audit) - 1, "%s: #%ld = %.60s", u->m_pszName, n,
               tvsetval);
     audit[sizeof(audit) - 1] = '\0';
     shocst("DICE SETTING CHANGED", "%s", audit);
     say(u, SETOK);
}

/***************************************************************************
 *   Rolling the dice                                                      *
 ***************************************************************************/

static VOID
roll(LPUSRSTRUCT u, const CHAR *spec)
{
INT n, sides, i, total = 0;
INT rolls[DC_HARDMAXN];
GBOOL listed;
CHAR pub[DC_PUBSIZ];

     switch (dc_parsedice(spec, dccfg.maxdice, dccfg.maxsides, DC_DEFSIDES,
                          &n, &sides)) {
     case DC_OK:
          break;
     case DC_BADN:
          say(u, BADN);
          return;
     case DC_BADSIDES:
          say(u, BADSIDES);
          return;
     default:
          say(u, BADFMT);
          return;
     }

     for (i = 0; i < n; i++) {
          rolls[i] = 1 + dc_rand(sides);
          total += rolls[i];
     }

     /* Fill in the text variables.  Past about 30 dice the list of results */
     /* no longer fits on a line, so only the total is shown.               */
     sprintf(tvndice, "%d", n);
     sprintf(tvsides, "%d", sides);
     sprintf(tvtotal, "%d", total);
     listed = (n > 1) && dc_fmtrolls(tvrolls, sizeof(tvrolls), rolls, n);

     /* The line everyone else sees.  It is sent as ordinary chat from this */
     /* user, which is what lets it cross MajorLink: other boards show it   */
     /* as "From <user>: <line>" in their own wording.                      */
     clrprf();
     prfmsg(n == 1 ? PUB1 : (listed ? PUBN : PUBNT));
     stzcpy(pub, prfbuf, sizeof(pub));
     clrprf();
     for (i = 0; pub[i] != '\0'; i++) {          /* keep it to one line      */
          if (pub[i] == '\r' || pub[i] == '\n') {
               pub[i] = ' ';
          }
     }

     /* The roller sees their own version first, in color...               */
     prfmsg(n == 1 ? ME1 : (listed ? MEN : MENT));
     tell(u);

     /* ...then everyone else in the channel, here and on linked boards.   */
     /* The roller is left out: with echo off, Teleconference would only    */
     /* show them "-- Message Sent --" for their own line.                  */
     dctlc_chansendbut(u->m_pszChannel, u->m_pszName, pub, RECV_TYPE_PUBLIC,
                       u->m_pszName);
}

/***************************************************************************
 *   The DICE and ROLL commands                                            *
 ***************************************************************************/

/* Checks that apply to turning dice on and to rolling.  Returns TRUE if   */
/* the user may go on; otherwise it has already told them why.            */
static GBOOL
allowed(LPUSRSTRUCT u)
{
     if (!dccfg.enabled) {
          say(u, RESTING);
     }
     else if (dccfg.usekey[0] != '\0' && haskey(dccfg.usekey) != 1) {
          say(u, NOACCESS);          /* haskey() is -1 for an unknown key    */
     }
     else if (u->m_bSquelched) {
          say(u, SQUELCH);
     }
     else if (!dc_chanok(dccfg.channels, u->m_pszChannel)) {
          stzcpy(tvchan, u->m_pszChannel, sizeof(tvchan));
          say(u, NOCHAN);
     }
     else {
          return(TRUE);
     }
     return(FALSE);
}

static VOID
dicecmd(LPUSRSTRUCT u, CHAR *args)   /* DICE [ON|OFF|HELP|SET ...]           */
{
     if (*args == '\0' || sameas(args, "?") || sameas(args, "HELP")) {
          say(u, HELP);
     }
     else if (haskey("SYSOP") == 1 && (sameas(args, "SET") || sameto("SET ", args))) {
          seteditor(u, args + 3);
     }
     else if (sameas(args, "ON")) {
          if (allowed(u)) {
               rollon[usrnum] = 1;
               say(u, DICEON);
          }
     }
     else if (sameas(args, "OFF")) {
          rollon[usrnum] = 0;
          say(u, DICEOFF);
     }
     else {
          say(u, HELP);
     }
}

static VOID
rollcmd(LPUSRSTRUCT u, CHAR *args)   /* ROLL [n][Dsides]|OFF|HELP            */
{
     if (*args == '\0' || sameas(args, "?") || sameas(args, "HELP")) {
          say(u, HELP);
     }
     else if (sameas(args, "OFF")) {
          rollon[usrnum] = 0;
          say(u, DICEOFF);
     }
     else if (allowed(u)) {
          if (!rollon[usrnum]) {
               say(u, ISOFF);
          }
          else {
               roll(u, args);
          }
     }
}

/* Both commands share this set-up: switch to the user's context (key      */
/* checks need it), and the message file, run the command, put both back.  */
static SHORT
dcrun(LPCMDPARAM param, VOID (*cmd)(LPUSRSTRUCT, CHAR *))
{
LPUSRSTRUCT u = param->m_pUser;
INT savusn = usrnum;
CHAR text[256];
CHAR *args;

     /* Only users on this board can type a command here.  Users from      */
     /* linked boards run their own board's dice, if it has any.            */
     if (u == NULL || u->m_iUsrnum < 0 || u->m_iUsrnum >= nterms) {
          return(0);
     }
     if (usrnum != u->m_iUsrnum) {
          curusr(u->m_iUsrnum);
     }
     setmbk(dcmb);

     stzcpy(text, param->m_pszCmdParam != NULL ? param->m_pszCmdParam : "",
            sizeof(text));
     for (args = text; *args == ' '; args++) {
     }
     cmd(u, args);

     rstmbk();
     if (usrnum != savusn && savusn >= 0) {
          curusr(savusn);
     }
     return(1);                      /* handled: not spoken to the channel   */
}

static SHORT
dcdicecmd(LPCMDPARAM param)
{
     return(dcrun(param, dicecmd));
}

static SHORT
dcrollcmd(LPCMDPARAM param)
{
     return(dcrun(param, rollcmd));
}

VOID EXPORT
end_elwdctel(VOID)                   /* marks the end of the code, for       */
{                                    /* crash reports (GALEXCEP.OUT)         */
}
