//
/*
 * Name:  eVTOLSimulation.cpp
 * Description:  eVTOL simulation 
 *				 Each request is a N-sate machine evaluation in every second interval.
 *               INACTIVE_REQ, ACTIVE_REQ, ACTIVE_AND_CHARGING, ACTIVE_CHARGED_REQ, ACTIVE_FLYING_REQ, DONE_REQ, PROCESSED_REQ
 *               Starts with one each company one vehicle pre-charged, once flying completed, push to charging next. 
 *				 Simulation output to JobAvSimu.txt file. 
 *				 
 *               
 * Revision:     Date:           Reason												Author
 * 1.0           Sep 16, 2023    Original											Chris Wang
 *								 
 */
using namespace std;
#include <fstream>
#include <iostream>
#include "DCFastCharger.h"
#include "eVTOLClass.h"

#include <chrono>
#include <thread>
#include <random>
#include <ctime>
#include <iomanip>
#include <string>

using namespace std::this_thread;     // sleep_for, sleep_until
using namespace std::chrono_literals; // ns, us, ms, s, h, etc.
using std::chrono::system_clock;

//Following objs are declared at the beginning of the code as global variables as static storage, those storage goes away once the exit(0) statement at
// the end of main() routine.
class eVTOL eVTOLAlpha( ALPHA_COMPANY, "Alpha",       120, 320, 0.6f, 1.6f, 4, 0.25f);
class eVTOL eVTOLBravo( BRAVO_COMPANY, "Bravo",       100, 100, 0.2f, 1.5f, 5, 0.1f);
class eVTOL eVTOLCharlie( CHARLIE_COMPANY, "Charlie", 160, 220, 0.8f, 2.2f, 3, 0.05f);
class eVTOL eVTOLDelta(DELTA_COMPANY, "Delta",         90, 120, 0.62f, 0.8f, 2, 0.22f);
class eVTOL eVTOLEcho( ECHO_COMPANY, "Echo",           30, 150, 0.3f, 5.8f, 2, 0.61f);
class eVTOL *eVTOLs[MAX_COMPANIES] = { &eVTOLAlpha, &eVTOLBravo, &eVTOLCharlie, &eVTOLDelta, &eVTOLEcho };

class eVTOLReport eVTOLReportAlpha(ALPHA_COMPANY, 0, 0, 0, 0, 0, 0);
class eVTOLReport eVTOLReportBravo(BRAVO_COMPANY, 0, 0, 0, 0, 0, 0);
class eVTOLReport eVTOLReportCharlie(CHARLIE_COMPANY, 0, 0, 0, 0, 0, 0);
class eVTOLReport eVTOLReportDelta(DELTA_COMPANY, 0, 0, 0, 0, 0, 0);
class eVTOLReport eVTOLReportEcho(ECHO_COMPANY, 0, 0, 0, 0, 0, 0);
class eVTOLReport *eVTOLReports[MAX_COMPANIES] = { &eVTOLReportAlpha , &eVTOLReportBravo, &eVTOLReportCharlie, &eVTOLReportDelta, &eVTOLReportEcho };
#define MAX_eVTOLs  20
class eVTOLRequest eVTOLReq[MAX_eVTOLs];  // Maximum ready eVTOL's, initialized to NULL

class DCFastCharger DCFastChargerCruz(CRUZ_ID, (int)READY, "Cruz", 0);
class DCFastCharger DCFastChargerMarina(MARINA_ID, (int)READY, "Marina", 0);
class DCFastCharger DCFastChargerCarlos(CARLOS_ID, (int)READY, "Carlos", 0);
class DCFastCharger *DCchargers[NUMBER_OF_CHARGER] = { &DCFastChargerCruz, &DCFastChargerMarina, &DCFastChargerCarlos };

// Total one each company's eVTOL in simulation, assume each vehicle is pre-charged.
// Once it is deployed, should set it to false, and start next REQ to ACTIVE but not charged
bool vechicleReadyToDeploy[MAX_COMPANIES] = { true, true, true, true, true };

