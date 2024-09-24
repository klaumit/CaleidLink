/***************************************************************************
                          protocol.h  -  description
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

#ifndef PROTOCOL_H
#define PROTOCOL_H

// C++ includes
#include <string>
// project includes
#include <pvcasio/serial.h>

/**This class manage the sending and receiving of the casio protocol
  *@author Selzer Michael
  */

// This defines are for the link packet
#define CALLING_UP														0x0000
#define NO_RESPONSE_INTERROGATION				0x0001
#define END_COMMUNICATION									0x0100
#define BREAK_COMMUNICATION								0x0101
#define BREAK_COMMAND_BLOCK							0x0021
#define BREAK_DATA_BLOCK										0x0031
// This defines are for the command packet
#define PHASE_TRANSITION_COMMAND				0x8100
#define PROTOCOL_LEVEL_EXCHANGE					0x8110
#define USER_ID_EXCHANGE										0x8111
#define COMMUNICATION_SPEED_SETTING			0x8112
#define APPEND_REGISTRATION									0x8010
#define DATA_SEND_REQUEST									0x8032
#define NUMBER_OF_DATA_REQUEST						0x8041
#define PROTOCOL_LEVEL_EXCHANGE_R				0x8910
#define USER_ID_EXCHANGE_R									0x8911
#define COMMUNICATION_SPEED_SETTING_R		0x8912
#define PHASE_TRANSITION_COMMAND_R			0x0900
#define APPEND_REGISTRATION_R							0x8810
#define DATA_SEND_REQUEST_R								0x8832
#define NUMBER_OF_DATA_REQUEST_R					0x8841
#define START_DATA_BLOCK										0x0200
#define END_DATA_BLOCK											0x0201
#define DATA_BLOCK_CHECK										0x0202

struct datapacket {
		unsigned int fieldCode;
		bool continued;
		string data;
};

class Protocol {
	public:
		/**
		   * Constructor.
		   * @param port takes a string which contains the path to the device file
		   */
		Protocol(string port);
		/**
		   * Constructor.
		   * @param com sets a com-object which will be used for the communication
		   */
		Protocol(Serial& com);

		/**
		   * Destructor.
		   */
		~Protocol();

		/**
		   * This method sets the speed, which will be used after phase 1 (handshak phase).
		   * @return  false in case of an error and true else.
		   */
		bool SetRequestedSpeed(int speed);

		/**
		   * toggles the DTR line to wake up the PV
		   */
		bool WakeUp();

		/**
		   * Every packet type has its own start byte which identify it. This method reads this byte from the serial line.
		   * @return recieved packet type
		   */
		unsigned int RecieveOrder();

		/**
		   * This method waits for the acknowledge of the currently recieved packet.
		   * The packet number is stored in m_actual_send_No.
		   * @return  false in case of a NAK packet and true in case of a ACK packet.
		   */
		bool RecieveACK();

		/**
		   * This method sends the acknowledge of the currently recieved packet.
		   * The packet number is stored in m_actual_recieve_No.
		   * @return  false in case of an error and true else.
		   */
		bool SendACK();

		/**
		   * This method sends the NOT acknowledge for the currently recieved packet.
		   * The packet number is stored in m_actual_recieve_No.
		   * This may happens if packet is corrupt.
		   * @return  false in case of an error and true else.
		   */
		bool SendNAK();

		/**
		   * This method waits for a link packet.
		   * @exception ProtocolException
		   */
		void RecieveLinkPacket();

		/**
		   * This method sends a link packet.
		   * @param type takes a int which contains the type of the link packet
		   * look in this header file to see correlatively predefined values
		   */
		void SendLinkPacket(int type);

		/**
		   * This method waits for a command packet.
		   * @param order_recieved defaults to false. If this is set to true the packet type byte must be checked with RecieveOrder.
		   * @return  false in case of an error and true else.
		   * @exception ProtocolException
		   */
		bool RecieveCommandPacket(bool order_recieved = false);

		/**
		   * This method sends a command packet.
		   * @param type takes a int which contains the type of the command packet
		   * @param DataCondition specifies the ModeCode of the data entry
		   * @param DataOrder specifies the nummer of the data entry
		   * look in this header file to see correlatively predefined values
		   */
		bool SendCommandPacket(int type, int DataCondition = 0, int DataOrder = 0);

		/**
		   * This method recieves a datapacket and store it in a datapacket.
		   * @param packet contains the data, the field code and the continued bit
		   * @param order_recieved defaults to false. If this is set to true the packet type byte must be checked with RecieveOrder.
		   * @return  false in case of an error and true else.
		   * @exception ProtocolException
		   */
		bool RecieveDataPacket(datapacket& packet, bool order_recieved = false);

		/**
		   * Send a datapacket to the PV
		   * @param packet contains the data, the field code and the continued bit
		   */
		bool SendDataPacket(datapacket& packet);

		/**
		   * This method sets the speed of the serial connection and waits one second as requested in the docs
		   */
		void EndPhase1();

	private:
		unsigned int m_speed;
		unsigned short m_actual_recieve_No;
		unsigned short m_actual_send_No;
		unsigned int m_recievedNAK;
		unsigned int m_sendNAK;
		string m_userid;
		Serial m_com;

		/**
		   * This method converts the int value in the corresponding hex value so that it could be send to the PV.
		   * @return converted value hex->int
		   */
		inline unsigned int HexToInt(unsigned char value){
			value -= 48;
			if ( value >= 17 ) value -= 7;
			return value;
		}

		/**
		   * This method converts the recieved byte from the PV, which is in hex format, in the corresponding int value.
		   * @return converted value int->hex
		   */
		inline unsigned char IntToHex(unsigned int value){
			value += 48;
			if ( value >= 58 ) value += 7;
			return value;
		}

		 /**
		    * This method recieves the actual count of  the packet and store it in m_actual_recieve_No.
		    * @return  false in case of an error and true else.
		    */
		inline unsigned int RecievePacketNo(){
			unsigned int recieveNo_byte1 = HexToInt(m_com.ReadByte());
			unsigned int recieveNo_byte2 = HexToInt(m_com.ReadByte());

			cout << "PacketNo = " << dec << recieveNo_byte1*16 + recieveNo_byte2 << endl;
			return recieveNo_byte1*16 + recieveNo_byte2;
		}

		 /**
		    * This method sends the actual count of  m_actual_send_No.
		    */
		inline void SendPacketNo(unsigned int value){
			m_com.WriteByte(IntToHex((int)value/16));
			m_com.WriteByte(IntToHex(value%16));
			cout << "PacketNo = " << hex << IntToHex((int)value/16) << IntToHex(value%16) << dec << endl;
		}

		 /**
		    * This method sends the checksum in the hex format
		    * @return  false in case of an error and true else.
		    */
		inline void SendChecksumFor(unsigned int checksum){
			checksum--;
			m_com.WriteByte(IntToHex(((int)checksum/16 ^ 0xff)%16));		// Checksum
			m_com.WriteByte(IntToHex(((checksum%16) ^ 0xff)%16));
		}

};

#endif
