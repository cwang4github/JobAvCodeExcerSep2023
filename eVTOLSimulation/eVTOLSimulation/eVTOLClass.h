#pragma once
/*
 * Name:  eVTOLClass.h
 * Description:  Includes eVTOL class definition.
 * Revision:     Date:           Reason												Author
 * 1.0           Sep 16, 2023    Original											Chris Wang
 *								 eVTOL, Request and Report class definition
 */
//#include <iostream>
//using namespace std;

#define SIMULATION_HOURS 3
#define MINUTES_PER_HOUR 60
#define SECONDS_PER_MINUTE 60
enum eVTOLChargeMutex { eVTOL_NO_CHARGE, eVTOL_IN_CHARGING, eVTOL_FULL_CHARGED};
enum eVTOLCCompany { ALPHA_COMPANY, BRAVO_COMPANY, CHARLIE_COMPANY, DELTA_COMPANY, ECHO_COMPANY, MAX_COMPANIES};
class eVTOL {
public:
	bool batteryChargeFull(int chargeTime) {
		return (chargeTime >= (int)(timeToChargeInHour * MINUTES_PER_HOUR * SECONDS_PER_MINUTE)) ? true : false;
	}
	bool flyingTimeReached(int flyingTime) {
		return ((float)cruiseSpeedMilesPerHour * eneryUseKWhPerMile * ((float)flyingTime/MINUTES_PER_HOUR/SECONDS_PER_MINUTE) >= (float)batteryCapacityKWh) ? true : false;
	}
	
	string readCompanyName() { return name; }
	int readCruiseSpeedMilesPerHour() { return cruiseSpeedMilesPerHour;  }
	eVTOL(int company, string sname, int cruiseSpeedMPH, int batteryCapKWh, float timeToChargH, float engeryKWhPerMile, int maxPassenger, float probFaultPerHour) {

		eVTOLCompany = company;
		name = sname;
		cruiseSpeedMilesPerHour = cruiseSpeedMPH;
		batteryCapacityKWh = batteryCapKWh;
		timeToChargeInHour = timeToChargH;
		eneryUseKWhPerMile = engeryKWhPerMile;
		maxPassergers = maxPassenger;
		probabilityOfFaultPerHour = probFaultPerHour;
	}
	~eVTOL() { };

private:
	int eVTOLCompany;
	string name;
	int cruiseSpeedMilesPerHour;
	int batteryCapacityKWh;  // battery capacity Kilo Watt Hour
	float timeToChargeInHour;
	float eneryUseKWhPerMile; 
	int maxPassergers;
	float probabilityOfFaultPerHour;

};

enum REUEST_STATUS { INACTIVE_REQ, ACTIVE_REQ, ACTIVE_AND_CHARGING, ACTIVE_CHARGED_REQ, ACTIVE_FLYING_REQ, DONE_REQ, PROCESSED_REQ};
class eVTOLRequest {
public:
	
	void clearChargeTime() { chargeTime = 0;  }
	int readChargeTime() { return chargeTime;  }
	void incrementChargeTimeOneTick() { chargeTime += 1; }

	int readReqStatus() { return active;  }
	void setReqStatus(int value) { active = value; }

	void setCompany(int val ) { company = val; }
	int readCompany() { return company; }
	string readCompanyName() { return rEVTOL->readCompanyName(); }

	int readFlyingTime() { return flyingTime; }
	void incrementFlightTimeOneTick() { flyingTime += 1; }

	int readFlyingMiles() { return flyingMiles; }
	void setFlyingMiles(int miles) { flyingMiles = miles;  }

	void setEVTOLptr(class eVTOL* reqEVTOL) { rEVTOL = reqEVTOL; }
	class eVTOL* readEVTOLPtr() { return rEVTOL; }
	void setDCChargerPtr(class DCFastCharger* charger) { dcCharger = charger;  }
	class DCFastCharger* readDCChargerPtr() { return dcCharger;  }
	
private:
	int active;  // Is this request active?
	int chargeTime;   // how much charging time elapse
	int company;      // which eVTOL type
	int flyingTime;   // how much flying time elapse
	int flyingMiles;  // how much flying miles
	class eVTOL* rEVTOL;  // assoicated eVTOL object pointer
	class DCFastCharger* dcCharger;  // assoicated DCCharger object pointer
};

// TODO --move public data to private in next rev
class eVTOLReport {
public:
	int company;  // company type
	int totalFlyTime; // fly time
	int totalFlyMiles;  // fly miles
	int totalChargeTime;  // charge time
	int totalFaults;      // total faults
	int totalPassengerMiles;  // total passenger miles
	int totalDoneReq;		  // total done request

	eVTOLReport(int rcompany, int flytime, int flymiles, int chargetime, int faults, int passengerMiles, int donereq) {
		company = rcompany;
		totalFlyTime = flytime;
		totalFlyMiles = flymiles;
		totalChargeTime = chargetime;
		totalFaults = faults;
		totalPassengerMiles = passengerMiles;
		totalDoneReq = donereq;
	}

	~eVTOLReport() {};
private:

};
