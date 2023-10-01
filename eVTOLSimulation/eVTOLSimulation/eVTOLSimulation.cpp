//
/*
 * Name:  eVTOLSimulation.cpp
 * Description:  eVTOL simulation 
 *               First, generate a preset number of request to fly tickets.
 *				 Each request is a N-sate machine evaluation in every second interval.
 *               INACTIVE_REQ, ACTIVE_REQ, ACTIVE_AND_CHARGING, ACTIVE_CHARGED_REQ, ACTIVE_FLYING_REQ, DONE_REQ, PROCESSED_REQ
 *               1.  INACTIVE_REQ: associated eVTOL is inactive when this Request is done and this Request is done
 *				 2.  ACTIVE_REQ: active request but also need to check associated eVTOL's "vehicleReadyToDeploy" status to determine whether to advance
 *                   to "ACTIVE_AND_CHARGING" if charger is available
 *				 3.  ACTIVE_AND_CHARGING: vehicle is in charging, only to exit to "ACTIVE_CHARGED_REQ" when battery fully charged
 *				 4.  ACTIVE_CHARGED_REQ: interim state if this vehicle is pre-charged or just finised charging, exit to "ACTIVE_FLYING_REQ"
 *				 5.  ACTIVE_FLYING_REQ: vehicle is in flying, only to exit to "DONE_REQ" when flight time expires
 *				 6.  DONE_REQ:  this Request ticket marked as DONE_REQ and accumulated the data to REPORT class, 
 *					 and it's associated eVTOL vechile's "vehicleReadyToDeploy" as "NOT_READY". This request ticket would not back to active again.			 
 *               
 * Revision:     Date:           Reason												                     Author
 * 1.0           Sep 16, 2023    Original											                     Chris Wang
 * 1.1			 Sep 22, 2023    Move request ticket and report class definitions
 *								 to FlightTicket.h
 *								 Move "vehicleReadyToDeploy" array to a eVTOLClass's 
 *								 public member. It is assumed each eVTOL was charged
 *								 full and be fully charged before deploying.
 *								 Delete un-necessary comments.
 * 1.2			 Sep 24, 2023    Add in "evaluateStationTransion" routine in eVTOLSimulation.cpp
 * 1.3			 Sep 25, 2023    Add in "TwoStreams.h" to write to cout and ostream simultaneously
 * 1.4           Oct 1,  2023    With "READY_TO_DEPLOY_AND_RESERVED" eVTOL state as the next state of "READY_TO_DEPLOY" 
 *								 
 */

//using namespace std;
#define __STDC_WANT_LIB_EXT1__1

#include <chrono>
#include <thread>
#include <random>
#include <ctime>
#include <iomanip>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include "DCFastCharger.h"
#include "eVTOLClass.h"
#include "FlightTicket.h"
#include "TwoStreams.h"

using namespace std::this_thread;     // sleep_for, sleep_until
using namespace std::chrono_literals; // ns, us, ms, s, h, etc.
using std::chrono::system_clock;

//Following objs are declared at the beginning of the code as global variables as static storage, those storage goes away once the exit(0) statement at
// the end of main() routine. Class is declared and initialed by parameterized constructor.
class eVTOL eVTOLAlpha(READY_TO_DEPLOY, ALPHA_COMPANY, "Alpha",        120, 320, 0.6f,  1.6f, 4, 0.25f);
class eVTOL eVTOLBravo(READY_TO_DEPLOY, BRAVO_COMPANY, "Bravo",        100, 100, 0.2f,  1.5f, 5, 0.1f);
class eVTOL eVTOLCharlie(READY_TO_DEPLOY, CHARLIE_COMPANY, "Charlie",  160, 220, 0.8f,  2.2f, 3, 0.05f);
class eVTOL eVTOLDelta(READY_TO_DEPLOY, DELTA_COMPANY, "Delta",         90, 120, 0.62f, 0.8f, 2, 0.22f);
class eVTOL eVTOLEcho(READY_TO_DEPLOY, ECHO_COMPANY, "Echo",            30, 150, 0.3f,  5.8f, 2, 0.61f);
class eVTOL *eVTOLs[MAX_COMPANIES] = { &eVTOLAlpha, &eVTOLBravo, &eVTOLCharlie, &eVTOLDelta, &eVTOLEcho };