// Prepare 20 request ticket class array, with only 5 tickets( 5 companies VTOL ) is pre-charged and ready to fly.
int generateRequestList()
{

	// eVTOL request array
	class eVTOLRequest* newReqPtr;
	for (int i = 0 ; i < MAX_eVTOLs; i++) {
		newReqPtr = &eVTOLReq[i];

		newReqPtr->setCompany(rand() % MAX_COMPANIES);
		if (vechicleReadyToDeploy[newReqPtr->readCompany()] == true) {
			newReqPtr->setReqStatus(ACTIVE_CHARGED_REQ); // assume each vehicle is active and charged
			vechicleReadyToDeploy[newReqPtr->readCompany()] = false;
		}
		else
			newReqPtr->setReqStatus(INACTIVE_REQ); // this request tick is now set as inactive

		newReqPtr->setEVTOLptr(eVTOLs[newReqPtr->readCompany()]);  // associated each eVTOL class with this REQ
		newReqPtr->clearChargeTime();
		newReqPtr->setDCChargerPtr(NULL);
		newReqPtr->setFlyingMiles(0);

	} 

	return 0;
}

class eVTOLRequest *nextActiveReq()
{
	class eVTOLRequest *rp = eVTOLReq;
	for (int i = 0; i < MAX_eVTOLs; i++) {
		if ((rp + i)->readReqStatus() == ACTIVE_REQ)
			return (rp + i);
	}
	return NULL;
}




class DCFastCharger *nextAvailableCharger()
{
	class DCFastCharger *dcChargerPtr;
	for (int i = 0;  i < NUMBER_OF_CHARGER; i++) {
		dcChargerPtr = DCchargers[i];
		if (dcChargerPtr->readChargerMutex() == READY)
			return dcChargerPtr;
	}
	return NULL;
}

int reportStatistic(ofstream *outfile)
{
	*outfile << "Vehicle - Flights - Average Flight Time - Average Distance Per Flight - Average Charging Time - Total Number of Faults - Total Number of Passenger Miles" << "\n";
	class eVTOLReport* evtolReportPtr;
	for (int i = 0; i < MAX_COMPANIES; i++) {
		evtolReportPtr = eVTOLReports[i];
		*outfile << "\n" << eVTOLs[evtolReportPtr->company]->readCompanyName() << "       ";


		*outfile << evtolReportPtr->totalDoneReq << "        ";

		if (evtolReportPtr->totalDoneReq == 0) { // prevent divided by zero error
			continue;
		}

		*outfile << evtolReportPtr->totalFlyTime/evtolReportPtr->totalDoneReq << "                                   ";
		
		*outfile << evtolReportPtr->totalFlyMiles/evtolReportPtr->totalDoneReq << "                    ";

		*outfile << evtolReportPtr->totalChargeTime / evtolReportPtr->totalDoneReq << "                    ";

		*outfile << evtolReportPtr->totalFaults << "                              ";

		*outfile << evtolReportPtr->totalPassengerMiles << "     ";

		if (i == (MAX_COMPANIES - 1) ) {
			*outfile << "\n";
			*outfile << "\n";
		}
	}

	// TODO -- printout each charger statistic 
	for (int j = 0; j < NUMBER_OF_CHARGER; j++) {

	}
	return 0;
}


