/***************************************************************************
                          pvreminder.cpp  -  description
                             -------------------
    begin                : Mon Jul 22 2002
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

#include "pvreminder.h"
#include "ModeCode.h"
#include "FieldCode.h"
#include <pvdataentryexception.h>


/**
   * Constructor.
   */
PVReminder::PVReminder() {
	m_data[TYPE] = "";
	m_data[DATE] = "";
	m_data[START_TIME] = "";
	m_data[END_TIME] = "";
	m_data[ALARM_TIME] = "";
	m_data[DESCRIPTION] = "";
}

/**
   * Destructor.
   */
PVReminder::~PVReminder(){
}

/**
   * Getter for the mode code.
   * @return The mode code of the data entry.
   */
unsigned int	 PVReminder::getModeCode() const{
	return SCHEDULE_REMINDER;
}

/**
   * Getter for the data.
   * @return Return all of the data.
   */
const map<unsigned int, string>& PVReminder::getData() const{
	return m_data;
}



string PVReminder::getType()
{
	return m_data[TYPE];
}

string PVReminder::getDate()
{
	return m_data[DATE];
}

string PVReminder::getStartTime()
{
	return m_data[START_TIME];
}

string PVReminder::getEndTime()
{
	return m_data[END_TIME];
}

string PVReminder::getAlarmTime()
{
	return m_data[ALARM_TIME];
}

string PVReminder::getDescription()
{
	return m_data[DESCRIPTION];
}


void PVReminder::setType( string& value )
{
	m_data[TYPE] = value;
}

void PVReminder::setDate( string& value )
{
	m_data[DATE] = value;
}

void PVReminder::setStartTime( string& value )
{
	m_data[START_TIME] = value;
}

void PVReminder::setEndTime( string& value )
{
	m_data[END_TIME] = value;
}

void PVReminder::setAlarmTime( string& value )
{
	m_data[ALARM_TIME] = value;
}

void PVReminder::setDescription( string& value )
{
	if ( value.length() > 2046 ) throw PVDataEntryException("PVReminder::setDescription : string longer than 2046 characters");
	m_data[DESCRIPTION] = value;
}

/**
   * Setter for the data of a field.
   * @param packet data packet for the field
   * @exception PVDataEntryException
   */
void PVReminder::setFieldData( datapacket& packet )
{
	switch( packet.fieldCode )
	{
		case TYPE:
		case DATE:
		case START_TIME:
		case END_TIME:
		case ALARM_TIME:
		case DESCRIPTION:
			m_data[packet.fieldCode] = packet.data;
			break;
		default:
			throw PVDataEntryException("PVReminder::setFieldData : recieved unsupported fieldCode : " + packet.fieldCode);
	}
}

/**
   * Checks if this instance of reminder is sendable.
   * The fields date, start time and description have to be set.
   * @return bool true if all nessecary fields are filled else false.
   */
bool PVReminder::isSendable(){
	return ( m_data[DATE] != "" && m_data[START_TIME] != "" && m_data[DESCRIPTION] != "" );
}

/**
   * stream the content
   */
ostream& operator<< (ostream& out, PVReminder& reminder)
{
	out << "----- 	PVReminder -----" << endl
		   << "Type\t\t: " << reminder.getType() << endl
		   << "Date\t\t: " << reminder.getDate() << endl
		   << "AlarmTime\t: " << reminder.getAlarmTime() << endl
		   << "StartTime\t: " << reminder.getStartTime() << endl
		   << "EndTime\t\t: " << reminder.getEndTime() << endl
		   << "Description\t: " << reminder.getDescription() << endl;
	return out;
}

