/***************************************************************************
                          pvtodo.cpp  -  description
                             -------------------
    begin                : Don Jul 11 2002
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

#include "pvtodo.h"
#include "ModeCode.h"
#include "FieldCode.h"
#include <pvdataentryexception.h>


/**
   * Constructor.
   */
PVTodo::PVTodo() {
	m_data[CHECK] = "";
	m_data[DUE_DATE] = "";
	m_data[ALARM_DATE] = "";
	m_data[ALARM_TIME] = "";
	m_data[CHECK_DATE] = "";
	m_data[PRIORITY] = "";
	m_data[CATEGORY] = "";
	m_data[DESCRIPTION] = "";
}

/**
   * Destructor.
   */
PVTodo::~PVTodo(){
}

/**
   * Getter for the mode code.
   * @return The mode code of the data entry.
   */
unsigned int	 PVTodo::getModeCode() const{
	return TODO;
}

/**
   * Getter for the data.
   * @return Return all of the data.
   */
const map<unsigned int, string>& PVTodo::getData() const{
	return m_data;
}


string PVTodo::getCheck()
{
	return m_data[CHECK];
}

string PVTodo::getDueDate()
{
	return m_data[DUE_DATE];
}

string PVTodo::getAlarmDate()
{
	return m_data[ALARM_DATE];
}

string PVTodo::getAlarmTime()
{
	return m_data[ALARM_TIME];
}

string PVTodo::getCheckDate()
{
	return m_data[CHECK_DATE];
}

string PVTodo::getPriority()
{
	return m_data[PRIORITY];
}

string PVTodo::getCategory()
{
	return m_data[CATEGORY];
}

string PVTodo::getDescription()
{
	return m_data[DESCRIPTION];
}


void PVTodo::setCheck( string& value )
{
	m_data[CHECK] = value;
}

void PVTodo::setDueDate( string& value )
{
	m_data[DUE_DATE] = value;
}

void PVTodo::setAlarmDate( string& value )
{
	m_data[ALARM_DATE] = value;
}

void PVTodo::setAlarmTime( string& value )
{
	m_data[ALARM_TIME] = value;
}

void PVTodo::setCheckDate( string& value )
{
	m_data[CHECK_DATE] = value;
}

void PVTodo::setPriority( string& value )
{
	m_data[PRIORITY] = value;
}

void PVTodo::setCategory( string& value )
{
	m_data[CATEGORY] = value;
}

void PVTodo::setDescription( string& value )
{
	if ( value.length() > 2046 ) throw PVDataEntryException("PVTodo::setDescription : string longer than 2046 characters");
	m_data[DESCRIPTION] = value;
}


/**
   * Setter for the data of a field.
   * @param packet data packet for the field
   * @exception PVDataEntryException
   */
void PVTodo::setFieldData( datapacket& packet )
{
	switch( packet.fieldCode )
	{
		case CHECK:
		case DUE_DATE:
		case ALARM_DATE:
		case ALARM_TIME:
		case CHECK_DATE:
		case PRIORITY:
		case CATEGORY:
		case DESCRIPTION:
			m_data[packet.fieldCode] = packet.data;
			break;
		default:
			throw PVDataEntryException("PVTodo::setFieldData : recieved unsupported fieldCode : " + packet.fieldCode);
	}
}

/**
   * Checks if this instance of todo is sendable.
   * The fields due date and description have to be set.
   * @return bool true if all nessecary fields are filled. false else
   */
bool PVTodo::isSendable(){
	return ( m_data[DUE_DATE] != "" && m_data[DESCRIPTION] != "" );
}

/**
   * stream the content
   */
ostream& operator<< (ostream& out, PVTodo& todo)
{
	out << "----- 	PVTodo -----" << endl
		   << "AlarmDate\t: " << todo.getAlarmDate() << endl
		   << "AlarmTime\t: " << todo.getAlarmTime() << endl
		   << "Category\t: " << todo.getCategory() << endl
		   << "Check\t\t: " << todo.getCheck() << endl
		   << "CheckDate\t: " << todo.getCheckDate() << endl
		   << "DueDate\t\t: " << todo.getDueDate() << endl
		   << "Priority\t: " << todo.getPriority() << endl
		   << "Description\t: " << todo.getDescription() << endl;
	return out;
}
