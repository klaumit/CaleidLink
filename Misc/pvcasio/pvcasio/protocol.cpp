/***************************************************************************
                          protocol.cpp  -  description
                             -------------------
    begin                : Sat Feb 23 2002
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

// C includes
#include <unistd.h>
// project includes
#include <pvcasio/protocol.h>
#include <pvcasio/serialexception.h>
#include <pvcasio/protocolexception.h>

#define SOH		0x01
#define STX		0x02
#define ACK		0x06
#define NAK		0x15
#define CAN		0x18

/**
   * Constructor.
   * @param port gives a string which contains the path to the device file
   */
Protocol::Protocol(string port){
	std::cout << "BEGIN:Protocol::Protocol(string)" << std::endl;

	m_speed = 9600;
	m_actual_send_No = 0;
	m_actual_recieve_No = 0;
	m_userid = "";
	m_recievedNAK = 0;
	m_sendNAK = 0;
	try {
		m_com.OpenPort(port.c_str());
	} catch (SerialException e) {
		cerr << "ERROR: " << e.getMessage() << endl;
		throw ProtocolException("Protocol::Protocol :  could not open port");
	}

	std::cout << "END:Protocol::Protocol(string)" << std::endl;
}

/**
   * Constructor.
   * @param com sets a com-object which will be used for the communication
   */
Protocol::Protocol(Serial& com){
	std::cout << "BEGIN:Protocol::Protocol(Serial)" << std::endl;

	m_speed = 9600;
	m_actual_send_No = 0;
	m_actual_recieve_No = 0;
	m_userid = "";
	m_recievedNAK = 0;
	m_sendNAK = 0;
	m_com = com;
	m_com.SetInputSpeed(m_speed);
	m_com.SetOutputSpeed(m_speed);

	std::cout << "END:Protocol::Protocol(Serial)" << std::endl;
}

/**
   * Destructor.
   */
Protocol::~Protocol(){
	std::cout << "BEGIN:Protocol::~Protocol()" << std::endl;
	std::cout << "END:Protocol::~Protocol()" << std::endl;
}

 /**
    * This method sets the speed, which will be used after phase 1 (handshak phase).
    * @return  false in case of an error and true else.
    */
bool Protocol::SetRequestedSpeed(int speed){
	std::cout << "BEGIN:Protocol::SetRequestedSpeed(int speed)" << std::endl;

	bool rc = true;
	if ( ( speed == 38400 ) || ( speed == 19200 ) || ( speed == 9600 ) ) {
		m_speed = speed;
	} else {
		cerr << "ERROR: speed NOT supported!!!!!" << endl;
		rc = false;
	}
	// set the I/O speed to default until phase 1 is over
	m_com.SetInputSpeed(9600);
	m_com.SetOutputSpeed(9600);

	std::cout << "END:Protocol::SetRequestedSpeed(int speed)" << std::endl;
	return rc;
}

/**
   * toggles the DTR line to wake up the PV
   */
bool Protocol::WakeUp(){
	std::cout << "BEGIN:CasioPV::WakeUp()" << std::endl;

	int count = 0;
	do {
		m_com.ClearDTR();							/* DTR OFF */
		usleep( 30000 );
		m_com.SetDTR();							/* DTR ON */
		usleep( 30000 );
		if ( count == 20 )  throw ProtocolException("Protocol::WakeUp : PV didn't wake up");
		count++;
	} while ( !m_com.CheckForNextByte() );

	std::cout << "END:CasioPV::WakeUp()" << std::endl;
	return true;
}

/**
   * Every packet type has its own start byte which identify it. This method reads this byte from the serial line.
   * @return recieved packet type
   */
unsigned int Protocol::RecieveOrder(){
//	std::cout << "BEGIN:Protocol::RecieveOrder()" << std::endl;

	unsigned int order = m_com.ReadByte(1000);
	switch ( order ) {
		case SOH	:	cout << "Command packet" << endl;
								break;
		case STX	:	cout << "Data packet" << endl;
								break;
		case ACK	:	cout << "ACK: Positive response packet" << endl;
								break;
		case NAK	:	cout << "NAK: Negative response packet" << endl;
								break;
		case CAN	:	cout << "Link packet" << endl;
								break;
		default : throw ProtocolException("Protocol::RecieveOrder : unknown command: ");
	}

//	std::cout << "END:Protocol::RecieveOrder()" << std::endl;
	return order;
}

/**
   * This method waits for the acknowledge of the currently recieved packet.
   * The packet number is stored in m_actual_send_No.
   * @return  false in case of a NAK packet and true in case of a ACK packet.
   */
