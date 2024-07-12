/*
 * protocol.h
 * All rights reserved. Copyright (C) 1998,1999 by NARITA Tomio.
 * $Id: protocol.h,v 1.9 2003/11/17 03:53:19 nrt Exp $
 */

#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#define CALEID_ID				"10002015"
#define CALEID_ID750                            "20000015"
#define CALEID_ID1600				"10002023"

/*
 * $B%3%^%s%I0lMw(B
 */

#define COM_CALEID_PDA_ID			"8110"
#define COM_CALEID_HOST_ID			"8910"

#define COM_PDA_SYNC_ID				"8111"
#define COM_HOST_SYNC_ID			"8911"

#define COM_AVAILABLE_SPEED			"8112"
#define COM_SELECT_SPEED			"8912"

#define COM_CLOSE_PREAMBLE			"8100"
#define COM_START_SESSION			"0900"


#define COM_DESIGNATE_NEW_OBJECT_FOR_SEND	"0010"
#define COM_END_OBJECT_FOR_SEND			"8201"
#define COM_STORED_NEW_OBJECT			"8810"

#define COM_DESIGNATE_OBJECT_FOR_SEND		"0011"
#define COM_STORED_OBJECT			"8811"

#define COM_DESIGNATE_OBJECT_FOR_REMOVE		"8020"
#define COM_REMOVED_OBJECT			"8820"

#define COM_DESIGNATE_OBJECT			"8030"
#define COM_END_OBJECT				"8830"

#define COM_DESIGNATE_NEW_OBJECT		"8031"
#define COM_END_NEW_OBJECT			"8831"

#define COM_DESIGNATE_OBJECT_FOR_GET		"8032"
#define COM_END_OBJECT_FOR_GET			"8832"


#define COM_DESIGNATE_CATEGORY			"8040"
#define COM_END_CATEGORY			"8840"

#define COM_DESIGNATE_CATEGORY_FOR_GET		"8041"
#define COM_END_CATEGORY_FOR_GET		"8841"

#define COM_DESIGNATE_CATEGORY_FOR_PUT		"8021"
#define COM_STANDBY_CATEGORY_FOR_PUT		"8821"


#define COM_START_DATA				"0200"
#define COM_END_DATA				"0201"
#define COM_DATA_LAP				"0202"

#define COM_SYNC_ID				"801F"
#define COM_STORED_SYNC_ID			"881F"

/*
 * $B%G!<%?%?%$%W0lMw(B
 */

#define DAT_CATALOG				"208001"
#define DAT_CATALOG_LAST			"208000"

#define DAT_COUNT				"208010"

#define DAT_PLAIN				"000001"
#define DAT_PLAIN_LAST				"000000"

#define DAT_BACKUP				"100001"
#define DAT_BACKUP_LAST				"100000"

#define DAT_ADDIN				"101001"
#define DAT_ADDIN_LAST				"101000"

#define DAT_QFORM				"102001"
#define DAT_QFORM_LAST				"102000"

#define DAT_BEGIN_DATE				"900000"
#define DAT_END_DATE				"900010"
#define DAT_EXEC_DATE				"900100"
#define DAT_ALARM_DATE				"901000"

#define DAT_BEGIN_TIME				"A00000"
#define DAT_END_TIME				"A00010"
#define DAT_ALARM_TIME				"A01000"

#define NEW_OBJECT_ID				"FFFFFF"

/*
 * $B%+%F%4%j!<0lMw(B
 */