// Class is declared and initialed by parameterized constructor.
class eVTOLReport eVTOLReportAlpha(ALPHA_COMPANY, 0, 0, 0, 0, 0, 0);
class eVTOLReport eVTOLReportBravo(BRAVO_COMPANY, 0, 0, 0, 0, 0, 0);
class eVTOLReport eVTOLReportCharlie(CHARLIE_COMPANY, 0, 0, 0, 0, 0, 0);
class eVTOLReport eVTOLReportDelta(DELTA_COMPANY, 0, 0, 0, 0, 0, 0);
class eVTOLReport eVTOLReportEcho(ECHO_COMPANY, 0, 0, 0, 0, 0, 0);
class eVTOLReport* eVTOLReports[MAX_COMPANIES] = { &eVTOLReportAlpha , &eVTOLReportBravo, &eVTOLReportCharlie, &eVTOLReportDelta, &eVTOLReportEcho };

// Initialized the declared eVTOLReq array by constructor.
#define MAX_eVTOLs  20
class eVTOLRequest eVTOLReq[MAX_eVTOLs];  // Initialed by constructor

class DCFastCharger DCFastChargerCruz(CRUZ_ID, (int)READY, "Cruz", 0);
class DCFastCharger DCFastChargerMarina(MARINA_ID, (int)READY, "Marina", 0);
class DCFastCharger DCFastChargerCarlos(CARLOS_ID, (int)READY, "Carlos", 0);
class DCFastCharger *DCchargers[NUMBER_OF_CHARGER] = { &DCFastChargerCruz, &DCFastChargerMarina, &DCFastChargerCarlos };


// Prepare 20 request ticket class array, with only 5 tickets( 5 companies VTOL ) is pre-charged and ready to fly.
// Which company to choose is randomly selected by "rand()" function, however, one or more company vehicles could be not selected.
// Total MAX_VTOLs entries are created, each enty is considered as AVTIVE_REQ and with eVTOL class pointer is associated.
int generateRequestList()
{

	// eVTOL request array
	class eVTOLRequest* newReqPtr = NULL;;
	for (int i = 0 ; i < MAX_eVTOLs; i++) {
		newReqPtr = &eVTOLReq[i];

		newReqPtr->setCompany(rand() % MAX_COMPANIES);
		newReqPtr->setEVTOLptr(eVTOLs[newReqPtr->readCompany()]);  // associated each eVTOL class with this REQ

		newReqPtr->setReqStatus(ACTIVE_REQ); // this request tick is now set as inactive

		newReqPtr->clearChargeTime();
		newReqPtr->setDCChargerPtr(NULL);
		newReqPtr->setFlyingMiles(0);

	} 

	return 0;
}

// Searching maximum created Request available, return the first available "ACTIVE_REQ" request.
class eVTOLRequest *nextActiveReq()
{
	class eVTOLRequest *rp = eVTOLReq;
	for (int i = 0; i < MAX_eVTOLs; i++) {
		if ((rp + i)->readReqStatus() == ACTIVE_REQ)
			return (rp + i);
	}
	return NULL;
}

// Searching dcCharger stations available, return the first available charger pointer.
class DCFastCharger *nextAvailableCharger()
{
	class DCFastCharger *dcChargerPtr = NULL;
	for (int i = 0;  i < NUMBER_OF_CHARGER; i++) {
		dcChargerPtr = DCchargers[i];
		if (dcChargerPtr->readChargerMutex() == READY)
			return dcChargerPtr;
	}
	return NULL;
}