bool Protocol::RecieveACK(){
	std::cout << "BEGIN:Protocol::RecieveACK()" << std::endl;

	unsigned int ro, No;
	ro = RecieveOrder();
	if ( ro == NAK ) {
		No = RecievePacketNo();
		cerr << "ERROR: recieved NAK" << endl;
		m_recievedNAK++;
		if ( m_recievedNAK == 5 ) throw ProtocolException("Protocol::RecieveACK : recieved to much NAK");
		return false;
	} else if ( ro == ACK ) {
		m_recievedNAK = 0;
		No = RecievePacketNo();
		m_actual_send_No++;
		m_actual_send_No %= 256;
	} else {
		throw ProtocolException("Protocol::RecieveACK : waited for ACK or NAK but recieved " + ro);
	}

	std::cout << "END:Protocol::RecieveACK()" << std::endl;
	return true;
}

/**
   * This method sends the acknowledge of the currently recieved packet.
   * The packet number is stored in m_actual_recieve_No.
   * @return  false in case of an error and true else.
   */
bool Protocol::SendACK() {
	std::cout << "BEGIN:Protocol::SendACK()" << std::endl;

	m_com.WriteByte(ACK);
	SendPacketNo(m_actual_recieve_No);
	m_sendNAK = 0;
	cout << "Send ACK for PacketNo " << dec << m_actual_recieve_No << endl;
	m_actual_recieve_No++;
	m_actual_recieve_No %= 256;

	std::cout << "END:Protocol::SendACK()" << std::endl;
	return true; // for now
}

/**
   * This method sends the NOT acknowledge for the currently recieved packet.
   * The packet number is stored in m_actual_recieve_No.
   * This may happens if packet is corrupt.
   * @return  false in case of an error and true else.
   */
bool Protocol::SendNAK() {
	std::cout << "BEGIN:Protocol::SendNAK()" << std::endl;

	m_com.WriteByte(NAK);
	SendPacketNo(m_actual_recieve_No);
	m_sendNAK++;
	if ( m_sendNAK == 5 ) throw ProtocolException("Protocol::SendNAK : sended to much NAK");
	cout << "Send NAK for PacketNo " << dec << m_actual_recieve_No << endl;

	std::cout << "END:Protocol::SendNAK()" << std::endl;
	return true; // for now
}

/*************************************
* This part is for phase 1
* to establish the link
**************************************/

/**
   * This method waits for a link packet.
   * @exception ProtocolException
   */
void Protocol::RecieveLinkPacket(){
	std::cout << "BEGIN:Protocol::RecieveLinkPacket()" << std::endl;

	try {
		if ( RecieveOrder() != CAN ) throw ProtocolException("Protocol::RecieveLinkPacket : not a link packet!!");

		unsigned int response_no = RecievePacketNo();
		if ( response_no != m_actual_recieve_No ) throw ProtocolException("Protocol::RecieveLinkPacket : Recieved wrong packet number");

		unsigned int linkcommand = HexToInt(m_com.ReadByte()); linkcommand *=16;
							linkcommand += HexToInt(m_com.ReadByte()); linkcommand *=16;
							linkcommand += HexToInt(m_com.ReadByte()); linkcommand *=16;
							linkcommand += HexToInt(m_com.ReadByte());

		switch ( linkcommand ) {
			case CALLING_UP :
				cout << "Calling up" << endl;
				break;
			case NO_RESPONSE_INTERROGATION :
				cout << "No response interrogation" << endl;
				break;
			case  END_COMMUNICATION :
				cout << "End communication" << endl;
				break;
			case BREAK_COMMUNICATION :
				cout << "Break data block" << endl;
				break;
			case BREAK_COMMAND_BLOCK :
				cout << "Break command block" << endl;
				break;
			case BREAK_DATA_BLOCK :
				 cout << "Break data block" << endl;
				 break;
			default :
				throw ProtocolException("Protocol::RecieveLinkPacket : recieved unkown link packet !!!!!!!!");
		}
	} catch (SerialException e) {
		cerr << "ERROR: " << e.getMessage() << endl;
		throw ProtocolException("Protocol::RecieveLinkPacket : timeout");
	}

	std::cout << "END:Protocol::RecieveLinkPacket()" << std::endl;
}

/**
   * This method sends a link packet.
   * @param type takes a int which contains the type of the link packet
   * look in this header file to see correlatively predefined values
   */
void Protocol::SendLinkPacket(int type){
	std::cout << "BEGIN:Protocol::SendLinkPacket(com, int)" << std::endl;
	/*	0000 Calling up
		0001 No response interrogation
		0100 End communication
		0101 Break communication
		0021 Break command block
		0031 Break data block
	*/
	m_com.WriteByte(CAN);
	SendPacketNo(m_actual_send_No);
	m_com.WriteByte(0x30);
	switch ( type ) {
		// Calling up
		case CALLING_UP :
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x30);
							break;
		// No responce interrogation
		case NO_RESPONSE_INTERROGATION :
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x31);
							break;
		// End communication
		case END_COMMUNICATION :
							m_com.WriteByte(0x31);
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x30);
							break;
		// Break communication
		case BREAK_COMMUNICATION :
							m_com.WriteByte(0x31);
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x31);
							break;
		// Break command block
		case BREAK_COMMAND_BLOCK :
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x32);
							m_com.WriteByte(0x31);
							break;
		// Break data block
		case BREAK_DATA_BLOCK :
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x33);
							m_com.WriteByte(0x31);
							break;
	}

	std::cout << "END:Protocol::SendLinkPacket(int)" << std::endl;
}

