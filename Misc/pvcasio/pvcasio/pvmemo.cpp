/***************************************************************************
                          pvmemo.cpp  -  description
                             -------------------
    begin                : Mit Jul 10 2002
    copyright            : (C) 2002 by Selzer Michael, Thomas Bonk
    email                : selzer@student.uni-kl.de, thomas@ghecko.saar.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "pvmemo.h"
#include "ModeCode.h"
#include "FieldCode.h"
#include <pvdataentryexception.h>


/**
   * Constructor.
   */
PVMemo::PVMemo(unsigned int modeCode) {
	switch (modeCode) {
		case	MEMO_1:
		case	MEMO_2:
		case	MEMO_3:
		case	MEMO_4:
		case	MEMO_5:
		case	MEMO_6:
					m_modeCode = modeCode;
					break;
		default :
					throw PVDataEntryException("PVMemo::PVMemo : tried to set an unsupported ModeCode : " + modeCode);
	}
	m_continued = false;
	m_data[MEMO] = "";
}

/**
   * Destructor.
   */
PVMemo::~PVMemo(){
}

/**
   * Getter for the mode code.
   * @return The mode code of the data entry.
   */
unsigned int PVMemo::getModeCode() const{
	return m_modeCode;
}

/**
   * Getter for the data.
   * @return Return all of the data.
   */
const map<unsigned int, string>& PVMemo::getData() const{
	return m_data;
}

string PVMemo::getMemo()
{
	return m_data[MEMO];
}


void PVMemo::setMemo( string& value )
{
	if ( value.length() > 2048 ) throw PVDataEntryException("PVMemo::setMemo : string longer than 2048 characters");
	m_data[MEMO] = value;
}

/**
   * Setter for the data of a field.
   * @param packet data packet for the field
   * @exception PVDataEntryException
   */
void PVMemo::setFieldData( datapacket& packet )
{
	if ( packet.fieldCode == MEMO ) {
		if ( m_continued ) m_data[MEMO] += packet.data;
		else m_data[MEMO] = packet.data;
		m_continued =  packet.continued;
	} else {
		throw PVDataEntryException("PVMemo::setFieldData : recieved unsupported fieldCode : " + packet.fieldCode);
	}
}

/**
   * Checks if this instance of memo is sendable.
   * The field memo have to be set.
   * @return bool true if all nessecary fields are filled else false.
   */
bool PVMemo::isSendable(){
	return ( m_data[MEMO] !=  "" );
}

/**
   * stream the content
   */
ostream& operator<< (ostream& out, PVMemo& memo)
{
	out << "----- 	PVMemo -----" << endl
		   << "Memo : " << memo.getMemo() << endl;
	return out;
}
