/* Build 26.10.27.3 08:21PM */
/*****************************************************************************
 *   DC_DEFS.H                     Dice for Teleconference - common header   *
 *                                                                           *
 *   Copyright (C) 2026 Elwynor Technologies.                                *
 *   Originally HOTELeconference Dice by Randall R. Spangler, annex.com.     *
 *                                                                           *
 *   Version numbers and the includes every source file needs.  Keep         *
 *   ELWDCTEL_VER_* in step with ELWDCTEL.rc, CHANGELOG.MD and ELWDCTEL.RLN. *
 *                                                                           *
 *   Licensed under the GNU Affero General Public License v3.0.              *
 *****************************************************************************/

#ifndef DC_DEFS_H
#define DC_DEFS_H

#include "gcomm.h"
#include "majorbbs.h"

#define ELWDCTEL_VER_MAJOR  1
#define ELWDCTEL_VER_MINOR  0
#define ELWDCTEL_VER_PATCH  0
#define ELWDCTEL_VERSION    "1.0.0"

#define DC_APPNAME   "Dice for Teleconference"
#define DC_CMDDICE   "DICE"          /* fixed: Teleconference can't drop one */
#define DC_CMDROLL   "ROLL"          /* fixed, for the same reason           */

#define DC_DEFSIDES  6               /* a bare "ROLL 3" uses d6              */
#define DC_LISTSIZ   160             /* longest "4, 2 and 6" list shown; more*/
                                     /* dice than fit show only the total.   */
#define DC_PUBSIZ    240             /* longest line sent to the channel.    */
                                     /* Teleconference lines are at most 255 */
                                     /* (GALTELE.MSG TINPSZ).                */

#endif /* DC_DEFS_H */