/**
   * This method waits for a command packet.
   * @param order_recieved defaults to false. If this is set to true the packet type byte must be checked with RecieveOrder.
   * @return  false in case of an error and true else.
   * @exception ProtocolException
   */
bool Protocol::RecieveCommandPacket(bool order_recieved) {
	std::cout << "BEGIN:Protocol::RecieveCommandPacket(bool order_recieved=false)" << std::endl;

	bool rc = true;
	try {
		if ( !order_recieved && RecieveOrder() != SOH ) throw ProtocolException("Protocol::RecieveCommandPacket : not a command packet!!");

		unsigned int response_no = RecievePacketNo();
		if ( response_no != m_actual_recieve_No ) throw ProtocolException("Protocol::RecieveCommandPacket : Recieved wrong packet number!!!!!!!!");

		unsigned int No_of_bytes1 = m_com.ReadByte();
		unsigned int No_of_bytes2 = m_com.ReadByte();

		unsigned int No_of_Bytes = HexToInt(No_of_bytes1)*16 + HexToInt(No_of_bytes2);
//		cout << "No of bytes : " << dec << No_of_Bytes << endl;

		unsigned int tmp = m_com.ReadByte();
		unsigned int check = No_of_bytes1 + No_of_bytes2 + tmp;
		unsigned int command = HexToInt(tmp); command *=16; tmp = m_com.ReadByte(); check += tmp;
							command += HexToInt(tmp); command *=16; tmp = m_com.ReadByte(); check += tmp;
							command += HexToInt(tmp); command *=16; tmp = m_com.ReadByte(); check += tmp;
							command += HexToInt(tmp);

		unsigned int byte[63];
		unsigned int i;
		for (i = 5; i<=No_of_Bytes; i++) {
			byte[i-5] = m_com.ReadByte();
			check += byte[i-5];
		}

		unsigned int checksum_byte1 = m_com.ReadByte();
		unsigned int checksum_byte2 = m_com.ReadByte();

		switch ( command ) {
			case PHASE_TRANSITION_COMMAND :						// 0x8100
				cout << "Command waiting for result" << endl;
				cout << "Phase transition command: Parameter = Transit host" << endl;
				cout << "Transit host = " << dec;
				for (i = 5; i<=No_of_Bytes; i++) {
					cout << byte[i-5];
				}
				cout << endl;
				break;
			case PROTOCOL_LEVEL_EXCHANGE :							// 0x8110
				cout << "Command waiting for result" << endl;
				cout << "Protocol level exchange: Parameter = Level info" << endl;
				cout << "Level info = " << dec;
				for (i = 5; i<=No_of_Bytes; i++) {
					cout << byte[i-5];
				}
				cout << endl;
				break;
			case USER_ID_EXCHANGE :												// 0x8111
				cout << "Command waiting for result" << endl;
				cout << "User ID exchange: Parameter = UserID" << endl;
				cout << "UserID = " << dec;
				m_userid = "";
				for (i = 5; i<=No_of_Bytes; i++) {
					cout << (unsigned char)byte[i-5];
					m_userid += byte[i-5];
				}
				cout << endl;
				break;
			case COMMUNICATION_SPEED_SETTING :				// 0x8112
				cout << "Command waiting for result" << endl;
				cout << "Communication Speed Setting: Parameter = Com. speed" << endl;
				cout << "Com. speed = " << dec;
				for (i = 5; i<=No_of_Bytes; i++) {
					cout << (unsigned char)byte[i-5];
				}
				cout << endl;
				break;
			case APPEND_REGISTRATION :										// 0x8010
				cout << "Command waiting for result" << endl;
				cout << "Append registration" << endl;
				cerr << "Not implemented!!!!!!!!!" << endl;
				break;
			case DATA_SEND_REQUEST :											// 0x8032
				cout << "Command waiting for result" << endl;
				cout << "Data send request" << endl;
				cerr << "Not implemented!!!!!!!!!" << endl;
				break;
			case NUMBER_OF_DATA_REQUEST :							// 0x8041
				cout << "Command waiting for result" << endl;
				cout << "Number of data request" << endl;
				cerr << "Not implemented!!!!!!!!!" << endl;
				break;
			case PROTOCOL_LEVEL_EXCHANGE_R :						// 0x8910
				cout << "Result command" << endl;
				cout << "Protocol level exchange" << endl;
				cout << "Level info = " << dec;
				for (i = 5; i<=No_of_Bytes; i++) {
					cout << byte[i-5];
				}
				cout << endl;
				break;
			case USER_ID_EXCHANGE_R :											// 0x8911
				cout << "Result command" << endl;
				cout << "User ID exchange" << endl;
				cout << "UserID = ";
				for (i = 5; i<=No_of_Bytes; i++) {
					cout << (unsigned char)byte[i-5];
				}
				cout << endl;
				break;
			case COMMUNICATION_SPEED_SETTING_R :			// 0x8912
				cout << "Result command" << endl;
				cout << "Communication speed setting" << endl;
				cout << "Com. speed = ";
				for (i = 5; i<=No_of_Bytes; i++) {
					cout << byte[i-5];
				}
				cout << endl;
				break;
			case PHASE_TRANSITION_COMMAND_R :					// 0x0900
				cout << "Result command" << endl;
				cout << "Phase transition command" << endl;
				cerr << "Not implemented!!!!!!!!!" << endl;
				break;
			case APPEND_REGISTRATION_R :									// 0x8810
				cout << "Result command" << endl;
				cout << "Append registration" << endl;
				cerr << "Not implemented!!!!!!!!!" << endl;
				break;
			case DATA_SEND_REQUEST_R :										// 0x8832
				cout << "Result command" << endl;
				cout << "Data send request" << endl;
				break;
			case NUMBER_OF_DATA_REQUEST_R :						// 0x8841
				cout << "Result command" << endl;
				cout << "Number of data request" << endl;
				break;
			case START_DATA_BLOCK :												// 0x0200
				cout << "Start data block" << endl;
				break;
			case END_DATA_BLOCK :													// 0x0201
				cout << "End data block" << endl;
				break;
			case DATA_BLOCK_CHECK :											// 0x0202
				cout << "Data block check" << endl;
				cerr << "Not implemented!!!!!!!!!" << endl;
				break;
			default :
				cerr << "ERROR: Wrong Command" << endl;
		}

		check--;
		if ( (IntToHex(((int)check/16 ^ 0xff)%16) != checksum_byte1) ||
			 (IntToHex(((check%16) ^ 0xff)%16) != checksum_byte2) ) {
					cerr << "ERROR: Checksum not correct !!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
					rc = false;
		}
	} catch (SerialException e) {
		cerr << "ERROR: " << e.getMessage() << endl;
		throw ProtocolException("Protocol::RecieveCommandPacket : timeout");
	}

	std::cout << "END:Protocol::RecieveCommandPacket(bool order_recieved=false)" << std::endl;
	return rc;
}

