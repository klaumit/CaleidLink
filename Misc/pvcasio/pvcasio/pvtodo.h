/***************************************************************************
                          pvtodo.h  -  description
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

#ifndef PVTODO_H
#define PVTODO_H

// project includes
#include <pvcasio/pvdataentry.h>

/**
  *@author Selzer Michael
  */

class PVTodo : public PVDataEntry  {
public: 
		/**
		   * Constructor.
		   */
		PVTodo();

		/**
		   * Destructor.
		   */
		virtual ~PVTodo();

		/**
		   * Getter for the mode code.
		   * @return The mode code of the data entry.
		   */
		virtual unsigned int	 getModeCode() const;

		/**
		   * Getter for the data.
		   * @return Return all of the data.
		   */
		virtual const map<unsigned int, string>& getData() const;

		virtual string getCheck();

		/**
		   * Getter for due date.
		   * @return Return the due date in a string with the format yyyymmdd
		   */
		virtual string getDueDate();

		/**
		   * Getter for alarm date.
		   * @return Return the alarm date in a string with the format yyyymmdd
		   */
		virtual string getAlarmDate();

		/**
		   * Getter for alarm time.
		   * @return Return the alarm time in a string with 4 bytes in the format hhdd. So every value is between 0-9 (0x30-0x39)
		   * hours 00-23
		   * minutes 00-59
		   */
		virtual string getAlarmTime();
		virtual string getCheckDate();

		/**
		   * Getter for the priority.
		   * @return Return one byte where 0 stands for A, 1:B, 2:C
		   */
		virtual string getPriority();

		/**
		   * Getter for the category.
		   * @return Return one byte where 0 stands for category  A, 1:B, 2:C, 3:D, 4:E
		   */
		virtual string getCategory();

		/**
		   * Getter for description.
		   * @return Return the description in a string with up to 2046 characters
		   */
		virtual string getDescription();




		virtual void setCheck( string& value );
		/**
		   * Setter for due date.
		   * @param value sets the due date in a string with the format yyyymmdd
		   */
		virtual void setDueDate( string& value );

		/**
		   * Setter for alarm date.
		   * @param value sets the alarm date in a string with the format yyyymmdd
		   */
		virtual void setAlarmDate( string& value );

		/**
		   * Setter for alarm time.
		   * @param value sets the alarm time in a string with 4 bytes in the format hhdd. So every value is between 0-9 (0x30-0x39)
		   * hours 00-23
		   * minutes 00-59
		   */
		virtual void setAlarmTime( string& value );
		virtual void setCheckDate( string& value );

		/**
		   * Setter for category.
		   * @param value sets one byte where 0 stands for A, 1:B, 2:C
		   */
		virtual void setPriority( string& value );

		/**
		   * Setter for category.
		   * @param value sets one byte where 0 stands for category  A, 1:B, 2:C, 3:D, 4:E
		   */
		virtual void setCategory( string& value );

		/**
		   * Setter for description.
		   * @param value sets the description in a string with up to 2046 characters
		   * @exception PVDataEntryException
		   */
		virtual void setDescription( string& value );


		/**
		   * Setter for the data of a field.
		   * @param packet data packet for the field
		   * @exception PVDataEntryException
		   */
		virtual void setFieldData( datapacket& packet );

		/**
		   * Checks if this instance of todo is sendable.
		   * The fields due date and description have to be set.
		   * @return bool true if all nessecary fields are filled else false.
		   */
		virtual bool isSendable();

		/**
		   * stream the content
		   */
		friend ostream& operator<< (ostream& out, PVTodo& todo);

private:
		/**
		   * map for handle the DataElements
		   */
		map<unsigned int, string> m_data;
};

#endif
