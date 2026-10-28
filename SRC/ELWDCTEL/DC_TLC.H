/* Build 26.10.27.3 08:21PM */
/*****************************************************************************
 *   DC_TLC.H                      Dice for Teleconference - the link to    *
 *                                 the Teleconference engine (GALTELE.DLL)   *
 *                                                                           *
 *   Copyright (C) 2026 Elwynor Technologies.                                *
 *                                                                           *
 *   The Teleconference engine offers a small C API for add-ons: register a  *
 *   command, and send text to a channel or to one user.  This module finds  *
 *   those functions in GALTELE.DLL by name when it starts,                  *
 *   instead of linking to them, for two reasons:                            *
 *                                                                           *
 *   1. The SDK's import library for them (GALTELE_LIB.LIB) points at the    *
 *      wrong DLL and the wrong export numbers, so a module linked with it   *
 *      will not load on a real board.                                       *
 *   2. If Teleconference is not installed, the module can say so in the     *
 *      Audit Trail and stay out of the way, instead of failing to load.     *
 *                                                                           *
 *   The SDK's full Teleconference header (TLCAPI.H) cannot be used either:  *
 *   one of the files it pulls in (TLCACT.H) does not compile.  So only the  *
 *   structure headers are included here, and the few functions this        *
 *   module calls are declared below, matching TLCAPIC.H exactly.            *
 *                                                                           *
 *   Licensed under the GNU Affero General Public License v3.0.              *
 *****************************************************************************/

#ifndef DC_TLC_H
#define DC_TLC_H

#include "DC_DEFS.H"
#include "tlcchan.h"                 /* channel/user name sizes              */
#include "tlcuser.h"                 /* USRSTRUCT: a Teleconference user     */
#include "tlccmd.h"                  /* CMDPARAM: what a command receives    */
#include "tlcdyncm.h"                /* CMDSTRUCT: how a command registers   */
#include "tlctrans.h"                /* RECV_TYPE_xxx: kinds of text         */

GBOOL dctlc_bind(VOID);              /* find the functions; FALSE if absent  */
const CHAR *dctlc_error(VOID);       /* why dctlc_bind() failed              */
VOID dctlc_init(VOID);               /* make sure Teleconference is set up   */

/* The Teleconference functions this module uses (see TLCAPIC.H).          */

GBOOL dctlc_register(LPCMDSTRUCT cmd);     /* add a Teleconference command   */

/* Send text to everyone in a channel except one user.  Goes through every  */
/* Teleconference transport, including a MajorLink / WorldLink link.        */
GBOOL dctlc_chansendbut(const CHAR *channel, const CHAR *from,
                        const CHAR *text, SHORT type, const CHAR *notuser);

/* Send text to one user through that user's own transport (terminal or    */
/* web).  With RECV_TYPE_PRFBUF the text is whatever is in prfbuf.         */
GBOOL dctlc_usersend(const CHAR *userid, const CHAR *from,
                     const CHAR *text, SHORT type);

#endif /* DC_TLC_H */