/**
   * This method sends a command packet.
   * @param type takes a int which contains the type of the command packet
   * @param DataCondition specifies the ModeCode of the data entry
   * @param DataOrder specifies the nummer of the data entry
   * look in this header file to see correlatively predefined values
   */
bool Protocol::SendCommandPacket(int type, int DataCondition, int DataOrder){
	std::cout << "BEGIN:Protocol::SendCommandPacket(int, int, int)" << std::endl;

	m_com.WriteByte(SOH);
	SendPacketNo(m_actual_send_No);			// PacketNo

	int tmpbyte1, tmpbyte2, tmpbyte3, tmpbyte4, tmpbyte5, tmpbyte6;
	unsigned int checksum;

	switch (type) {
		// command packets
		// Negotiation Command
		// Protocol level exchange
		case PROTOCOL_LEVEL_EXCHANGE :
							cout << "Protocol level exchange" << endl;
							m_com.WriteByte(0x30);		// No of bytes
							m_com.WriteByte(0x43);
							m_com.WriteByte(0x38);		// sub command
							m_com.WriteByte(0x31);		// Protocol level exchange
							m_com.WriteByte(0x31);
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x32);		// level for PC application
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x31);
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x34);		// Checksum
							m_com.WriteByte(0x30);
							break;
		// User ID exchange
		case USER_ID_EXCHANGE :
							cout << "User ID exchange" << endl;
							m_com.WriteByte(0x32);		// No of bytes
							m_com.WriteByte(0x43);
							m_com.WriteByte(0x38);		// sub command
							m_com.WriteByte(0x31);		// User ID exchange
							m_com.WriteByte(0x31);
							m_com.WriteByte(0x31);
							m_com.WriteByte(0x64);		// Send ID String
							m_com.WriteByte(0x91);
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x20);
							m_com.WriteByte(0x20);
							m_com.WriteByte(0x20);
							m_com.WriteByte(0x20);
							m_com.WriteByte(0x20);
							m_com.WriteByte(0x20);
							m_com.WriteByte(0x20);
							m_com.WriteByte(0x20);
							m_com.WriteByte(0x20);
							m_com.WriteByte(0x20);
							m_com.WriteByte(0x20);
							m_com.WriteByte(0x20);
							m_com.WriteByte(0x50);
							m_com.WriteByte(0x43);
							m_com.WriteByte(0x53);
							m_com.WriteByte(0x59);
							m_com.WriteByte(0x4e);
							m_com.WriteByte(0x43);
							m_com.WriteByte(0x31);
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x34);
							m_com.WriteByte(0x31);
							m_com.WriteByte(0x44);
							m_com.WriteByte(0x43);
							m_com.WriteByte(0x44);
							m_com.WriteByte(0x41);
							m_com.WriteByte(0x39);
							m_com.WriteByte(0x33);
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x35);		// Checksum
							m_com.WriteByte(0x44);
							break;
		// Communication speed setting
		case COMMUNICATION_SPEED_SETTING :
							cout << "Communication speed setting" << endl;
							m_com.WriteByte(0x31);		// No of bytes
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x38);		// sub command
							m_com.WriteByte(0x31);		// Communication speed setting
							m_com.WriteByte(0x31);
							m_com.WriteByte(0x32);
							// Possible speed settings
							// 230400,153600,115200, 76800, 57600, 38400, 28800, 19200, 14400, 9600, 7200, 4800
							// supported speed settings: 57600, 38400, 19200, 9600
							m_com.WriteByte(0x30);		// Send speed
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x32);
							m_com.WriteByte(0x31);
							m_com.WriteByte(0x31);
							m_com.WriteByte(0x31);
							m_com.WriteByte(0x31);
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x38);		// Checksum
							m_com.WriteByte(0x44);
							break;
		// Ordinal Commands
		// Append registration
		case APPEND_REGISTRATION :
							cout << "Append registration" << endl;
							m_com.WriteByte(0x31);		// No of bytes
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x38);		// sub command
							m_com.WriteByte(0x30);		// Append registration
							m_com.WriteByte(0x31);
							m_com.WriteByte(0x30);
							checksum = 0x31 + 0x30 + 0x38 + 0x30 + 0x31 + 0x30;
							tmpbyte1 = IntToHex(DataCondition%16); DataCondition /=16;		// Data condition
							tmpbyte2 = IntToHex(DataCondition%16); DataCondition /=16;
							tmpbyte3 = IntToHex(DataCondition%16); DataCondition /=16;
							tmpbyte4 = IntToHex(DataCondition%16); DataCondition /=16;
							tmpbyte5 = IntToHex(DataCondition%16); DataCondition /=16;
							tmpbyte6 = IntToHex(DataCondition%16); DataCondition /=16;
							checksum += tmpbyte1 + tmpbyte2 + tmpbyte3 + tmpbyte4 + tmpbyte5 + tmpbyte6;
							m_com.WriteByte(tmpbyte6);
							m_com.WriteByte(tmpbyte5);
							m_com.WriteByte(tmpbyte4);
							m_com.WriteByte(tmpbyte3);
							m_com.WriteByte(tmpbyte2);
							m_com.WriteByte(tmpbyte1);

							checksum += 6 * 0x46;
							m_com.WriteByte(0x46);		// Option (fixed value FFFFFF)
							m_com.WriteByte(0x46);
							m_com.WriteByte(0x46);
							m_com.WriteByte(0x46);
							m_com.WriteByte(0x46);
							m_com.WriteByte(0x46);

							SendChecksumFor(checksum);
							break;
		// Data send request
		case DATA_SEND_REQUEST :
							cout << "Data send request" << endl;
							m_com.WriteByte(0x31);		// No of bytes
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x38);		// sub command
							m_com.WriteByte(0x30); 		// Data send request
							m_com.WriteByte(0x33);
							m_com.WriteByte(0x32);
							checksum = 0x31 + 0x30 + 0x38 + 0x30 + 0x33 + 0x32;
							tmpbyte1 = IntToHex(DataCondition%16); DataCondition /=16;		// Data condition
							tmpbyte2 = IntToHex(DataCondition%16); DataCondition /=16;
							tmpbyte3 = IntToHex(DataCondition%16); DataCondition /=16;
							tmpbyte4 = IntToHex(DataCondition%16); DataCondition /=16;
							tmpbyte5 = IntToHex(DataCondition%16); DataCondition /=16;
							tmpbyte6 = IntToHex(DataCondition%16); DataCondition /=16;
							checksum += tmpbyte1 + tmpbyte2 + tmpbyte3 + tmpbyte4 + tmpbyte5 + tmpbyte6;
							m_com.WriteByte(tmpbyte6);
							m_com.WriteByte(tmpbyte5);
							m_com.WriteByte(tmpbyte4);
							m_com.WriteByte(tmpbyte3);
							m_com.WriteByte(tmpbyte2);
							m_com.WriteByte(tmpbyte1);
							tmpbyte1 = IntToHex(DataOrder%16); DataOrder /=16;					// Data order
							tmpbyte2 = IntToHex(DataOrder%16); DataOrder /=16;
							tmpbyte3 = IntToHex(DataOrder%16); DataOrder /=16;
							tmpbyte4 = IntToHex(DataOrder%16); DataOrder /=16;
							tmpbyte5 = IntToHex(DataOrder%16); DataOrder /=16;
							tmpbyte6 = IntToHex(DataOrder%16); DataOrder /=16;
							checksum += tmpbyte1 + tmpbyte2 + tmpbyte3 + tmpbyte4 + tmpbyte5 + tmpbyte6;
							m_com.WriteByte(tmpbyte6);
							m_com.WriteByte(tmpbyte5);
							m_com.WriteByte(tmpbyte4);
							m_com.WriteByte(tmpbyte3);
							m_com.WriteByte(tmpbyte2);
							m_com.WriteByte(tmpbyte1);

							SendChecksumFor(checksum);
							break;
		// Number of data request
		case NUMBER_OF_DATA_REQUEST :
							cout << "Number of data request" << endl;
							m_com.WriteByte(0x30);		// No of bytes
							m_com.WriteByte(0x41);
							m_com.WriteByte(0x38);		// sub command
							m_com.WriteByte(0x30);		// Number of data request
							m_com.WriteByte(0x34);
							m_com.WriteByte(0x31);
							checksum = 0x30 + 0x41 + 0x38 + 0x30 + 0x34 + 0x31;
							tmpbyte1 = IntToHex(DataCondition%16); DataCondition /=16;		// Data condition
							tmpbyte2 = IntToHex(DataCondition%16); DataCondition /=16;
							tmpbyte3 = IntToHex(DataCondition%16); DataCondition /=16;
							tmpbyte4 = IntToHex(DataCondition%16); DataCondition /=16;
							tmpbyte5 = IntToHex(DataCondition%16); DataCondition /=16;
							tmpbyte6 = IntToHex(DataCondition%16); DataCondition /=16;
							checksum += tmpbyte1 + tmpbyte2 + tmpbyte3 + tmpbyte4 + tmpbyte5 + tmpbyte6;
							m_com.WriteByte(tmpbyte6);
							m_com.WriteByte(tmpbyte5);
							m_com.WriteByte(tmpbyte4);
							m_com.WriteByte(tmpbyte3);
							m_com.WriteByte(tmpbyte2);
							m_com.WriteByte(tmpbyte1);

							SendChecksumFor(checksum);
							break;
		// response packets
		// Negotiation command
		// Protocol level exchange response
		case PROTOCOL_LEVEL_EXCHANGE_R :
							cout << "Protocol level exchange response" << endl;
							m_com.WriteByte(0x30);		// No of bytes
							m_com.WriteByte(0x43);
							m_com.WriteByte(0x38);		// sub command
							m_com.WriteByte(0x39);		// Protocol level exchange
							m_com.WriteByte(0x31);
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x32);		// level for PC application 20001000
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x31);
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x33);		// Checksum
							m_com.WriteByte(0x38);
							break;
		// User ID exchange response
		case USER_ID_EXCHANGE_R :
							cout << "User ID exchange response" << endl;
							m_com.WriteByte(0x32);		// No of bytes
							m_com.WriteByte(0x43);
							m_com.WriteByte(0x38);		// sub command
							m_com.WriteByte(0x39);		// User ID exchange
							m_com.WriteByte(0x31);
							m_com.WriteByte(0x31);
							checksum = 0x32 + 0x43 + 0x38 + 0x39 + 0x31 + 0x31;
							for ( unsigned int i = 0; i<40; i++ ) {						// Send ID String
								m_com.WriteByte(m_userid[i]);
								checksum += (unsigned int)m_userid[i];
							}

							SendChecksumFor(checksum);
							break;
		// Communication speed setting response
		case COMMUNICATION_SPEED_SETTING_R :
							cout << "Communication speed setting response" << endl;
							m_com.WriteByte(0x31);		// No of bytes
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x38);		// sub command
							m_com.WriteByte(0x39);		// Communication speed setting
							m_com.WriteByte(0x31);
							m_com.WriteByte(0x32);
							// Possible speed settings
							// 230400,153600,115200, 76800, 57600, 38400, 28800, 19200, 14400, 9600, 7200, 4800
							// supported speed settings: 57600, 38400, 19200, 9600
							for (unsigned int i = 12; i>0; i--) {							// speed
								if ( (i == 8) && ( m_speed == 57600 ) ||
									  (i == 7) && ( m_speed == 38400 ) ||
									  (i == 5) && ( m_speed == 19200 ) ||
									  (i == 3) && ( m_speed == 9600 ) ) {
									m_com.WriteByte(0x31);
								} else {
									m_com.WriteByte(0x30);
								}
							}
							m_com.WriteByte(0x38);		// Checksum
							m_com.WriteByte(0x41);
							break;
		// Phase Transition Command response
		case PHASE_TRANSITION_COMMAND_R :
							cout << "Phase Transition Command response to 01" << endl;
							m_com.WriteByte(0x30);		// No of bytes
							m_com.WriteByte(0x38);
							m_com.WriteByte(0x30);		// sub command
							m_com.WriteByte(0x39);		// Phase Transition Command
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x30);		// Send Phase Transition Command
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x30);		// Checksum
							m_com.WriteByte(0x46);
							break;
		// Ordinal Commands
		// Append registration response
		case APPEND_REGISTRATION_R :
							cout << "Append registration response" << endl;
							m_com.WriteByte(0x30);		// No of bytes
							m_com.WriteByte(0x38);
							m_com.WriteByte(0x38);		// sub command
							m_com.WriteByte(0x38);		// Append registration response
							m_com.WriteByte(0x31);
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x30);		// Result
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x38);		// Checksum
							m_com.WriteByte(0x44);
							break;
		// Data send request response
		case DATA_SEND_REQUEST_R :	cout << "Data send request response" << endl;
							m_com.WriteByte(0x30);		// No of bytes
							m_com.WriteByte(0x38);
							m_com.WriteByte(0x38);		// sub command
							m_com.WriteByte(0x38);		// Data send request response
							m_com.WriteByte(0x33);
							m_com.WriteByte(0x32);
							m_com.WriteByte(0x30);		// Result
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x38);		// Checksum
							m_com.WriteByte(0x44);
							break;
		// Number of data request response
		case NUMBER_OF_DATA_REQUEST_R :
							cout << "Number of data request response" << endl;
							m_com.WriteByte(0x30);		// No of bytes
							m_com.WriteByte(0x41);
							m_com.WriteByte(0x38);		// sub command
							m_com.WriteByte(0x38);		// Number of data request response
							m_com.WriteByte(0x34);
							m_com.WriteByte(0x31);
							m_com.WriteByte(0x30);		// Result
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x38);		// Checksum
							m_com.WriteByte(0x44);
							break;
		// Sub-commands for data block
		// Start Data Block
		case START_DATA_BLOCK :
							cout << "Start Data Block" << endl;
							m_com.WriteByte(0x30);		// No of bytes
							m_com.WriteByte(0x34);
							m_com.WriteByte(0x30);		// sub command
							m_com.WriteByte(0x32);	
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x44);		// Checksum
							m_com.WriteByte(0x41);
							break;
		// End Data Block
		case END_DATA_BLOCK :
							cout << "End Data Block" << endl;
							m_com.WriteByte(0x30);		// No of bytes
							m_com.WriteByte(0x34);
							m_com.WriteByte(0x30);		// sub command
							m_com.WriteByte(0x32);	
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x31);
							m_com.WriteByte(0x44);		// Checksum
							m_com.WriteByte(0x39);
							break;
		// Data Block Check
		case DATA_BLOCK_CHECK : 	cout << "Data Block Check" << endl;
							m_com.WriteByte(0x30);		// No of bytes
							m_com.WriteByte(0x34);
							m_com.WriteByte(0x30);		// sub command
							m_com.WriteByte(0x32);	
							m_com.WriteByte(0x30);
							m_com.WriteByte(0x32);
							m_com.WriteByte(0x44);		// Checksum
							m_com.WriteByte(0x38);
							break;
	}
	std::cout << "END:Protocol::SendCommandPacket(int, int, int)" << std::endl;
	return true; //for now
}

