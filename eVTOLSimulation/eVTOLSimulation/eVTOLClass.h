#pragma once
/*
 * Name:  eVTOLClass.h
 * Description:  Includes eVTOL class definition.
 * Revision:     Date:           Reason												Author
 * 1.0           Sep. 16, 2023   Original											Chris Wang
 *								 eVTOL, Request and Report class definition
 * 1.1           Sep. 22, 2023   Move "vehicleToDeploy" to here with documented.    Chris Wang
 * 1.2           Sep. 24  2023   Delet "eVTOLChargerMutex" enum						Chris Wang
 * 1.4           Oct   1, 2023   Add READY_TO_DEPLOY_AND_RESERVED as the next		Chris Wang 
 *									state of eVTOL
 */
//#include <iostream>
using namespace std;

#define SIMULATION_HOURS 3
#define MINUTES_PER_HOUR 60
#define SECONDS_PER_MINUTE 60
enum eVTOLCompany { ALPHA_COMPANY, BRAVO_COMPANY, CHARLIE_COMPANY, DELTA_COMPANY, ECHO_COMPANY, MAX_COMPANIES};

// v1.4 add in READY_TO_DEPLOY_AND_RESERVED Oct 1, 2023
enum eVTOLDeployReady { NOT_READY, IN_DEPLOY, IN_CHARGING, READY_TO_DEPLOY, READY_TO_DEPLOY_AND_RESERVED};


class eVTOL {
public:
	int readDeployStatus() { return vehicleReadyToDeploy;  }

	void setDeployStatus(int status) { vehicleReadyToDeploy = status;  }
	bool batteryChargeFull(int chargeTime) {
		return (chargeTime >= (int)(timeToChargeInHour * MINUTES_PER_HOUR * SECONDS_PER_MINUTE)) ? true : false;
	}
	bool flyingTimeReached(int flyingTime) {
		return ((((float)cruiseSpeedMilesPerHour*flyingTime/ MINUTES_PER_HOUR /SECONDS_PER_MINUTE) * eneryUseKWhPerMile) >= (float)batteryCapacityKWh) ? true : false;
	}
	
	std::string readCompanyName() { return name; }
	int readCruiseSpeedMilesPerHour() { return cruiseSpeedMilesPerHour;  }
	int readMaxPassengers() { return maxPassengers;  }
	eVTOL(int readyToDeploy, int company, std::string sname, int cruiseSpeedMPH, int batteryCapKWh, float timeToChargH, float engeryKWhPerMile, int maxPassenger, float probFaultPerHour) {
		vehicleReadyToDeploy = readyToDeploy;
		eVTOLCompany = company;
		name = sname;
		cruiseSpeedMilesPerHour = cruiseSpeedMPH;
		batteryCapacityKWh = batteryCapKWh;
		timeToChargeInHour = timeToChargH;
		eneryUseKWhPerMile = engeryKWhPerMile;
		maxPassengers = maxPassenger;
		probabilityOfFaultPerHour = probFaultPerHour;
	}
	~eVTOL() { };


private:
	// Total one each company's eVTOL in simulation, assume each vehicle is pre-charged.
	// This state machine is eVTOL readiness state machine, it interacts with Request ticket.
	// 1. NOT_READY -- not ready and battery needs to charge
	// 2. IN_DEPLOY -- in flying
	// 3. IN_CHARGING -- previous flight done, back to charger if available; otherwise, this request stays in ACTIVE_REQ and IN_CHARGING states
	//                   if charger is available, it sets to ACTIVE_CHARGING and IN_CHARGING states
	// 4. READY_TO_DEPLOY -- Initially each vehicle is assumed pre-charged and READY_TO_FLY, later to be reserved.
	// 5. READY_TO_DEPLOY_AND_RESERVED -- This vehicle is ready to deploy and is reserved.
	//         `
	int vehicleReadyToDeploy;

	int eVTOLCompany;
	std::string name;
	int cruiseSpeedMilesPerHour;
	int batteryCapacityKWh;  // battery capacity Kilo Watt Hour
	float timeToChargeInHour;
	float eneryUseKWhPerMile; 
	int maxPassengers;
	float probabilityOfFaultPerHour;

};

