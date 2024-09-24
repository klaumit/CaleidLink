/***************************************************************************
                          serial.cpp  -  description
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

// C++ includes
#include <iostream>
// C includes
#include <stdio.h>		/* Standard input/output definitions */
#include <unistd.h>		/* UNIX standard function definitions */
#include <fcntl.h>			/* File control definitions */
#include <errno.h>		/* Error number definitions */
#include <sys/ioctl.h>
//#include <string.h>
#include <sys/time.h>
// project includes
#include "serial.h"

#define NO_PARITY				1			// 8N1
#define EVEN_PARITY			2			// 7E1
#define ODD_PARITY			3			// 7O1
#define SPACE_PARITY		4			// 7S1

/**
   * Constructor.
   */
Serial::Serial(){
	cout << "BEGIN: Serial::Serial()" <<endl;

	m_fd = -1;		// Port not open

	cout << "END: Serial::Serial()" <<endl;
}

/**
   * Constructor.
   * @param port gives a char pointer to a string which contains the path to the device file
   */
Serial::Serial(const char* port){
	cout << "BEGIN: Serial::Serial(const char*)" <<endl;

	OpenPort(port);

	cout << "END: Serial::Serial(const char*)" <<endl;
}

/**
   * Destructor.
   */
Serial::~Serial(){
	cout << "BEGIN: Serial::~Serial()" <<endl;

	close(m_fd);

	cout << "END: Serial::~Serial()" <<endl;
}

void Serial::OpenPort(const char* port){
	cout << "BEGIN: Serial::OpenPort(string port)" <<endl;
	m_fd = open(port, O_RDWR | O_NOCTTY | O_NDELAY);
	if ( m_fd == -1) { // Could not open the port.   !!!!!!!!!Does not work!!!!!!!!! if the port is opened twice
		cerr << "Serial::OpenPort: Unable to open " << port << " - " << endl;
		throw SerialException("Serial::OpenPort: Unable to open port ");
	} else {
		// set the options for the port ???????????????????????????????
		fcntl(m_fd, F_SETFL, 0);
		// Get the current options for the port...
		tcgetattr(m_fd, &m_options);
		// Enable hardware flow control
//		m_options.c_cflag |= CRTSCTS;//(CLOCAL | CREAD | CRTSCTS);
		// Setting raw mode
		cfmakeraw(&m_options);
//		m_options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);		//raw input
//		m_options.c_oflag &= ~OPOST;														//raw output
		// Set the new options for the port...
		tcsetattr(m_fd, TCSANOW, &m_options);
		// read non-blocking from port
//		fcntl(m_fd, F_SETFL, FNDELAY);
		// set parity
		SetParity(NO_PARITY);
		// Set the baud rates to 9600...
		SetInputSpeed(9600);
		SetOutputSpeed(9600);
	}
	cout << "Port " << port << " successfully opened" << endl;
	cout << "END: Serial::OpenPort(string port)" <<endl;
}

unsigned char& Serial::ReadByte(int timeout){										// parameter time out in msec
	if ( m_fd == -1 ) throw SerialException("Serial::ReadByte : Port not open");
	timeval timestart, timenow;
	gettimeofday(&timestart, NULL);

	while (!CheckForNextByte()) {
		gettimeofday(&timenow, NULL);
		if ( (timeout > 0) &&
				(((timenow.tv_sec - timestart.tv_sec)*1000000 + (timenow.tv_usec - timestart.tv_usec)) > timeout*1000)) {
			throw SerialException("Serial::ReadByte : timeout");
		}
	}
	unsigned char* buffer = (unsigned char*)malloc(1);
	memset(buffer, 0, 1);
	if ( read(m_fd, buffer, 1) == -1 ) throw SerialException("Serial::ReadByte : could not read next byte");
	m_recieved = *buffer;
	free(buffer);
//	cout << "recieved " << hex << (unsigned int)m_recieved << dec << endl;
	return m_recieved;
}

bool Serial::WriteByte(unsigned char byte){
	if ( m_fd == -1 ) throw SerialException("Serial::WriteByte : Port not open");
	int n = write(m_fd, &byte, 1);
	if (n < 0) {
		cerr << "could not send " << hex << byte << endl;
		throw SerialException("Serial::WriteByte : could not send");
		return false;
	} else {
//		cout << hex << "byte " << byte << " sended" << endl;
	}
	return true;
}

