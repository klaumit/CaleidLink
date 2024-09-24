/***************************************************************************
                          casiopv.h  -  description
                             -------------------
    begin                : Thu Dec 13 2001
    copyright            : (C) 2001 by Selzer Michael
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

#ifndef CASIOPV_H
#define CASIOPV_H

// C++ includes
#include <string>
// project includes
#include <pvcasio/protocol.h>
#include <pvcasio/pvdataentry.h>

/**This is a class for the syncronization with the casio pv
  *@author Selzer Michael
  */

class CasioPV {
	public:
		/**
		   * Constructor.
		   * @param port a string which contains the path to the device file
		   */
		CasioPV(string port);
		/**
		   * Destructor.
		   */
		~CasioPV();

		/**
		   * Make a wake up call to the protocol
		   */
		bool WakeUp();

		/**
		   * This method waits for the link packet from the PV, which will be send if the WakeUp() method is called
		   * or if the Start-button on the craddle is pressed
		   * @param speed can be 38400, 19200 or 9600 (for the PV-450X)
		   * @return false in case of an error and true else.
		   * @exception CasioPVException
		   */
		bool WaitForLink(int speed);

		/**
		   * Get number of data in a section
		   * @param DataCondition specified through the ModeCode
		   * @exception CasioPVException
		   */
		int GetNumberOfData(int DataCondition);

		 /**
		    * This method gets the data for the corresponding section
		    * which is internaly handled through the data condition (mode code)
		    * @param PVDataEntry there can only be derivated classes used because this is an interface
		    * the data will be stored in this class
		    * @param DataOrder this is the number of the entry which will be downloaded
		    * @exception CasioPVException
		    */
		void GetData(PVDataEntry& dataEntry, unsigned int DataOrder);

		/**
		   * This method loads the content of a class derivated from PVDataEntry in the corresponding section
		   * which is internaly handled through the data condition (mode code)
		   * @param PVDataEntry there can only derivated classes be used because this is an interface
		   * @exception CasioPVException
		   */
		void SendData(PVDataEntry& dataEntry);

		/**
		   * This method releases the link
		   * This is phase 3 as descriped in the documentation
		   */
		void ReleaseLink();

	private:
		Protocol::Protocol* m_protocol;

};

#endif
