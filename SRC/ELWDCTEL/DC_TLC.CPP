/* Build 26.10.27.3 08:21PM */
/*****************************************************************************
 *   DC_TLC.CPP                    Dice for Teleconference - the link to    *
 *                                 the Teleconference engine (GALTELE.DLL)   *
 *                                                                           *
 *   Copyright (C) 2026 Elwynor Technologies.                                *
 *                                                                           *
 *   See DC_TLC.H for why the functions are found by name at start-up.       *
 *                                                                           *
 *   Licensed under the GNU Affero General Public License v3.0.              *
 *****************************************************************************/

#include "DC_TLC.H"

/* Function shapes, copied from TLCAPIC.H.  The engine exports them as C    */
/* (cdecl) functions with a leading underscore on the name.                 */
typedef VOID        (*PINIT)(VOID);
typedef GBOOL       (*PCMDREG)(LPCMDSTRUCT st);
typedef GBOOL       (*PUSRSEND)(const CHAR *userid, const CHAR *from,
                                const CHAR *to, const CHAR *pMsg, SHORT fType,
                                SHORT iMsgNum, const CHAR *ptr1,
                                const CHAR *ptr2, const CHAR *ptr3);
typedef GBOOL       (*PCHNSENDB)(const CHAR *channel, const CHAR *from,
                                 const CHAR *to, const CHAR *pMsg,
                                 SHORT fType, SHORT iMsgNum, const CHAR *ptr1,
                                 const CHAR *ptr2, const CHAR *ptr3,
                                 const CHAR *not1, const CHAR *not2);

static PINIT     pinit = NULL;
static PCMDREG   pcmdreg = NULL;
static PUSRSEND  pusrsend = NULL;
static PCHNSENDB pchnsendb = NULL;
static CHAR      bnderr[80] = "";

static FARPROC
find(HMODULE h, const CHAR *name)    /* one export, noting a missing one     */
{
FARPROC p = GetProcAddress(h, name);

     if (p == NULL && bnderr[0] == '\0') {
          _snprintf(bnderr, sizeof(bnderr) - 1, "GALTELE.DLL has no %s", name);
          bnderr[sizeof(bnderr) - 1] = '\0';
     }
     return(p);
}

GBOOL
dctlc_bind(VOID)
{
HMODULE h;

     bnderr[0] = '\0';
     /* LoadLibrary returns the copy the board already loaded, or loads it  */
     /* from the BBS folder (the working directory at start-up).            */
     if ((h = LoadLibraryA("GALTELE.DLL")) == NULL) {
          stzcpy(bnderr, "GALTELE.DLL (Teleconference) not found",
                 sizeof(bnderr));
          return(FALSE);
     }
     pinit = (PINIT)find(h, "_init__galtele");
     pcmdreg = (PCMDREG)find(h, "_tlcCommandRegister");
     pusrsend = (PUSRSEND)find(h, "_tlcUserSendText");
     pchnsendb = (PCHNSENDB)find(h, "_tlcChannelPublicSendBut");
     if (bnderr[0] != '\0') {
          return(FALSE);
     }
     return(TRUE);
}

VOID
dctlc_init(VOID)
{
     /* Make sure the engine is set up before we use it.  It only runs once */
     /* (TLC.CPP init__galtele, guarded by "isinit"), so this is a no-op    */
     /* when the board has already started Teleconference.  The WorldLink   */
     /* Teleconference add-on does the same thing.                          */
     if (pinit != NULL) {
          pinit();
     }
}

const CHAR *
dctlc_error(VOID)
{
     return(bnderr);
}

GBOOL
dctlc_register(LPCMDSTRUCT cmd)
{
     return(pcmdreg != NULL && pcmdreg(cmd));
}

GBOOL
dctlc_chansendbut(const CHAR *channel, const CHAR *from, const CHAR *text,
                  SHORT type, const CHAR *notuser)
{
     if (pchnsendb == NULL) {
          return(FALSE);
     }
     /* Message number 0 means "use the Teleconference's own wording".  Any */
     /* other number would be looked up in the RECEIVING board's           */
     /* GALTELE.MSG, which is different on every board.                     */
     return(pchnsendb(channel, from, NULL, text, type, 0, NULL, NULL, NULL,
                      notuser, NULL));
}

GBOOL
dctlc_usersend(const CHAR *userid, const CHAR *from, const CHAR *text,
               SHORT type)
{
     if (pusrsend == NULL) {
          return(FALSE);
     }
     return(pusrsend(userid, from, NULL, text, type, 0, NULL, NULL, NULL));
}
