/***************************************************************************
                          pvmultidate.h  -  description
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

#ifndef PVMULTIDATE_H
#define PVMULTIDATE_H

// project includes
#include <pvcasio/pvdataentry.h>

/**This class is similar to PVScheduler, but it has only Date, End Date and Description
  *@author Selzer Michael
  */

class PVMultiDate : public PVDataEntry  {
public: 
		/**
		 * Constructor.
		 */
		PVMultiDate();

		/**
		 * Destructor.
		 */
		virtual ~PVMultiDate();

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

		/**
		   * Getter for date.
		   * @return Return the date in a string with the format yyyymmdd
		   */
		virtual string getDate();

		/**
		   * Getter for enddate.
		   * @return Return the date in a string with the format yyyymmdd
		   */
		virtual string getEndDate();

		/**
		   * Getter for description.
		   * @return Return the description in a string with up to 2046 characters
		   */
		virtual string getDescription();




		/**
		   * Setter for date.
		   * @param value sets the date in a string with the format yyyymmdd
		   */
		virtual void setDate( string& value );

		/**
		   * Setter for enddate.
		   * @param value sets the date in a string with the format yyyymmdd
		   */
		virtual void setEndDate( string& value );

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
		   * Checks if this instance of mulitdate is sendable.
		   * The fields date and description have to be set.
		   * @return bool true if all nessecary fields are filled else false.
		   */
		virtual bool isSendable();

		/**
		   * stream the content
		   */
		friend ostream& operator<< (ostream& out, PVMultiDate& multidate);

private:
		/**
		   * map for handle the DataElements
		   */
		map<unsigned int, string> m_data;
};

#endif
