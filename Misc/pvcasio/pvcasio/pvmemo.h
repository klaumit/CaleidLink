/***************************************************************************
                          pvmemo.h  -  description
                             -------------------
    begin                : Mit Jul 10 2002
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

#ifndef PVMEMO_H
#define PVMEMO_H

// project includes
#include <pvcasio/pvdataentry.h>

/**
  *@author Selzer Michael
  */

class PVMemo : public PVDataEntry  {
public:
		/**
		 * Constructor.
		 */
		PVMemo(unsigned int modeCode);

		/**
		 * Destructor.
		 */
		virtual ~PVMemo();

		/**
		   * Getter for the mode code.
		   * @return The mode code of the data entry.
		   */
		virtual unsigned int getModeCode() const;

		/**
		   * Getter for the data.
		   * @return Return all of the data.
		   */
		virtual const map<unsigned int, string>& getData() const;

		/**
		   * Getter for memo.
		   * @return Return the memo in a string with a maximum of 2048 characters
		   */
		virtual string getMemo();



		/**
		   * Setter for memo.
		   * @param value sets the memo in a string with a maximum of 2048 characters
		   * @exception PVDataEntryException
		   */
		virtual void setMemo( string& value );

		/**
		   * Setter for the data of a field.
		   * @param packet data packet for the field
		   * @exception PVDataEntryException
		   */
		virtual void setFieldData( datapacket& packet );

		/**
		   * Checks if this instance of memo is sendable.
		   * The field memo have to be set.
		   * @return bool true if all nessecary fields are filled else false.
		   */
		virtual bool isSendable();

		/**
		   * stream the content
		   */
		friend ostream& operator<< (ostream& out, PVMemo& memo);

private:
		/**
		   * map for handle the DataElements
		   */
		map<unsigned int, string> m_data;

		unsigned int m_modeCode;
		bool m_continued;
};

#endif
