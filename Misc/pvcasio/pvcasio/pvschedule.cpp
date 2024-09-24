/***************************************************************************
                          pvschedule.cpp  -  description
                             -------------------
    begin                : Don Jul 11 2002
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

#include "pvschedule.h"
#include "ModeCode.h"
#include "FieldCode.h"
#include <pvdataentryexception.h>


/**
   * Constructor.
   */
PVSchedule::PVSchedule() {
	m_data[DATE] = "";
	m_data[START_TIME] = "";
	m_data[END_TIME] = "";
	m_data[ALARM_TIME] = "";
	m_data[DESCRIPTION] = "";
}

/**
   * Destructor.
   */
PVSchedule::~PVSchedule(){
}

/**
   * Getter for the mode code.
   * @return The mode code of the data entry.
   */
unsigned int	 PVSchedule::getModeCode() const{
	return SCHEDULE;
}

/**
   * Getter for the data.
   * @return Return all of the data.
   */
const map<unsigned int, string>& PVSchedule::getData() const{
	return m_data;
}



string PVSchedule::getDate()
{
	return m_data[DATE];
}

string PVSchedule::getStartTime()
{
	return m_data[START_TIME];
}

string PVSchedule::getEndTime()
{
	return m_data[END_TIME];
}

string PVSchedule::getAlarmTime()
{
	return m_data[ALARM_TIME];
}

string PVSchedule::getDescription()
{
	return m_data[DESCRIPTION];
}


void PVSchedule::setDate( string& value )
{
	m_data[DATE] = value;
}

void PVSchedule::setStartTime( string& value )
{
	m_data[START_TIME] = value;
}

void PVSchedule::setEndTime( string& value )
{
	m_data[END_TIME] = value;
}

void PVSchedule::setAlarmTime( string& value )
{
	m_data[ALARM_TIME] = value;
}

void PVSchedule::setDescription( string& value )
{
	if ( value.length() > 2046 ) throw PVDataEntryException("PVSchedule::setDescription : string longer than 2046 characters");
	m_data[DESCRIPTION] = value;
}

/**
   * Setter for the data of a field.
   * @param packet data packet for the field
   * @exception PVDataEntryException
   */
void PVSchedule::setFieldData( datapacket& packet )
{
	switch( packet.fieldCode )
	{
		case DATE:
		case START_TIME:
		case END_TIME:
		case ALARM_TIME:
		case DESCRIPTION:
			m_data[packet.fieldCode] = packet.data;
			break;
		default:
			throw PVDataEntryException("PVSchedule::setFieldData : recieved unsupported fieldCode : " + packet.fieldCode);
	}
}

/**
   * Checks if this instance of schedule is sendable.
   * The fields date, start time and description have to be set.
   * @return bool true if all nessecary fields are filled else false.
   */
bool PVSchedule::isSendable(){
	return ( m_data[DATE] != "" && m_data[START_TIME] != "" && m_data[DESCRIPTION] != "" );
}

/**
   * stream the content
   */
ostream& operator<< (ostream& out, PVSchedule& schedule)
{
	out << "----- 	PVSchedule -----" << endl
		   << "Date\t\t: " << schedule.getDate() << endl
		   << "AlarmTime\t: " << schedule.getAlarmTime() << endl
		   << "StartTime\t: " << schedule.getStartTime() << endl
		   << "EndTime\t\t: " << schedule.getEndTime() << endl
		   << "Description\t: " << schedule.getDescription() << endl;
	return out;
}

