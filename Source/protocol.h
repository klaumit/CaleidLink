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
 * コマンド一覧
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
 * データタイプ一覧
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
 * カテゴリー一覧
 */

#define CAT_PHONE_PERSON	"100000" /* 電話帳・個人 */
#define CAT_PHONE_PERSON_Q	"100100" /* 電話帳・個人・クイックフォーム */
#define CAT_PHONE_COMPANY	"110000" /* 電話帳・会社 */
#define CAT_PHONE_COMPANY_Q	"110100" /* 電話帳・会社・クイックフォーム */
#define CAT_PHONE_SHOP		"101000" /* 電話帳・店名 */
#define CAT_PHONE_SHOP_Q	"101100" /* 電話帳・店名・クイックフォーム */
#define CAT_PHONE_OTHERS	"102000" /* 電話帳・その他 */
#define CAT_PHONE_OTHERS_Q	"102100" /* 電話帳・その他・クイックフォーム */
#define CAT_NOTE_MEMO		"000000" /* メモ帳・メモ */
#define CAT_NOTE_MEMO_Q		"000100" /* メモ帳・メモ・クイックフォーム */
#define CAT_NOTE_TRAIN		"001000" /* メモ帳・時刻表 */
#define CAT_NOTE_FAMILY		"002000" /* メモ帳・家族 */
#define CAT_NOTE_CARD		"003000" /* メモ帳・カード */
#define CAT_NOTE_CAR		"004000" /* メモ帳・車 */
#define CAT_NOTE_BOOK		"005000" /* メモ帳・BOOK */
#define CAT_NOTE_CD		"006000" /* メモ帳・CD */
#define CAT_NOTE_VIDEO		"007000" /* メモ帳・映画&ビデオ */
#define CAT_NOTE_INSURANCE	"008000" /* メモ帳・保険 */
#define CAT_NOTE_MINUTES	"009000" /* メモ帳・議事録 */
#define CAT_NOTE_INFO		"00A000" /* メモ帳・連絡書 */
#define CAT_NOTE_BUSINESSTRIP	"00B000" /* メモ帳・出張報告 */
#define CAT_NOTE_REPORT		"00C000" /* メモ帳・日報 */
#define CAT_NOTE_CHECKLIST	"00D000" /* メモ帳・チェックリスト */
#define CAT_NOTE_FAX		"00E000" /* メモ帳・FAX 送付状 */
#define CAT_NOTE_DATA		"00F000" /* メモ帳・クイックデータコピー */
#define CAT_SCHEDULE		"200000" /* スケジュール*/
#define CAT_SCHEDULE_Q		"200100" /* スケジュール・クイックフォーム */
#define CAT_PLAN		"200800" /* 期間スケジュール */
#define CAT_EVENT		"201000" /* 記念日 */
#define CAT_TODO		"220000" /* ToDo */
#define CAT_TODO_Q		"220100" /* ToDo・クイックフォーム */
#define CAT_SHEET		"600000" /* イージーシート */
#define CAT_ADDIN		"700000" /* アドイン */
#define CAT_CALENDAR		"300000" /* 休日設定・うまくカタログを受信できません. */
#define CAT_CALENDAR_Q		"320000" /* 手書きカレンダー. 手書データの断片と何?? */
#define CAT_BACKUP		"F00000" /* バックアップ・データ */


/*
 * 新規のセッション ID
 */

#define INIT_SESSION_ID "\xa1\xe1" "00                                    "

#endif /* __PROTOCOL_H__ */
