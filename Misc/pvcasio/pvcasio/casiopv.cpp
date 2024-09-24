/***************************************************************************
                          casiopv.cpp  -  description
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

// C++ includes
#include <map>
// project includes
#include <pvcasio/ModeCode.h>
#include <pvcasio/FieldCode.h>
#include <pvcasio/casiopv.h>
#include <pvcasio/protocolexception.h>
#include <pvcasio/casiopvexception.h>

/**
   * Constructor.
   */
CasioPV::CasioPV(string port){
	std::cout << "BEGIN:CasioPV::CasioPV()" << std::endl;

	m_protocol = new Protocol::Protocol(port);

	std::cout << "END:CasioPV::CasioPV()"<<std::endl;
}

/**
   * Destructor.
   */
CasioPV::~CasioPV(){
	std::cout << "BEGIN: CasioPV::~CasioPV()" << std::endl;

	delete(m_protocol);

	std::cout << "END: CasioPV::~CasioPV()" << std::endl;
}

/**
   * Make a wake up call to the protocol
   */
bool CasioPV::WakeUp(){
	std::cout << "BEGIN:CasioPV::WakeUp()" << std::endl;

	m_protocol->WakeUp();

	std::cout << "END:CasioPV::WakeUp()" << std::endl;
	return true;
}

/**
   * This method waits for the link packet from the PV, which will be send if the WakeUp() method is called
   * or if the Start-button on the craddle is pressed
   * @param speed can be 38400, 19200 or 9600 (for the PV-450X)
   * @return  false in case of an error and true else.
   * @exception CasioPVException
   */
bool CasioPV::WaitForLink(int speed){
	std::cout << "BEGIN:CasioPV::WaitForLink(int speed)" << std::endl;

	m_protocol->SetRequestedSpeed(speed);
// !!!!!!!!!!!!!!!!!!!!!!! have to add time out !!!!!!!!!!!!!!!!!!!!!!!!!!
	try {
		m_protocol->RecieveLinkPacket();																						// recieve calling up
	} catch (ProtocolException e) {
		throw CasioPVException("CasioPV::WaitForLink : didn't recieve calling up !timeout!");
	}
	m_protocol->SendACK();

	while ( !m_protocol->RecieveCommandPacket() ) {																// recieve protocol level exchange
		m_protocol->SendNAK();
	}
	m_protocol->SendACK();
	do {
		m_protocol->SendCommandPacket(PROTOCOL_LEVEL_EXCHANGE_R);				// send protocol level exchange respond
	} while (!m_protocol->RecieveACK());

	while ( !m_protocol->RecieveCommandPacket() ) {																// recieve UserID
		m_protocol->SendNAK();
	}
	m_protocol->SendACK();
	do {
		m_protocol->SendCommandPacket(USER_ID_EXCHANGE_R);									// send UserID respond
	} while (!m_protocol->RecieveACK());

	while ( !m_protocol->RecieveCommandPacket() ) {																// recieve Com speed settings
		m_protocol->SendNAK();
	}
	m_protocol->SendACK();
	do {
		m_protocol->SendCommandPacket(COMMUNICATION_SPEED_SETTING_R);	// send Com speed settings respond
	} while (!m_protocol->RecieveACK());

	while ( !m_protocol->RecieveCommandPacket() ) {																// recieve Phase transition
		m_protocol->SendNAK();
	}
	m_protocol->SendACK();
	do {
		m_protocol->SendCommandPacket(PHASE_TRANSITION_COMMAND_R);			// send Phase transition for 01
	} while (!m_protocol->RecieveACK());

	m_protocol->EndPhase1();

	std::cout << "END:CasioPV::WaitForLink(int speed)"<<std::endl;
	return true;
}

/**
   * Get number of data in a section
   * @param DataCondition specified through the ModeCode
   * @exception CasioPVException
   */
int CasioPV::GetNumberOfData(int DataCondition){
	std::cout << "BEGIN:CasioPV::GetNumberOfData(int)" << std::endl;

	int NoOfData = 0;
	datapacket packet;

	do {
		m_protocol->SendCommandPacket(NUMBER_OF_DATA_REQUEST, DataCondition);
	} while (!m_protocol->RecieveACK());
	while ( !m_protocol->RecieveCommandPacket() ) {																// recieve start data block
		m_protocol->SendNAK();
	}
	m_protocol->SendACK();

	m_protocol->RecieveDataPacket(packet);																			// recieve number of data

	if ( packet.fieldCode == NUMBER_OF_DATA ) {
//		cout << "number of data = " << packet.data << endl;
		for (int i = 0; i<=5; i++) {
			NoOfData *=16;
			// This calculation is the same as in Protocol::IntToHex
			packet.data[i] -= 48;
			if ( packet.data[i] >= 17 ) packet.data[i] -= 7;
			NoOfData += packet.data[i];
		}
	} else {
		throw CasioPVException("CasioPV::GetNumberOfData : Didn't recieve the number of data!!!!!");
	}
	while ( !m_protocol->RecieveCommandPacket() ) {																// recieve end data block
		m_protocol->SendNAK();
	}
	m_protocol->SendACK();
	while ( !m_protocol->RecieveCommandPacket() ) {																// recieve number of data request result
		m_protocol->SendNAK();
	}
	m_protocol->SendACK();
	std::cout << "END:CasioPV::GetNumberOfData(int)" << std::endl;
	return  NoOfData;
}

