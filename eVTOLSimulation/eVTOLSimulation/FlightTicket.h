#pragma once
/*
 * Name:  FlightTicket.h
 * Description:  Flight ticet request and report class definitions.
 * Revision:     Date:           Reason											Author
 * 1.0           Sep 16, 2023    Original										Chris Wang
 * 1.1							 Request and report class definitions
 *
 */
/* 1.  INACTIVE_REQ: associated eVTOL is inactive when this Request is done and this Request is done
 * 2.  ACTIVE_REQ : active request but also need to check associated eVTOL's "vehicleReadyToDeploy" status to determine whether to advance
 *     to "ACTIVE_AND_CHARGING" if charger is available
 * 3.  ACTIVE_AND_CHARGING : vehicle is in charging, only to exit to "ACTIVE_CHARGED_REQ" when battery fully charged
 * 4.  ACTIVE_CHARGED_REQ : interim state if this vehicle is pre - charged or just finised charging, exit to "ACTIVE_FLYING_REQ"
 * 5.  ACTIVE_FLYING_REQ : vehicle is in flying, only to exit to "DONE_REQ" when flight time expires
 * 6.  DONE_REQ : this Request ticket marked as DONE_REQ and accumulated the data to REPORT class,
 *      and it's associated eVTOL vechile's "vehicleReadyToDeploy" as "NOT_READY".This request ticket would not back to active again.
 */
enum REUEST_STATUS { INACTIVE_REQ, ACTIVE_REQ, ACTIVE_AND_CHARGING, ACTIVE_CHARGED_REQ, ACTIVE_FLYING_REQ, DONE_REQ, PROCESSED_REQ };
class eVTOLRequest {
public:

	void clearChargeTime() { chargeTime = 0; }
	int readChargeTime() { return chargeTime; }
	void incrementChargeTimeOneTick() { chargeTime += 1; }

	int readReqStatus() { return active; }
	void setReqStatus(int value) { active = value; }

	void setCompany(int val) { company = val; }
	int readCompany() { return company; }
	string readCompanyName() { return rEVTOL->readCompanyName(); }

	int readFlyingTime() { return flyingTime; }
	void incrementFlightTimeOneTick() { flyingTime += 1; }

	int readFlyingMiles() { return flyingMiles; }
	void setFlyingMiles(int miles) { flyingMiles = miles; }

	void setEVTOLptr(class eVTOL* reqEVTOL) { rEVTOL = reqEVTOL; }
	class eVTOL* readEVTOLPtr() { return rEVTOL; }
	void setDCChargerPtr(class DCFastCharger* charger) { dcCharger = charger; }
	class DCFastCharger* readDCChargerPtr() { return dcCharger; }

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

