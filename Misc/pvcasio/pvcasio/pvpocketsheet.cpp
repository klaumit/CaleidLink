/***************************************************************************
                          pvpocketsheet.cpp  -  description
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

#include "pvpocketsheet.h"
#include "ModeCode.h"
#include "FieldCode.h"
#include <pvdataentryexception.h>


PVPocketSheet::PVPocketSheet() {
	m_data[PS_SHEET_DATA] = "";
	m_data[PS_X_LINE_DATA] = "";
	m_data[PS_Y_LINE_DATA] = "";
	m_data[PS_CELL_DATA] = "";
}

/**
   * Destructor.
   */
PVPocketSheet::~PVPocketSheet(){
}

/**
   * Getter for the mode code.
   * @return The mode code of the data entry.
   */
unsigned int	 PVPocketSheet::getModeCode() const{
	return POCKET_SHEET_PV;
}

/**
   * Getter for the data.
   * @return Return all of the data.
   */
const map<unsigned int, string>& PVPocketSheet::getData() const{
	return m_data;
}


string PVPocketSheet::getSheetData()
{
	return m_data[PS_SHEET_DATA];
}

string PVPocketSheet::getXLineData()
{
	return m_data[PS_X_LINE_DATA];
}

string PVPocketSheet::getYLineData()
{
	return m_data[PS_Y_LINE_DATA];
}

string PVPocketSheet::getCellData()
{
	return m_data[PS_CELL_DATA];
}


void PVPocketSheet::setSheetData( string& value )
{
	m_data[PS_SHEET_DATA] = value;
}

void PVPocketSheet::setXLineData( string& value )
{
	m_data[PS_X_LINE_DATA] = value;
}

void PVPocketSheet::setYLineData( string& value )
{
	m_data[PS_Y_LINE_DATA] = value;
}

void PVPocketSheet::setCellData( string& value )
{
	m_data[PS_CELL_DATA] = value;
}


/**
   * Setter for the data of a field.
   * @param packet data packet for the field
   * @exception PVDataEntryException
   */
void PVPocketSheet::setFieldData( datapacket& packet )
{
	switch( packet.fieldCode )
	{
		case PS_SHEET_DATA:
		case PS_X_LINE_DATA:
		case PS_Y_LINE_DATA:
		case PS_CELL_DATA:
			m_data[packet.fieldCode] = packet.data;
			break;
		default:
			throw PVDataEntryException("PVPocketSheet::setFieldData : recieved unsupported fieldCode : " + packet.fieldCode);
	}
}

/**
   * Checks if this instance of  pocketsheet is sendable.
   * The fields xxxxxxxxxxxxxx have to be set.
   * @return bool true if all nessecary fields are filled else false.
   */
bool PVPocketSheet::isSendable(){
	return true;
}

/**
   * stream the content
   */
ostream& operator<< (ostream& out, PVPocketSheet& pocketsheet)
{
	out << "----- 	PVPocketSheet -----" << endl
		   << "XLineData\t: " << pocketsheet.getXLineData() << endl
		   << "YLineData\t: " << pocketsheet.getYLineData() << endl
		   << "CellData\t: " << pocketsheet.getCellData() << endl
		   << "SheetData\t: " << pocketsheet.getSheetData() << endl;
	return out;
}

