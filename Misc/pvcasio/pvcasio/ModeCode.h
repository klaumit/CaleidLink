/***************************************************************************
                          ModeCode.h  -  description
                             -------------------
    begin                : Tue Feb 19 2002
    copyright            : (C) 2002 by Selzer Michael
    email                : selzer@student.uni-kl.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#define MEMO_1									0x000000
#define MEMO_2									0x001000
#define MEMO_3									0x002000
#define MEMO_4									0x003000
#define MEMO_5									0x004000
//?????????????????????????????????????????? Gibts den????????????????????????????
#define MEMO_6									0x005000

#define QUICK_MEMO							0x010100
#define CONTACT_PRIVATE				0x100000
#define CONTACT_BUSINESS_1		0x110000
#define CONTACT_BUSINESS_2		0x102000
#define CONTACT_BUSINESS_3		0x103000
#define CONTACT_BUSINESS_4		0x104000
// only for PV-750
#define CONTACT_BUSINESS_5		0x105000
#define CONTACT_BUSINESS_6		0x106000

#define SCHEDULE									0x200000
#define SCHEDULE_MULTI_DATE	0x200800
#define SCHEDULE_REMINDER			0x201000
#define TODO											0x220000
#define EXPENSE_BN							0x230000
#define EXPENSE_PV							0x231000

#define MAIL_PROVIDER_1_RE		0x500000
#define MAIL_PROVIDER_2_RE		0x501000
#define MAIL_PROVIDER_3_RE		0x502000
#define MAIL_PROVIDER_1_SE		0x504000
#define MAIL_PROVIDER_2_SE		0x505000
#define MAIL_PROVIDER_3_SE		0x506000
#define MAIL_PROVIDER_1_SET		0x508000
#define MAIL_PROVIDER_2_SET		0x509000
#define MAIL_PROVIDER_3_SET		0x50A000

#define SMS_RECEIVE							0x50B000
#define SMS_SEND								0x50C000
#define SMS_SETTING							0x50D000

#define SPREAD_SHEET_BN				0x600000
#define POCKET_SHEET_PV				0x610000


#define ADDIN											0x700000

#define FULL_BACKUP							0xF00000
/*
#define CAT_PHONE_PERSON_Q	"100100"
#define CAT_PHONE_COMPANY_Q	"110100"
#define CAT_PHONE_SHOP		"101000"
#define CAT_PHONE_SHOP_Q	"101100"
#define CAT_PHONE_OTHERS	"102000"
#define CAT_PHONE_OTHERS_Q	"102100"
#define CAT_NOTE_MEMO_Q		"000100"
#define CAT_NOTE_VIDEO		"007000"
#define CAT_NOTE_INSURANCE	"008000"
#define CAT_NOTE_MINUTES	"009000"
#define CAT_NOTE_INFO		"00A000"
#define CAT_NOTE_BUSINESSTRIP	"00B000"
#define CAT_NOTE_REPORT		"00C000"
#define CAT_NOTE_CHECKLIST	"00D000"
#define CAT_NOTE_FAX		"00E000"
#define CAT_NOTE_DATA		"00F000"
#define CAT_SCHEDULE_Q		"200100"
#define CAT_PLAN		"200800"
#define CAT_EVENT		"201000"
#define CAT_TODO_Q		"220100"
#define CAT_CALENDAR		"300000"
#define CAT_CALENDAR_Q		"320000"
*/