bool Serial::SetOutputSpeed(int speed){
	// Set the baud rates to ...
	/* B0, B50, B75, B110, B134, B150, B200, B300
		B600, B1200, B1800, B2400, B4800, B9600
		B19200, B38400, B57600, B115200, B230400
	*/
	if ( m_fd == -1 ) throw SerialException("Serial::SetOutputSpeed : Port not open");
	tcgetattr(m_fd, &m_options);
	switch ( speed ) {
			case 1200			:	cfsetospeed(&m_options, B1200); break;
			case 2400			:	cfsetospeed(&m_options, B2400); break;
			case 4800			:	cfsetospeed(&m_options, B4800); break;
			case 9600			:	cfsetospeed(&m_options, B9600); break;
			case 19200		:	cfsetospeed(&m_options, B19200); break;
			case 38400		:	cfsetospeed(&m_options, B38400); break;
			case 57600		:	cfsetospeed(&m_options, B57600); break;
			case 115200	:	cfsetospeed(&m_options, B115200); break;
			case 230400	:	cfsetospeed(&m_options, B230400); break;
			default				:	return false;
	}
	// Set the new options for the port...
	tcsetattr(m_fd, TCSANOW, &m_options);
	return true;
}

bool Serial::SetInputSpeed(int speed){
	// Set the baud rates to ...
	/* B0, B50, B75, B110, B134, B150, B200, B300
		B600, B1200, B1800, B2400, B4800, B9600
		B19200, B38400, B57600, B115200, B230400
	*/
	if ( m_fd == -1 ) throw SerialException("Serial::SetInputSpeed : Port not open");
	tcgetattr(m_fd, &m_options);
	switch ( speed ) {
			case 1200			:	cfsetispeed(&m_options, B1200); break;
			case 2400			:	cfsetispeed(&m_options, B2400); break;
			case 4800			:	cfsetispeed(&m_options, B4800); break;
			case 9600			:	cfsetispeed(&m_options, B9600); break;
			case 19200		:	cfsetispeed(&m_options, B19200); break;
			case 38400		:	cfsetispeed(&m_options, B38400); break;
			case 57600		:	cfsetispeed(&m_options, B57600); break;
			case 115200	:	cfsetispeed(&m_options, B115200); break;
			case 230400	:	cfsetispeed(&m_options, B230400); break;
			default				:	return false;
	}
	// Set the new options for the port...
	tcsetattr(m_fd, TCSANOW, &m_options);
	return true;
}

bool Serial::SetDTR() {
	if ( m_fd == -1 ) throw SerialException("Serial::SetDTR : Port not open");
	int status;

	ioctl(m_fd, TIOCMGET, &status);
	status |= TIOCM_DTR;
	ioctl(m_fd, TIOCMSET, &status);
	return true;
}

bool Serial::ClearDTR() {
	if ( m_fd == -1 ) throw SerialException("Serial::ClearDTR : Port not open");
	int status;

	ioctl(m_fd, TIOCMGET, &status);
	status &= ~TIOCM_DTR;
	ioctl(m_fd, TIOCMSET, &status);
	return true;
}

/** No descriptions */

bool Serial::CheckDTR(){
	int status;

	ioctl(m_fd, TIOCMGET, &status);
    //cout << "status = " << status << endl;
	if ( (status & TIOCM_DTR) == 2 ) {
       //cout << "DTR on" << endl;
       return true;
    }
    //cout << "DTR off" << endl;
	return false;
}

bool Serial::SetParity(int parity){
	if ( m_fd == -1 ) throw SerialException("Serial::SetParity : Port not open");
	tcgetattr(m_fd, &m_options);
	switch ( parity ) {
		case NO_PARITY		 	:  // No parity (8N1)
		case SPACE_PARITY	:  // Space parity is setup the same as no parity (7S1)
				m_options.c_cflag &= ~PARENB;
				m_options.c_cflag &= ~CSTOPB;
				m_options.c_cflag &= ~CSIZE;
				m_options.c_cflag |= CS8;
				break;

		case EVEN_PARITY		:  // Even parity (7E1)
				m_options.c_cflag |= PARENB;
				m_options.c_cflag &= ~PARODD;
				m_options.c_cflag &= ~CSTOPB;
				m_options.c_cflag &= ~CSIZE;
				m_options.c_cflag |= CS7;
				break;

		case ODD_PARITY		:  // Odd parity (7O1)
				m_options.c_cflag |= PARENB;
				m_options.c_cflag |= PARODD;
				m_options.c_cflag &= ~CSTOPB;
				m_options.c_cflag &= ~CSIZE;
				m_options.c_cflag |= CS7;
				break;

	}
	tcsetattr(m_fd, TCSANOW, &m_options);
	return true;
}

