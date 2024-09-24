/***************************************************************************
                          pvpocketsheet.h  -  description
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

#ifndef PVPOCKETSHEET_H
#define PVPOCKETSHEET_H

// project includes
#include <pvcasio/pvdataentry.h>

/**
  *@author Selzer Michael
  */

class PVPocketSheet : public PVDataEntry  {
public: 
		/**
		 * Constructor.
		 */
		PVPocketSheet();

		/**
		 * Destructor.
		 */
		virtual ~PVPocketSheet();

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

		virtual string getSheetData();
		virtual string getXLineData();
		virtual string getYLineData();
		virtual string getCellData();

		virtual void setSheetData( string& value );
		virtual void setXLineData( string& value );
		virtual void setYLineData( string& value );
		virtual void setCellData( string& value );

		/**
		   * Setter for the data of a field.
		   * @param packet data packet for the field
		   * @exception PVDataEntryException
		   */
		virtual void setFieldData( datapacket& packet );

		/**
		   * Checks if this instance of  pocketsheet is sendable.
		   * The fields xxxxxxxxxxxxxx have to be set.
		   * @return bool true if all nessecary fields are filled else false.
		   */
		virtual bool isSendable();

		/**
		   * stream the content
		   */
		friend ostream& operator<< (ostream& out, PVPocketSheet& pocketsheet);

private:
		/**
		   * map for handle the DataElements
		   */
		map<unsigned int, string> m_data;
};

#endif