#define CAT_PHONE_PERSON	"100000" /* $BEEOCD"!&8D?M(B */
#define CAT_PHONE_PERSON_Q	"100100" /* $BEEOCD"!&8D?M!&%/%$%C%/%U%)!<%`(B */
#define CAT_PHONE_COMPANY	"110000" /* $BEEOCD"!&2q<R(B */
#define CAT_PHONE_COMPANY_Q	"110100" /* $BEEOCD"!&2q<R!&%/%$%C%/%U%)!<%`(B */
#define CAT_PHONE_SHOP		"101000" /* $BEEOCD"!&E9L>(B */
#define CAT_PHONE_SHOP_Q	"101100" /* $BEEOCD"!&E9L>!&%/%$%C%/%U%)!<%`(B */
#define CAT_PHONE_OTHERS	"102000" /* $BEEOCD"!&$=$NB>(B */
#define CAT_PHONE_OTHERS_Q	"102100" /* $BEEOCD"!&$=$NB>!&%/%$%C%/%U%)!<%`(B */
#define CAT_NOTE_MEMO		"000000" /* $B%a%bD"!&%a%b(B */
#define CAT_NOTE_MEMO_Q		"000100" /* $B%a%bD"!&%a%b!&%/%$%C%/%U%)!<%`(B */
#define CAT_NOTE_TRAIN		"001000" /* $B%a%bD"!&;~9oI=(B */
#define CAT_NOTE_FAMILY		"002000" /* $B%a%bD"!&2HB2(B */
#define CAT_NOTE_CARD		"003000" /* $B%a%bD"!&%+!<%I(B */
#define CAT_NOTE_CAR		"004000" /* $B%a%bD"!&<V(B */
#define CAT_NOTE_BOOK		"005000" /* $B%a%bD"!&(BBOOK */
#define CAT_NOTE_CD		"006000" /* $B%a%bD"!&(BCD */
#define CAT_NOTE_VIDEO		"007000" /* $B%a%bD"!&1G2h(B&$B%S%G%*(B */
#define CAT_NOTE_INSURANCE	"008000" /* $B%a%bD"!&J]81(B */
#define CAT_NOTE_MINUTES	"009000" /* $B%a%bD"!&5D;vO?(B */
#define CAT_NOTE_INFO		"00A000" /* $B%a%bD"!&O"Mm=q(B */
#define CAT_NOTE_BUSINESSTRIP	"00B000" /* $B%a%bD"!&=PD%Js9p(B */
#define CAT_NOTE_REPORT		"00C000" /* $B%a%bD"!&F|Js(B */
#define CAT_NOTE_CHECKLIST	"00D000" /* $B%a%bD"!&%A%'%C%/%j%9%H(B */
#define CAT_NOTE_FAX		"00E000" /* $B%a%bD"!&(BFAX $BAwIU>u(B */
#define CAT_NOTE_DATA		"00F000" /* $B%a%bD"!&%/%$%C%/%G!<%?%3%T!<(B */
#define CAT_SCHEDULE		"200000" /* $B%9%1%8%e!<%k(B*/
#define CAT_SCHEDULE_Q		"200100" /* $B%9%1%8%e!<%k!&%/%$%C%/%U%)!<%`(B */
#define CAT_PLAN		"200800" /* $B4|4V%9%1%8%e!<%k(B */
#define CAT_EVENT		"201000" /* $B5-G0F|(B */
#define CAT_TODO		"220000" /* ToDo */
#define CAT_TODO_Q		"220100" /* ToDo$B!&%/%$%C%/%U%)!<%`(B */
#define CAT_SHEET		"600000" /* $B%$!<%8!<%7!<%H(B */
#define CAT_ADDIN		"700000" /* $B%"%I%$%s(B */
#define CAT_CALENDAR		"300000" /* $B5YF|@_Dj!&$&$^$/%+%?%m%0$r<u?.$G$-$^$;$s(B. */
#define CAT_CALENDAR_Q		"320000" /* $B<j=q$-%+%l%s%@!<(B. $B<j=q%G!<%?$NCGJR$H2?(B?? */
#define CAT_BACKUP		"F00000" /* $B%P%C%/%"%C%W!&%G!<%?(B */


/*
 * $B?75,$N%;%C%7%g%s(B ID
 */

#define INIT_SESSION_ID "\xa1\xe1" "00                                    "

#endif /* __PROTOCOL_H__ */
