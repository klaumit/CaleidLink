/***************************************************************************
                          pvquickmemo.h  -  description
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

#ifndef PVQUICKMEMO_H
#define PVQUICKMEMO_H

// project includes
#include <pvcasio/pvdataentry.h>

/**
  *@author Selzer Michael
  */

class PVQuickMemo : public PVDataEntry  {
public: 
		/**
		 * Constructor.
		 */
		PVQuickMemo();

		/**
		 * Destructor.
		 */
		virtual ~PVQuickMemo();


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
		   * Getter for the category.
		   * @return Return one byte where 0 stands for category  1, 1 for category 2, 2 for category 3
		   */
		virtual string getCategory();

		/**
		   * Getter for bitmap data.
		   * @return Return the data in bmp format
		   */
		virtual string getBitmapData();




		/**
		   * Setter for category.
		   * @param value sets one byte where 0 stands for category  1, 1 for category 2, 2 for category 3
		   */
		virtual void setCategory( string& value );

		/**
		   * Setter for bitmap data.
		   * @param value sets the bitmap data in a string with the format of a bmp
		   */
		virtual void setBitmapData( string& value );

		/**
		   * Setter for the data of a field.
		   * @param packet data packet for the field
		   * @exception PVDataEntryException
		   */
		virtual void setFieldData( datapacket& packet );

		/**
		   * Checks if this instance of quickmemo is sendable.
		   * The field bitmapdata have to be set.
		   * @return bool true if all nessecary fields are filled else false.
		   */
		virtual bool isSendable();

		/**
		   * stream the content
		   */
		friend ostream& operator<< (ostream& out, PVQuickMemo& quickmemo);

private:
		/**
		   * map for handle the DataElements
		   */
		map<unsigned int, string> m_data;

		bool m_continued;
};

#endif
