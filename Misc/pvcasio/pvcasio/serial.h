/***************************************************************************
                          serial.h  -  description
                             -------------------
    begin                : Mon Nov 19 2001
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

#ifndef SERIAL_H
#define SERIAL_H

// C includes
#include <stdlib.h>
#include <termios.h>	/* POSIX terminal control definitions */

//project includes
#include <serialexception.h>

/**This is a class to control a serial port. It can write, read and set the speed of a serial port.
  *@author Selzer Michael
  */

class Serial {
	public:
		/**
		   * Constructor.
		   */
		Serial();
		/**
		   * Constructor.
		   * @param port gives a char pointer to a string which contains the path to the device file
		   */
		Serial(const char* port);
		/**
		   * Destructor.
		   */
		~Serial();

		void OpenPort(const char* port);
		unsigned char& ReadByte(int timeout=0);
		bool WriteByte(unsigned char byte);
		bool SetOutputSpeed(int speed);
		bool SetInputSpeed(int speed);
		bool SetParity(int parity);
		bool SetDTR();
		bool ClearDTR();
  /** No descriptions */
  bool CheckDTR();
  /** No descriptions */
  bool CheckForNextByte();
  /** No descriptions */
  bool SetState(int state);
  /** No descriptions */
  int GetState();
  /** No descriptions */
  bool SetOptions(struct termios comoptions);
  /** No descriptions */
  struct termios GetOptions();
  /** No descriptions */
  bool CheckDSR();
  /** No descriptions */
  bool CheckCAR();
  /** No descriptions */
  bool ClearCAR();
  /** No descriptions */
  bool SetCAR();
  /** No descriptions */
  bool CheckCTS();
  /** No descriptions */
  bool CheckRTS();
  /** No descriptions */
  bool ClearRTS();
  /** No descriptions */
  bool SetRTS();

	private:
		int m_fd;
		termios m_options;	// Struct for serial parameters
		unsigned char m_recieved;

};

#endif