int main()
{
	// Open a file
	ofstream myfile;
	myfile.open("JobAvSimu.txt");

	// TODO -- merge both cout and myfile in one combo stream
	myfile << "Start JobAv eVTOL Simulation\n";
	myfile << "Start JobAv eVTOL Simulation\n";

	// Declare and get now Date time
	time_t t = time(0);   // get time now
	struct tm * now = localtime(&t);
	myfile << "Date: " << (now->tm_mon + 1) << " " << now->tm_mday << " " << (now->tm_year + 1900) << " Time: " << now->tm_hour << ":" << now->tm_min << ":" << now->tm_sec << endl;
	std::cout << "Date: " << (now->tm_mon + 1) << " " << now->tm_mday << " " << (now->tm_year + 1900) << " Time: " << now->tm_hour << ":" << now->tm_min << ":" << now->tm_sec << endl;

	// generate the requesst list
	generateRequestList();

	// loop for every second till 3 hours simulation time
	int tick;

	class eVTOLReport *evtolReportPtr;
	class eVTOLRequest* newReqPtr;
	class DCFastCharger* chargerPtr;
	for (tick = 0; tick < (SIMULATION_HOURS * MINUTES_PER_HOUR * SECONDS_PER_MINUTE); tick++) {

		for (int i = 0; i < MAX_eVTOLs; i++) {
			newReqPtr = &eVTOLReq[i];
			switch (newReqPtr->readReqStatus()) {
			case INACTIVE_REQ:
				if (vechicleReadyToDeploy[newReqPtr->readCompany()] == true) {
					newReqPtr->setReqStatus(ACTIVE_REQ);
					myfile << newReqPtr->readCompanyName() << " ACT_REQ again to charge\n";
					vechicleReadyToDeploy[newReqPtr->readCompany()] = false; // reset it back to false
				}
				else 
					break;

			case ACTIVE_REQ:
				chargerPtr = nextAvailableCharger();
				if (chargerPtr != NULL) { // One charger is available
					newReqPtr->setReqStatus(ACTIVE_AND_CHARGING);
					newReqPtr->clearChargeTime();
					chargerPtr->setChargerMutex(IN_USE);
					newReqPtr->setDCChargerPtr(chargerPtr);   // associate this charger to this Request
					//newReqPtr->reqEVTOL and newReqPtr->company were already set in generateRequestList
					myfile << "ACTIVE_REQ = " << newReqPtr << " " << newReqPtr->readCompanyName() << " DCCharger = " << chargerPtr << " " << chargerPtr->readChargerName() << "\n";
				}
				break;

			case ACTIVE_AND_CHARGING:
				// Increment the chargeTime by one tick(one second) first, then compare
				newReqPtr->incrementChargeTimeOneTick();
				if (newReqPtr->readEVTOLPtr()->batteryChargeFull(newReqPtr->readChargeTime()) == true) {
					myfile << "ACTIVE_AND_CHARGING = " << newReqPtr << " " << newReqPtr->readCompanyName() << " chargeTime = " << newReqPtr->readChargeTime() << " seconds\n";
					newReqPtr->setReqStatus(ACTIVE_CHARGED_REQ);
				}
				break;

			case ACTIVE_CHARGED_REQ:
				newReqPtr->setReqStatus(ACTIVE_FLYING_REQ);
				if (newReqPtr->readDCChargerPtr() == NULL) { // first time vechicle already charged
					myfile << "ACTIVE_CHARGED_REQ = " << newReqPtr << " " << newReqPtr->readCompanyName() << " pre-charged Ready to Fly\n";
				}
				else {
					myfile << "ACTIVE_CHARGED_REQ = " << newReqPtr << " " << newReqPtr->readCompanyName() << " " << " dcCharger = " << newReqPtr->readDCChargerPtr() << "\n";
					// Now is time to record total time in-use and release associated Charger
					newReqPtr->readDCChargerPtr()->addTotalCharingTime(newReqPtr->readChargeTime());
					newReqPtr->readDCChargerPtr()->setChargerMutex(READY);
				}
				break;

			case ACTIVE_FLYING_REQ:
				newReqPtr->incrementFlightTimeOneTick();
				if (newReqPtr->readEVTOLPtr()->flyingTimeReached(newReqPtr->readFlyingTime()) == true) {
					myfile << "ACTIVE_FLYING_REQ = " << newReqPtr << " " << newReqPtr->readCompanyName() << " flyingTime = " << newReqPtr->readFlyingTime() << " seconds\n";
					newReqPtr->setReqStatus(DONE_REQ);
				}
				break;

			case DONE_REQ:  // Report this Request statistic
				
				newReqPtr->setReqStatus(PROCESSED_REQ);
				evtolReportPtr = eVTOLReports[newReqPtr->readCompany()];
				evtolReportPtr->totalDoneReq++;
				evtolReportPtr->company = newReqPtr->readCompany();
				evtolReportPtr->totalChargeTime += newReqPtr->readChargeTime();
				evtolReportPtr->totalFlyTime += newReqPtr->readFlyingTime();
				evtolReportPtr->totalFlyMiles += newReqPtr->readFlyingTime() * newReqPtr->readEVTOLPtr()->readCruiseSpeedMilesPerHour() / MINUTES_PER_HOUR / SECONDS_PER_MINUTE;
				myfile << "DONE_REQ newReqPtr " << newReqPtr << " evtolReportPtr " << evtolReportPtr << " totalDoneReqs " << evtolReportPtr->totalDoneReq << " " << newReqPtr->readCompanyName() << "\n";
				
				// when this request is completed, set the vechicleReadyToDeploy back to true again, and next to 
				vechicleReadyToDeploy[newReqPtr->readCompany()] = true;
			    myfile << newReqPtr->readCompanyName() << " is back to ready but not charged\n";				
				break;

			case PROCESSED_REQ:
				break;

			default:
				break;
			}
		}
		sleep_until(system_clock::now() + 10ms);  // changed to 5ms per tick but simulate it as one second
		

	}

	myfile << "JobAv eVTOL Simulation Done Tick = " << tick << "\n";

	reportStatistic(&myfile);
	
	myfile.close();  // close this stream file
	std::cout << "JobAv eVTOL Simulation JobAvSimu.txt Done Tick = " << tick << "\n";
	std::exit(0);
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
