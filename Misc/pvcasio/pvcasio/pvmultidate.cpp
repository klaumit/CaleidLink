/***************************************************************************
                          pvmultidate.cpp  -  description
                             -------------------
    begin                : Mon Jul 22 2002
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

#include "pvmultidate.h"
#include "ModeCode.h"
#include "FieldCode.h"
#include <pvdataentryexception.h>


/**
   * Constructor.
   */
PVMultiDate::PVMultiDate() {
	m_data[DATE] = "";
	m_data[END_DATE] = "";
	m_data[DESCRIPTION] = "";
}

/**
   * Destructor.
   */
PVMultiDate::~PVMultiDate(){
}

/**
   * Getter for the mode code.
   * @return The mode code of the data entry.
   */
unsigned int	 PVMultiDate::getModeCode() const{
	return SCHEDULE_MULTI_DATE;
}

/**
   * Getter for the data.
   * @return Return all of the data.
   */
const map<unsigned int, string>& PVMultiDate::getData() const{
	return m_data;
}



string PVMultiDate::getDate()
{
	return m_data[DATE];
}

string PVMultiDate::getEndDate()
{
	return m_data[END_DATE];
}

string PVMultiDate::getDescription()
{
	return m_data[DESCRIPTION];
}



void PVMultiDate::setDate( string& value )
{
	m_data[DATE] = value;
}

void PVMultiDate::setEndDate( string& value )
{
	m_data[END_DATE] = value;
}

void PVMultiDate::setDescription( string& value )
{
	if ( value.length() > 2046 ) throw PVDataEntryException("PVMultiDate::setDescription : string longer than 2046 characters");
	m_data[DESCRIPTION] = value;
}

/**
   * Setter for the data of a field.
   * @param packet data packet for the field
   * @exception PVDataEntryException
   */
void PVMultiDate::setFieldData( datapacket& packet )
{
	switch( packet.fieldCode )
	{
		case DATE:
		case END_DATE:
		case DESCRIPTION:
			m_data[packet.fieldCode] = packet.data;
			break;
		default:
			throw PVDataEntryException("PVMultiDate::setFieldData : recieved unsupported fieldCode : " + packet.fieldCode);
	}
}

/**
   * Checks if this instance of mulitdate is sendable.
   * The fields date and description have to be set.
   * @return bool true if all nessecary fields are filled else false.
   */
bool PVMultiDate::isSendable(){
	return ( m_data[DATE] != "" && m_data[DESCRIPTION] != "" );
}

/**
   * stream the content
   */
ostream& operator<< (ostream& out, PVMultiDate& multidate)
{
	out << "----- 	PVReminder -----" << endl
		   << "Date\t\t: " << multidate.getDate() << endl
		   << "EndDate\t\t: " << multidate.getEndDate() << endl
		   << "Description\t: " << multidate.getDescription() << endl;
	return out;
}