/*************************************
* This part is for phase 2
* to exchange data
**************************************/

/**
   * This method recieves a datapacket and store it in a datapacket.
   * @param packet contains the data, the field code and the continued bit
   * @param order_recieved defaults to false. If this is set to true the packet type byte must be checked with RecieveOrder.
   * @return  false in case of an error and true else.
   * @exception ProtocolException
   */
bool Protocol::RecieveDataPacket(datapacket& packet, bool order_recieved){
	std::cout << "BEGIN:Protocol::RecieveDataPacket(datapacket&, bool order_recieved = false)" << std::endl;

	bool rc = true;
	try {
		if ( !order_recieved && RecieveOrder() != STX ) throw ProtocolException("Protocol::RecieveDataPacket :  not a data packet!!");

		unsigned int response_no = RecievePacketNo();
		if ( response_no != m_actual_recieve_No ) throw ProtocolException("Protocol::RecieveDataPacket :  Recieved wrong packet number");

		unsigned int No_of_bytes1 = m_com.ReadByte(),
								No_of_bytes2 = m_com.ReadByte(),
								No_of_bytes3 = m_com.ReadByte(),
								No_of_bytes4 = m_com.ReadByte();

		unsigned int No_of_Bytes = HexToInt(No_of_bytes1)*16*16*16
															+ HexToInt(No_of_bytes2)*16*16 + HexToInt(No_of_bytes3)*16 + HexToInt(No_of_bytes4);

		unsigned int FieldCodeNo1 = m_com.ReadByte(),
								FieldCodeNo2 = m_com.ReadByte(),
								FieldCodeNo3 = m_com.ReadByte(),
								FieldCodeNo4 = m_com.ReadByte(),
								FieldCodeNo5 = m_com.ReadByte(),
								FieldCodeNo6 = m_com.ReadByte();

		packet.fieldCode =  HexToInt(FieldCodeNo1)*16*16*16*16 + HexToInt(FieldCodeNo2)*16*16*16 +
					HexToInt(FieldCodeNo3)*16*16 + HexToInt(FieldCodeNo4)*16 +
					HexToInt(FieldCodeNo5);

		FieldCodeNo6 == 0x31 ?  packet.continued = true : packet.continued = false;
		cout << "packet.fieldCode = " << hex << packet.fieldCode << dec << endl;
		cout << "packet.continued = " << (packet.continued ? "true" : "false") << endl;
		unsigned int check = No_of_bytes1 + No_of_bytes2 + No_of_bytes3 + No_of_bytes4
									+ FieldCodeNo1 + FieldCodeNo2 + FieldCodeNo3 + FieldCodeNo4 + FieldCodeNo5 + FieldCodeNo6;

		unsigned int byte[1023];
		unsigned int i;

		packet.data = "";
		for (i = 7; i<=No_of_Bytes; i++) {
			byte[i-7] = m_com.ReadByte();
			packet.data += byte[i-7];
			check += byte[i-7];
//			if ( byte[i-7] == 0x0A ) check += 3; // move this in the DataEntry classes
		}
		cout << "packet.data = " << packet.data << endl;
		// increment of  actual_recieve_No
		m_actual_recieve_No++;
		m_actual_recieve_No %= 256;

		unsigned int checksum_byte1 = m_com.ReadByte();
		unsigned int checksum_byte2 = m_com.ReadByte();

		check--;
		if ( (IntToHex(((int)check/16 ^ 0xff)%16) != checksum_byte1) ||
			 (IntToHex(((check%16) ^ 0xff)%16) != checksum_byte2) ) {
					cerr << "ERROR: Checksum not correct !!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
					rc = false;
		}
	} catch (SerialException e) {
		cerr << "ERROR: " << e.getMessage() << endl;
		throw ProtocolException("Protocol::RecieveDataPacket : timeout");
	}

	std::cout << "END:Protocol::RecieveDataPacket(datapacket&, bool order_recieved = false)" << std::endl;
	return rc;
}

