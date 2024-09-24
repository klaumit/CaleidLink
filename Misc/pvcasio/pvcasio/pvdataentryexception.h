/***************************************************************************
                          pvdataentryexception.h  -  description
                             -------------------
    begin                : Don Aug 1 2002
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

#ifndef PVDATAENTRYEXCEPTION_H
#define PVDATAENTRYEXCEPTION_H

#include <pvcasio/baseexception.h>

/**This is the exception class for the data entry classes
  *@author Selzer Michael
  */

class PVDataEntryException : public BaseException  {
	public:
		/**
		   * Constructor.
		   */
		PVDataEntryException(string message);
		/**
		   * Destructor.
		   */
		~PVDataEntryException();

};

#endif
