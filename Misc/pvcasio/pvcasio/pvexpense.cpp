/***************************************************************************
                          pvexpense.cpp  -  description
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

#include "pvexpense.h"
#include "ModeCode.h"
#include "FieldCode.h"
#include <pvdataentryexception.h>


/**
   * Constructor.
   */
PVExpense::PVExpense() {
	m_data[EX_DATE] = "";
	m_data[EX_PAYMENT_TYPE] = "";
	m_data[EX_AMOUNT] = "";
	m_data[EX_EXPENSE_TYPE] = "";
	m_data[EX_NOTE] = "";
}

/**
   * Destructor.
   */
PVExpense::~PVExpense(){
}

/**
   * Getter for the mode code.
   * @return The mode code of the data entry.
   */
unsigned int PVExpense::getModeCode() const{
	return EXPENSE_PV;
}

/**
   * Getter for the data.
   * @return Return all of the data.
   */
const map<unsigned int, string>& PVExpense::getData() const{
	return m_data;
}


string PVExpense::getDate()
{
	return m_data[EX_DATE];
}

string PVExpense::getPaymentType()
{
	return m_data[EX_PAYMENT_TYPE];
}

string PVExpense::getAmount()
{
	return m_data[EX_AMOUNT];
}

string PVExpense::getExpenseType()
{
	return m_data[EX_EXPENSE_TYPE];
}

string PVExpense::getNote()
{
	return m_data[EX_NOTE];
}


void PVExpense::setDate( string& value )
{
	m_data[EX_DATE] = value;
}


void PVExpense::setPaymentType( string& value )
{
	if ( value.length() > 14 ) throw PVDataEntryException("PVExpense::setPaymentType : string longer than 14 characters");
	m_data[EX_PAYMENT_TYPE] = value;
}

void PVExpense::setAmount( string& value )
{
	m_data[EX_AMOUNT] = value;
}

void PVExpense::setExpenseType( string& value )
{
	if ( value.length() > 14 ) throw PVDataEntryException("PVExpense::setExpenseType : string longer than 14 characters");
	m_data[EX_EXPENSE_TYPE] = value;
}

void PVExpense::setNote( string& value )
{
	if ( value.length() > 2008 ) throw PVDataEntryException("PVExpense::setNote : string longer than 2008 characters");
	m_data[EX_NOTE] = value;
}

/**
   * Setter for the data of a field.
   * @param packet data packet for the field
   * @exception PVDataEntryException
   */
void PVExpense::setFieldData( datapacket& packet )
{
	switch( packet.fieldCode )
	{
		case EX_DATE:
		case EX_PAYMENT_TYPE:
		case EX_AMOUNT:
		case EX_EXPENSE_TYPE:
		case EX_NOTE:
			m_data[packet.fieldCode] = packet.data;
			break;
		default:
			throw PVDataEntryException("PVExpense::setFieldData : recieved unsupported fieldCode : " + packet.fieldCode);
	}
}

/**
   * Checks if this instance of expense is sendable.
   * The fields date and amount have to be set.
   * @return bool true if all nessecary fields are filled else false.
   */
bool PVExpense::isSendable(){
	return ( m_data[EX_DATE] != "" && m_data[EX_AMOUNT] != "" );
}

/**
   * stream the content
   */
ostream& operator<< (ostream& out, PVExpense expense)
{
	out << "----- 	PVExpense -----" << endl
		   << "Date\t\t: " << expense.getDate() << endl
		   << "Amount\t\t: " << expense.getAmount() << endl
		   << "ExpenseType\t: " << expense.getExpenseType() << endl
		   << "PaymentType\t: " << expense.getPaymentType() << endl
		   << "Note\t\t: " << expense.getNote() << endl;
	return out;
}