/**
   * Send a datapacket to the PV
   * @param packet contains the data, the field code and the continued bit
   */
bool Protocol::SendDataPacket(datapacket& packet) {
	std::cout << "BEGIN:Protocol::SendDataPacket(datapacket&)" << std::endl;

	unsigned int checksum;

	m_com.WriteByte(STX);

	SendPacketNo(m_actual_send_No);   // PacketNo

//	cout << "packet.data.size() = " << packet.data.size() << endl;
	unsigned int No_of_bytes4 = IntToHex( (packet.data.size()+6) % 16 );
	unsigned int No_of_bytes3 = IntToHex( ((unsigned int)( (packet.data.size()+6) / 16)) % 16 );
	unsigned int No_of_bytes2 = IntToHex( ((unsigned int)( (packet.data.size()+6) / 16 / 16)) % 16 );
	unsigned int No_of_bytes1 = IntToHex( ((unsigned int)( (packet.data.size()+6) / 16 / 16 / 16)) % 16 );

	m_com.WriteByte(No_of_bytes1);
	m_com.WriteByte(No_of_bytes2);
	m_com.WriteByte(No_of_bytes3);
	m_com.WriteByte(No_of_bytes4);

	unsigned int FieldCodeNo5 = IntToHex( packet.fieldCode % 16 );
	unsigned int FieldCodeNo4 = IntToHex( (unsigned int)(packet.fieldCode / 16) % 16 );
	unsigned int FieldCodeNo3 = IntToHex( (unsigned int)(packet.fieldCode / 16 / 16) % 16 );
	unsigned int FieldCodeNo2 = IntToHex( (unsigned int)(packet.fieldCode / 16 / 16 / 16) % 16 );
	unsigned int FieldCodeNo1 = IntToHex( (unsigned int)(packet.fieldCode / 16 / 16 / 16 / 16) % 16 );

	m_com.WriteByte(FieldCodeNo1);
	m_com.WriteByte(FieldCodeNo2);
	m_com.WriteByte(FieldCodeNo3);
	m_com.WriteByte(FieldCodeNo4);
	m_com.WriteByte(FieldCodeNo5);

	checksum = No_of_bytes1 + No_of_bytes2 + No_of_bytes3 + No_of_bytes4 + FieldCodeNo1 + FieldCodeNo2
							+ FieldCodeNo3 + FieldCodeNo4 + FieldCodeNo5;

	if ( packet.continued ) {
		m_com.WriteByte(0x31);
		checksum += 0x31;
	} else {
		m_com.WriteByte(0x30);
		checksum += 0x30;
	}

	for (unsigned int i = 0; i<packet.data.size(); i++) {
		m_com.WriteByte(packet.data[i]);
		checksum += packet.data[i];
	}
	SendChecksumFor(checksum);
	// increment of  actual_send_No
	m_actual_send_No++;
	m_actual_send_No %= 256;

	std::cout << "END:Protocol::SendDataPacket(datapacket&)" << std::endl;
	return true;
}

/**
   * This method sets the speed of the serial connection and waits one second as requested in the docs
   */
void Protocol::EndPhase1(){
	std::cout << "BEGIN:Protocol::EndPhase1()" << std::endl;

	cout <<  "Link established !! Now setting the new com speed and wait for a second" << endl;
	m_com.SetInputSpeed(m_speed);
	m_com.SetOutputSpeed(m_speed);
	cout << "com speed set to " << m_speed << endl;
	// have to wait one second as requested in the docs
	sleep(1);

	std::cout << "END:Protocol::EndPhase1()" << std::endl;
}