/**
   * This method gets the data for the corresponding section
   * which is internaly handled through the data condition (mode code)
   * @param PVDataEntry there can only be derivated classes used because this is an interface
   * the data will be stored in this class
   * @param DataOrder this is the number of the entry which will be downloaded
   * @exception CasioPVException
   */
void CasioPV::GetData(PVDataEntry& dataEntry, unsigned int DataOrder) {
	std::cout << "BEGIN:CasioPV::GetData(PVDataEntry&, unsigned int)" << std::endl;

	datapacket packet;

	m_protocol->SendCommandPacket(DATA_SEND_REQUEST, dataEntry.getModeCode(), DataOrder);
	m_protocol->RecieveACK();
	while ( !m_protocol->RecieveCommandPacket() ) {																// recieve recieve start data block
		m_protocol->SendNAK();
	}
	m_protocol->SendACK();
	if ( m_protocol->RecieveOrder() == 0x02 ) {
		do {
			m_protocol->RecieveDataPacket(packet, true);															// recieve data
			dataEntry.setFieldData( packet );
		} while ( m_protocol->RecieveOrder() == 0x02 );
	} else {
		throw CasioPVException("CasioPV::GetData : Communication error!!");
	}
	bool checkorder = true;
	while ( !m_protocol->RecieveCommandPacket(checkorder) ) {										// recieve end data block
		m_protocol->SendNAK();
		checkorder = false;
	}
	m_protocol->SendACK();
	while ( !m_protocol->RecieveCommandPacket() ) {																// recieve result command
		m_protocol->SendNAK();
	}
	m_protocol->SendACK();
	std::cout << "END:CasioPV::GetData(PVDataEntry&, unsigned int)" << std::endl;
}

/**
   * This method loads the content of a class derivated from PVDataEntry in the corresponding section
   * which is internaly handled through the data condition (mode code)
   * @param PVDataEntry there can only derivated classes be used because this is an interface
   * @exception CasioPVException
   */
void CasioPV::SendData(PVDataEntry& dataEntry){
	std::cout << "BEGIN:CasioPV::SendData(const PVDataEntry&)" << std::endl;

	if ( !dataEntry.isSendable() ) throw CasioPVException("CasioPV::SendData : This DataEntry is not sendable. Some fields are missing");
	do {
		m_protocol->SendCommandPacket(APPEND_REGISTRATION, dataEntry.getModeCode());
	} while (!m_protocol->RecieveACK());
	do {
		m_protocol->SendCommandPacket(START_DATA_BLOCK);
	} while (!m_protocol->RecieveACK());
	for (map<unsigned int, string>::const_iterator packet = dataEntry.getData().begin(); packet != dataEntry.getData().end(); packet++ ){
		int  fieldCode = packet->first;
		string data = packet->second;

		if ( data.size() > 0 ) {
			if ( data.size() > 1024 ) {
				cout << "truncate data to match 1024 bytes" << endl;
				datapacket truncpacket;
				truncpacket.fieldCode = fieldCode;
				unsigned int truncsize;
				int tmp = (data.size()%1024 == 0) ? (unsigned int)data.size()/1024 : (unsigned int)data.size()/1024 + 1;

				for (int i = 0; i<tmp; i++) {
					truncpacket.data = "";
					if ( (data.size()-(1024*i)) > 1024 ) {
						truncsize = 1024;
						truncpacket.continued = true;
					} else {
						truncsize = data.size()-(1024*i);
						truncpacket.continued = false;
					}
					for ( unsigned int j = 0; j<truncsize; j++) {
						truncpacket.data += data[j+(1024*i)];
					}
					m_protocol->SendDataPacket(truncpacket);
				}
			} else {
				datapacket tmp;
				tmp.data = data;
				tmp.fieldCode = fieldCode;
				m_protocol->SendDataPacket(tmp);
			}
		}
	}
	do {
		m_protocol->SendCommandPacket(END_DATA_BLOCK);
	} while (!m_protocol->RecieveACK());
// !!!!!!!!!!!!!!!!!!!! send result command like GetData ??????????????????

	std::cout << "END:CasioPV::SendData(PVDataEntry&)"<<std::endl;
}

/**
   * This method releases the link
   * This is phase 3 as descriped in the documentation
   */
void CasioPV::ReleaseLink() {
	std::cout << "BEGIN:CasioPV::ReleaseLink()" << std::endl;

	m_protocol->SendLinkPacket(END_COMMUNICATION);

	std::cout << "END:CasioPV::ReleaseLink()"<<std::endl;
}
