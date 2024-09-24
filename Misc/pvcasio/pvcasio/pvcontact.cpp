/***************************************************************************
                          pvcontact.cpp  -  Implementation of the
                                                   class for a contact.
                             -------------------
    begin                : Sat Mar 16 2002
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

//project includes
#include "pvcontact.h"
#include "ModeCode.h"
#include "FieldCode.h"
#include <pvdataentryexception.h>


/**
   * Constructor.
   */
PVContact::PVContact(unsigned int modeCode) {
	switch (modeCode) {
		case	CONTACT_PRIVATE:
		case	CONTACT_BUSINESS_1:
		case	CONTACT_BUSINESS_2:
		case	CONTACT_BUSINESS_3:
		case	CONTACT_BUSINESS_4:
// only for PV-750
		case	CONTACT_BUSINESS_5:
		case	CONTACT_BUSINESS_6:
					m_modeCode = modeCode;
					break;
		default :
					throw PVDataEntryException("PVContact::PVContact : tried to set an unsupported ModeCode : " + modeCode);
	}
	m_data[NAME] = "";
	m_data[HOME_NUMBER] = "";
	m_data[BUSINESS_NUMBER] = "";
	m_data[FAX_NUMBER] = "";
	m_data[BUSINESS_FAX] = "";
	m_data[MOBILE] = "";
	m_data[ADDRESS] = "";
	m_data[EMAIL] = "";
	m_data[EMPLOYER] = "";
	m_data[BUSINESS_ADDRESS] = "";
	m_data[DEPARTMENT] = "";
	m_data[POSITION] = "";
	m_data[NOTE] = "";
}


/**
   * Destructor.
   */
PVContact::~PVContact(){
}

/**
   * Getter for the mode code.
   * @return The mode code of the data entry.
   */
unsigned int PVContact::getModeCode() const
{
	return m_modeCode;
}


string PVContact::getName()
{
	return m_data[NAME];
}

string PVContact::getHomeNumber()
{
	return m_data[HOME_NUMBER];
}


string PVContact::getBusinessNumber()
{
	return m_data[BUSINESS_NUMBER];
}


string PVContact::getFaxNumber()
{
	return m_data[FAX_NUMBER];
}


string PVContact::getBusinessFax()
{
	return m_data[BUSINESS_FAX];
}


string PVContact::getMobile()
{
	return m_data[MOBILE];
}


string PVContact::getAddress()
{
	return m_data[ADDRESS];
}


string PVContact::getEmail()
{
	return m_data[EMAIL];
}

string PVContact::getEmployer()
{
	return m_data[EMPLOYER];
}


string PVContact::getBusinessAddress()
{
	return m_data[BUSINESS_ADDRESS];
}


string PVContact::getDepartment()
{
	return m_data[DEPARTMENT];
}


string PVContact::getPosition()
{
	return m_data[POSITION];
}


string PVContact::getNote()
{
	return m_data[NOTE];
}


void PVContact::setName( string& value )
{
	m_data[NAME] = value;
}


void PVContact::setHomeNumber( string& value )
{
	m_data[HOME_NUMBER] = value;
}


void PVContact::setBusinessNumber( string& value )
{
	m_data[BUSINESS_NUMBER] = value;
}

void PVContact::setFaxNumber( string& value )
{
	m_data[FAX_NUMBER] = value;
}


void PVContact::setBusinessFax( string& value )
{
	m_data[BUSINESS_FAX] = value;
}


void PVContact::setMobile( string& value )
{
	m_data[MOBILE] = value;
}


void PVContact::setAddress( string& value )
{
	if ( value.length() > 2036 ) throw PVDataEntryException("PVContact::setAddress : string longer than 2036 characters");
	m_data[ADDRESS] = value;
}


void PVContact::setEmail( string& value )
{
	if ( value.length() > 2036 ) throw PVDataEntryException("PVContact::setEmail : string longer than 2036 characters");
	m_data[EMAIL] = value;
}

void PVContact::setEmployer( string& value )
{
	if ( value.length() > 2036 ) throw PVDataEntryException("PVContact::setEmployer : string longer than 2036 characters");
	m_data[EMPLOYER] = value;
}


void PVContact::setBusinessAddress( string& value )
{
	if ( value.length() > 2036 ) throw PVDataEntryException("PVContact::setBusinessAddress : string longer than 2036 characters");
	m_data[BUSINESS_ADDRESS] = value;
}


void PVContact::setDepartment( string& value )
{
	if ( value.length() > 2036 ) throw PVDataEntryException("PVContact::setDepartment : string longer than 2036 characters");
	m_data[DEPARTMENT] = value;
}


void PVContact::setPosition( string& value )
{
	if ( value.length() > 2036 ) throw PVDataEntryException("PVContact::setPosition : string longer than 2036 characters");
	m_data[POSITION] = value;
}


void PVContact::setNote( string& value )
{
	if ( value.length() > 2036 ) throw PVDataEntryException("PVContact::setNote : string longer than 2036 characters");
	m_data[NOTE] = value;
}



/**
   * Getter for the data.
   * @return Return all of the data.
   */
const map<unsigned int, string>& PVContact::getData() const
{
	return m_data;
}



/**
   * Setter for the data of a field.
   * @param packet data packet for the field
   * @exception PVDataEntryException
   */
void PVContact::setFieldData( datapacket& packet )
{
	switch( packet.fieldCode )
	{
		case NAME:
		case HOME_NUMBER:
		case BUSINESS_NUMBER:
		case FAX_NUMBER:
		case BUSINESS_FAX:
		case MOBILE:
		case ADDRESS:
		case EMAIL:
		case EMPLOYER:
		case BUSINESS_ADDRESS:
		case DEPARTMENT:
		case POSITION:
		case NOTE:
			m_data[packet.fieldCode] = packet.data;
			break;
		default:
			throw PVDataEntryException("PVContact::setFieldData : recieved unsupported fieldCode : " + packet.fieldCode);
	}
}

/**
   * Checks if this instance of reminder is sendable.
   * The field name have to be set if this have the modecode private contact.
   * If this have the modecode for a business contacte the field employer have to be set, too.
   * @return bool true if all nessecary fields are filled else false.
   */
bool PVContact::isSendable(){
	bool rc = false;
	if ( m_modeCode == CONTACT_PRIVATE ) {
		rc = ( m_data[NAME] != "" );
	} else {
		rc = ( m_data[NAME] != "" && m_data[EMPLOYER] != "" );
	}
	return rc;
}

/**
   * stream the content
   */
ostream& operator<< (ostream& out, PVContact& contact)
{
	out << "----- 	PVContact -----" << endl
		   << "Name\t\t: " << contact.getName() << endl
		   << "HomeNumber\t: " << contact.getHomeNumber() << endl
		   << "BusinessNumber\t: " << contact.getBusinessNumber() << endl
		   << "FaxNumber\t: " << contact.getFaxNumber() << endl
		   << "BusinessFax\t: " << contact.getBusinessFax() << endl
		   << "Mobile\t\t: " << contact.getMobile() << endl
		   << "Address\t\t: " << contact.getAddress() << endl
		   << "Email\t\t: " << contact.getEmail() << endl
		   << "Employer\t: " << contact.getEmployer() << endl
		   << "BusinessAddress\t: " << contact.getBusinessAddress() << endl
		   << "Department\t: " << contact.getDepartment() << endl
		   << "Position\t: " << contact.getPosition() << endl
		   << "Note\t\t: " << contact.getNote() << endl;
	return out;
}
