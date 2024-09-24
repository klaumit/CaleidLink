/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Mit Okt 31 16:51:56 CET 2001
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// C++ includes
#include <iostream.h>
#include <iomanip.h>
#include <stdlib.h>
#include <fstream.h>
#include <vector>

// project includes
#include "casiopv.h"
#include "pvcontact.h"
#include "pvexpense.h"
#include "pvmemo.h"
#include "pvpocketsheet.h"
#include "pvquickmemo.h"
#include "pvschedule.h"
#include "pvmultidate.h"
#include "pvreminder.h"
#include "pvtodo.h"
#include "serial.h"
#include "ModeCode.h"

int main(int argc, char *argv[])
{

	CasioPV* casiopv = 0;
	try {
		casiopv = new CasioPV("/dev/ttyS0");
		//casiopv = new CasioPV(argv[1]);
	} catch ( BaseException e ) {
		cerr << "ERROR : Could not open port" << endl;
		return false;
	}
	try {

		std::cout << "Hello, Casio_PV!" << std::endl;

		int category = 0;
		int select_category;
		cout << "Which Category do you want?" << endl
				 << " (1) Contact" << endl
				 << " (2) Memo" << endl
				 << " (3) Scheduler" << endl
				 << " (4) Expense" << endl
				 << " (5) Todo" << endl
				 << " (6) PocketSheet" << endl
				 << " (7) QuickMemo" << endl;
		cin >> select_category;
		int select;
		switch ( select_category ) {
			case 1 :
					cout << "Do you want :" << endl
							 << " (1) Privat Contact" << endl
							 << " (2) Business Contact sec. 1" << endl
							 << " (3) Business Contact sec. 2" << endl
							 << " (4) Business Contact sec. 3" << endl
							 << " (5) Business Contact sec. 4" << endl
							 << "\tonly for PV-750" << endl
							 << " (6) Business Contact sec. 5" << endl
							 << " (7) Business Contact sec. 6" << endl;
					cin >> select;
					switch ( select ) {
							case 1 : category = CONTACT_PRIVATE; break;
							case 2 : category = CONTACT_BUSINESS_1; break;
							case 3 : category = CONTACT_BUSINESS_2; break;
							case 4 : category = CONTACT_BUSINESS_3; break;
							case 5 : category = CONTACT_BUSINESS_4; break;
							case 6 : category = CONTACT_BUSINESS_5; break;
							case 7 : category = CONTACT_BUSINESS_6; break;
							default :
								cerr << "This type of contact does not exist" << endl;
								return 0;
					}
					break;
			case 2 :
					cout << "Do you want :" << endl
							 << " (1) Category 1" << endl
							 << " (2) Category 2" << endl
							 << " (3) Category 3" << endl
							 << " (4) Category 4" << endl
							 << " (5) Category 5" << endl
							 << " (6) Category 6" << endl;
					cin >> select;
					switch ( select ) {
							case 1 : category = MEMO_1; break;
							case 2 : category = MEMO_2; break;
							case 3 : category = MEMO_3; break;
							case 4 : category = MEMO_4; break;
							case 5 : category = MEMO_5; break;
							case 6 : category = MEMO_6; break;
							default :
								cerr << "This type of memo does not exist" << endl;
								return 0;
					}
					break;
			case 3 :
					cout << "Do you want :" << endl
							 << " (1) Scheduler" << endl
							 << " (2) Multi-date reminder" << endl
							 << " (3) Schedule reminder" << endl;
					cin >> select;
					switch ( select ) {
							case 1 : category = SCHEDULE; break;
							case 2 : category = SCHEDULE_MULTI_DATE; break;
							case 3 : category = SCHEDULE_REMINDER; break;
							default :
								cerr << "This type of schedule does not exist" << endl;
								return 0;
					}
					break;
			case 4 :
					category = EXPENSE_PV;
					break;
			case 5 :
					category = TODO;
					break;
			case 6 :
					category = POCKET_SHEET_PV;
					break;
			case 7 :
					category = QUICK_MEMO;
					break;
		default :
			cerr << "The category " << select_category << " does not exist" << endl;
		}

// download the data of the selected category
		casiopv->WakeUp();
		casiopv->WaitForLink(38400);
		cout << "Jetzt kommen Daten" << endl;
		int NoOfData =  casiopv->GetNumberOfData(category);
		cout << "********************************************"<<endl;
		cout << "NoOfData = " << NoOfData << endl;
		cout << "********************************************"<<endl;
		vector<PVDataEntry*> data(NoOfData);
		for (int i =  0; i<NoOfData; i++){
			cout << "***************************************cycle****************************" << endl;
			// create new data element
			switch ( category ) {
				case CONTACT_PRIVATE :
				case CONTACT_BUSINESS_1 :
				case CONTACT_BUSINESS_2 :
				case CONTACT_BUSINESS_3 :
				case CONTACT_BUSINESS_4 :
//	 only for PV-750
				case CONTACT_BUSINESS_5 :
				case CONTACT_BUSINESS_6 :
						data[i] = new PVContact(category); break;
						break;
				case MEMO_1 :
				case MEMO_2 :
				case MEMO_3 :
				case MEMO_4 :
				case MEMO_5 :
//?????????????????????????????????????????? Gibts den????????????????????????????
				case MEMO_6 :
						data[i] = new PVMemo(category);
						break;
				case SCHEDULE :
						data[i] = new PVSchedule();
						break;
				case SCHEDULE_MULTI_DATE :
						data[i] = new PVMultiDate();
						break;
				case SCHEDULE_REMINDER :
						data[i] = new PVReminder();
						break;
				case EXPENSE_PV :
						data[i] = new PVExpense();
						break;
				case TODO :
						data[i] = new PVTodo();
						break;
				case POCKET_SHEET_PV :
						data[i] = new PVPocketSheet();
						break;
				case QUICK_MEMO :
						data[i] = new PVQuickMemo();
						break;
			}
			casiopv->GetData(*data[i], i);
		}

		casiopv->ReleaseLink();

// output of the downloaded data
		for (int i =  0; i<NoOfData; i++){
			cout << "***************************************cycle****************************" << endl;
			switch ( category ) {
				case CONTACT_PRIVATE :
				case CONTACT_BUSINESS_1 :
				case CONTACT_BUSINESS_2 :
				case CONTACT_BUSINESS_3 :
				case CONTACT_BUSINESS_4 :
// only for PV-750
				case CONTACT_BUSINESS_5 :
				case CONTACT_BUSINESS_6 :
						cout << "Entry No: " << i << endl << *dynamic_cast<PVContact*>(data[i]);
						break;
				case MEMO_1 :
				case MEMO_2 :
				case MEMO_3 :
				case MEMO_4 :
				case MEMO_5 :
//?????????????????????????????????????????? Gibts den????????????????????????????
				case MEMO_6 :
						cout << "Entry No: " << i << endl << *dynamic_cast<PVMemo*>(data[i]);
						break;
				case SCHEDULE :
						cout << "Entry No: " << i << endl << *dynamic_cast<PVSchedule*>(data[i]);
						break;
				case SCHEDULE_MULTI_DATE :
						cout << "Entry No: " << i << endl << *dynamic_cast<PVMultiDate*>(data[i]);
						break;
				case SCHEDULE_REMINDER :
						cout << "Entry No: " << i << endl << *dynamic_cast<PVReminder*>(data[i]);
						break;
				case EXPENSE_PV :
						cout << "Entry No: " << i << endl << *dynamic_cast<PVExpense*>(data[i]);
						break;
				case TODO :
						cout << "Entry No: " << i << endl << *dynamic_cast<PVTodo*>(data[i]);
						break;
				case POCKET_SHEET_PV :
						cout << "Entry No: " << i << endl << *dynamic_cast<PVPocketSheet*>(data[i]);
						break;
				case QUICK_MEMO :
						cout << "Entry No: " << i << endl << *dynamic_cast<PVQuickMemo*>(data[i]);
						string tmp;
						int tmpi = i;
						if ( tmpi == 0 ) tmp = "0";
						while (tmpi>0) {
							switch ( tmpi%10){
								case 0: tmp = "0" +tmp; break;
								case 1: tmp = "1" +tmp; break;
								case 2: tmp = "2" +tmp; break;
								case 3: tmp = "3" +tmp; break;
								case 4: tmp = "4" +tmp; break;
								case 5: tmp = "5" +tmp; break;
								case 6: tmp = "6" +tmp; break;
								case 7: tmp = "7" +tmp; break;
								case 8: tmp = "8" +tmp; break;
								case 9: tmp = "9" +tmp; break;
							}
							tmpi = (int)tmpi/10;
						}
						tmp += "-";
						tmpi = atoi(dynamic_cast<PVQuickMemo*>(data[i])->getCategory().c_str());
						string tmp2;
						if ( tmpi == 0 ) tmp2 = "0";
						while (tmpi>0) {
							switch ( tmpi%10){
								case 0: tmp2 = "0" +tmp2; break;
								case 1: tmp2 = "1" +tmp2; break;
								case 2: tmp2 = "2" +tmp2; break;
								case 3: tmp2 = "3" +tmp2; break;
								case 4: tmp2 = "4" +tmp2; break;
								case 5: tmp2 = "5" +tmp2; break;
								case 6: tmp2 = "6" +tmp2; break;
								case 7: tmp2 = "7" +tmp2; break;
								case 8: tmp2 = "8" +tmp2; break;
								case 9: tmp2 = "9" +tmp2; break;
							}
							tmpi = (int)tmpi/10;
						}
						tmp += tmp2+".bmp";
						cout << "file : " << tmp;
						ofstream ofile(tmp.c_str());
						ofile <<  *dynamic_cast<PVQuickMemo*>(data[i]);

						break;
			}
		}    

/*		PVContact contact(CONTACT_PRIVATE);
		string name, address;
		int entryNo = 0xffffff;
		cout << "Name : ";
		cin >> name;
		cout << "Address : ";
		cin >> address;

		contact.setName(name);
		contact.setAddress(address);
		if ( !contact.isSendable() ) {
			cerr << "ERROR : Data Entry is not sendable !" << endl;
			casiopv->ReleaseLink();
			return false;
		}

		casiopv->WakeUp();
		casiopv->WaitForLink(38400);

		//PVMemo memo(MEMO_1);
		string senddata;
		//senddata.fieldCode = 0x00000;
		senddata = "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789";
		for (unsigned int i = 0; i<5; i++){
			senddata += senddata;
		}
		//memo.setMemo(senddata);
		//casiopv->SendData(memo);


		casiopv->SendData(contact);

		casiopv->ReleaseLink();
*/

// test for sync with kaddressbook

/*		ofstream ofile("/home/michael/addressbook.csv");
		ofile << "\"First Name\",\"Last Name\",\"Middle Name\",\"Name Prefix\",\"Job Title\",\"Company\",\"Email\",\"Nickname\",\"Note\",\"Business Phone\",\"Home Phone\",\"Mobile Phone\",\"Home Fax\",\"Business Fax\",\"Pager\",\"Street Home Address\",\"City Home Address\",\"State Home Address\",\"Zip Home Address\",\"Country Home Address\",\"Street Business Address\",\"City Business Address\",\"State Business Address\",\"Zip Business Address\",\"Country Business Address\"" <<endl;
		casiopv->WakeUp();
		casiopv->WaitForLink(38400);
		cout << "Jetzt kommen Daten" << endl;
		int NoOfData =  casiopv->GetNumberOfData(CONTACT_PRIVATE);
		cout << "********************************************"<<endl;
		cout << "NoOfData = " << NoOfData << endl;
		cout << "********************************************"<<endl;
		for (int i =  0; i<NoOfData; i++){
			cout << "***************************************cycle****************************" << endl;
			PVContact contact(CONTACT_PRIVATE);
			casiopv->GetData(contact, i);

			cout << contact << endl;
			ofile << "\"" << contact.getName() << "\",\"\",\"\",\"\",\""<< contact.getPosition() << "\",\"" << contact.getBusinessAddress() << "\",\""
				<< contact.getEmail() << "\",\"\",\"" << contact.getNote() << "\",\"" << contact.getBusinessNumber() << "\",\"" << contact.getHomeNumber() << "\",\""
				<< contact.getMobile() << "\",\"" << contact.getFaxNumber() << "\",\"" << contact.getBusinessFax() << "\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\"," << endl;
		}
		casiopv->ReleaseLink();*/

	} catch ( BaseException e ) {
		cerr << "ERROR : " << e.getMessage() << endl;
		casiopv->ReleaseLink();
	}
	delete( casiopv );

	return EXIT_SUCCESS;
}