// Report the statistic data once the SIMULATION_HOURS reach.
int reportStatistic(TwoStreams *outfile)
{
	*outfile << "Vehicle - Flights - Average Flight Time - Average Distance Per Flight - Average Charging Time - Total Number of Faults - Total Number of Passenger Miles" << "\n";
	class eVTOLReport* evtolReportPtr = NULL;
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


// State machine of the Request ticket.  Such state machine is evaluated every 10ms per tick by calling "sleep_until(system_clock::now() + 10ms)" for total of SIMULATION_HOURSshal.
// Each tick shall go through the whole Request tickts already allocated.
// Each Request ticket's associated eVTOL's status is also in consideration, and if eVTOL is ready_to_deploy then proceed to "ACTIVED_CHARGED_REQ", and modify eVTOL to "IN_FLIGHT" status.
// 
int evaluateStateTransistion(TwoStreams *outfile)
{
	// loop for every second till 3 hours simulation time
	int tick;

	class eVTOLReport* evtolReportPtr = NULL;
	class eVTOLRequest* newReqPtr = NULL;
	class DCFastCharger* chargerPtr = NULL;
	class eVTOL* localEVTOLPtr = NULL;
	for (tick = 0; tick < (SIMULATION_HOURS * MINUTES_PER_HOUR * SECONDS_PER_MINUTE); tick++) {

		for (int i = 0; i < MAX_eVTOLs; i++) {
			newReqPtr = &eVTOLReq[i];
			switch (newReqPtr->readReqStatus() % MAX_REQS) {  // make sure the ReqStatus falls within MAX_REQS range.
			case INACTIVE_REQ:
				break;

			case ACTIVE_REQ:
				if (newReqPtr->readEVTOLPtr()->readDeployStatus() == READY_TO_DEPLOY) {
					newReqPtr->readEVTOLPtr()->setDeployStatus(READY_TO_DEPLOY_AND_RESERVED);
					newReqPtr->setReqStatus(ACTIVE_CHARGED_REQ);
					*outfile << newReqPtr->readCompanyName() << " Precharged ready to fly\n";
				}
				else if (newReqPtr->readEVTOLPtr()->readDeployStatus() == IN_DEPLOY || newReqPtr->readEVTOLPtr()->readDeployStatus() == IN_CHARGING
						|| newReqPtr->readEVTOLPtr()->readDeployStatus() == READY_TO_DEPLOY_AND_RESERVED) {  // do nothing if was in flying
				}
				else { // not in deploying but ready to charge for next run, the NOT_READY state
					chargerPtr = nextAvailableCharger();
					if (chargerPtr != NULL) { // One charger is available
						newReqPtr->readEVTOLPtr()->setDeployStatus(IN_CHARGING);
						newReqPtr->setReqStatus(ACTIVE_AND_CHARGING);
						newReqPtr->clearChargeTime();
						chargerPtr->setChargerMutex(IN_USE);
						newReqPtr->setDCChargerPtr(chargerPtr);   // associate this charger to this Request
						//newReqPtr->reqEVTOL and newReqPtr->company were already set in generateRequestList
						*outfile << "ACTIVE_REQ = " << newReqPtr << " " << newReqPtr->readCompanyName() << " DCCharger = " << chargerPtr << " " << chargerPtr->readChargerName() << "\n";
					}
				}
				break;

			case ACTIVE_AND_CHARGING:
				// Increment the chargeTime by one tick(one second) first, then compare
				newReqPtr->incrementChargeTimeOneTick();
				if (newReqPtr->readEVTOLPtr()->batteryChargeFull(newReqPtr->readChargeTime()) == true) {
					*outfile << "ACTIVE_AND_CHARGING = " << newReqPtr << " " << newReqPtr->readCompanyName() << " chargeTime = " << newReqPtr->readChargeTime() << " seconds\n";
					newReqPtr->setReqStatus(ACTIVE_CHARGED_REQ);
				}
				break;

			case ACTIVE_CHARGED_REQ:
				// first time pre-charged, the associated DCCharger may be null
				if (newReqPtr->readEVTOLPtr()->readDeployStatus() == READY_TO_DEPLOY_AND_RESERVED) { // first time vehicle already charged
					*outfile << "ACTIVE_CHARGED_REQ = " << newReqPtr << " " << newReqPtr->readCompanyName() << " pre-charged Ready to Fly\n";
				}
				else {
					// This eVTOL is fully charged, accumulates the charging time for release this Charger.
					if (newReqPtr->readDCChargerPtr() != NULL && newReqPtr->readEVTOLPtr()->readDeployStatus() == IN_CHARGING) {
						*outfile << "ACTIVE_CHARGED_REQ = " << newReqPtr << " " << newReqPtr->readCompanyName() << " " << " dcCharger = " << newReqPtr->readDCChargerPtr() << "\n";
						// Now is time to record total time in-use and release associated Charger
						newReqPtr->readDCChargerPtr()->addTotalCharingTime(newReqPtr->readChargeTime());
						newReqPtr->readDCChargerPtr()->setChargerMutex(READY);
					}
				}
				newReqPtr->readEVTOLPtr()->setDeployStatus(IN_DEPLOY); // set it to interim status
				newReqPtr->setReqStatus(ACTIVE_FLYING_REQ);
				break;

			case ACTIVE_FLYING_REQ:
				newReqPtr->incrementFlightTimeOneTick();
				if (newReqPtr->readEVTOLPtr()->flyingTimeReached(newReqPtr->readFlyingTime()) == true) {
					*outfile << "ACTIVE_FLYING_REQ = " << newReqPtr << " " << newReqPtr->readCompanyName() << " flyingTime = " << newReqPtr->readFlyingTime() << " seconds\n";
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
				localEVTOLPtr = newReqPtr->readEVTOLPtr();
				evtolReportPtr->totalPassengerMiles += localEVTOLPtr->readCruiseSpeedMilesPerHour() * localEVTOLPtr->readMaxPassengers() * newReqPtr->readFlyingTime() / MINUTES_PER_HOUR / SECONDS_PER_MINUTE;
				*outfile << "DONE_REQ newReqPtr " << newReqPtr << " evtolReportPtr " << evtolReportPtr << " totalDoneReqs " << evtolReportPtr->totalDoneReq << " " << newReqPtr->readCompanyName() << "\n";

				// when this request is completed, set the vehicleReadyToDeploy NOT_READY
				newReqPtr->readEVTOLPtr()->setDeployStatus(NOT_READY);
				*outfile << newReqPtr->readCompanyName() << " is back to ready but not charged\n";
				break;

			case PROCESSED_REQ:
				break;

			default:
				break;
			}
		}
		sleep_until(system_clock::now() + 10ms);  // changed to 5ms per tick but simulate it as one second


	}
	*outfile << "JobAv eVTOL Simulation Done Tick = " << tick << "\n\n";
	return 0;
}

int main()

{
	std::ofstream fileName;

	fileName.open("JobAvSimu.txt");
	// TwoStreams is a struct of two streams..one is fileName("JobAvSimu.txt") and one is cout.
	TwoStreams myfile(fileName, std::cout);

	myfile << "Start JobAv eVTOL Simulation\n";

	// Declare and get now Date time
	time_t t = time(0);   // get time now
	struct tm* now = localtime(&t);
	myfile << "Date: " << (now->tm_mon + 1) << " " << now->tm_mday << " " << (now->tm_year + 1900) << " Time: " << now->tm_hour << ":" << now->tm_min << ":" << now->tm_sec << "\n";

	// generate the requesst list
	myfile << "Request list generation " << ((generateRequestList() == 0) ? "success\n" : "fail\n");

	// Evaluate each state machine and make the transition, return 0 means success.
	if (evaluateStateTransistion(&myfile) == 0)
	{
		reportStatistic(&myfile);
		myfile << "JobAv eVTOL Simulation JobAvSimu.txt Done Successfully.\n";
	}
	else
		myfile << "JobAv eVTOL Simulation JobAvSimu.txt Done with Error!\n";

	fileName.close();  // close this stream file

	// Exit 0 means success, the system reclaims the memory.
	std::exit(0);
}