/** No descriptions */
bool Serial::CheckForNextByte(){
	if ( m_fd == -1 ) throw SerialException("Serial::CheckForByte : Port not open");
	int bytes = 0;

	ioctl(m_fd, FIONREAD, &bytes);
	if ( bytes ) return true;
	return false;
}


/** No descriptions */
int Serial::GetState(){
    int state;
    ioctl(m_fd, TIOCMGET, &state);
	 return state;
}
/** No descriptions */
bool Serial::SetState(int state){
    int tmp;
    ioctl(m_fd, TIOCMGET, &tmp);
    struct termios tmp2;
    tcgetattr(m_fd, &tmp2);
    tmp = state;
    ioctl(m_fd, TIOCMSET, &tmp);

    tcsetattr(m_fd, TCSANOW, &tmp2);
    ioctl(m_fd, TIOCMSET, &tmp);
    tcsetattr(m_fd, TCSANOW, &m_options);
    return true;
}
/** No descriptions */
struct termios Serial::GetOptions(){
    tcgetattr(m_fd, &m_options);
    return m_options;
}
/** No descriptions */
bool Serial::SetOptions(struct termios comoptions){
    m_options = comoptions;
    tcsetattr(m_fd, TCSANOW, &m_options);
    return true;
}
/** No descriptions */
bool Serial::CheckCTS(){
	int status;

	ioctl(m_fd, TIOCMGET, &status);
//    cout << "status = " << status << endl;
//    cout << "status&TIOCM_CTS = " << (status&TIOCM_CTS) << endl;
	if ( (status & TIOCM_CTS) == 32 ) {
       //cout << "CTS on" << endl;
       return true;
    }
    //cout << "CTS off" << endl;
	return false;
}
/** No descriptions */
bool Serial::SetCAR(){
	int status;

	ioctl(m_fd, TIOCMGET, &status);
cout << "set CAR status1: " << status<<endl;
	status |= TIOCM_CAR;
cout << "set CAR status2: " << status<<endl;
	ioctl(m_fd, TIOCMSET, &status);
	tcsetattr(m_fd, TCSANOW, &m_options);
	return true;
}
/** No descriptions */
bool Serial::ClearCAR(){
	int status;

	ioctl(m_fd, TIOCMGET, &status);
cout << "clear CAR status1: " << status<<endl;
	status &= ~TIOCM_CAR;
cout << "clear CAR status2: " << status<<endl;
	ioctl(m_fd, TIOCMSET, &status);
	tcsetattr(m_fd, TCSANOW, &m_options);
	return true;
}
/** No descriptions */
bool Serial::CheckCAR(){
	int status;

	ioctl(m_fd, TIOCMGET, &status);
    //cout << "status = " << status << endl;
	if ( (status & TIOCM_CAR) == 64 ) {
       //cout << "CAR on" << endl;
       return true;
    }
    //cout << "CAR off" << endl;
	return false;
}
/** No descriptions */
bool Serial::CheckDSR(){
	int status;

	ioctl(m_fd, TIOCMGET, &status);
    //cout << "status = " << status << endl;
  //cout <<"&&" <<             (status & TIOCM_DSR)<<endl;
	if ( (status & TIOCM_DSR) == 256 ) {
       //cout << "DSR on" << endl;
       return true;
    }
    //cout << "DSR off" << endl;
	return false;
}
/** No descriptions */
bool Serial::CheckRTS(){
	int status;

	ioctl(m_fd, TIOCMGET, &status);
    //cout << "status = " << status << endl;
	if ( (status & TIOCM_RTS) == 4 ) {
       //cout << "RTS on" << endl;
       return true;
    }
    //cout << "RTS off" << endl;
	return false;
}
