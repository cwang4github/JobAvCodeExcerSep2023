#pragma once
/*
 * Name:  FlightTicket.h
 * Description:  Flight ticet request and report class definitions.
 * Revision:     Date:           Reason											Author
 * 1.0           Sep 16, 2023    Original										Chris Wang
 * 1.1							 Request and report class definitions
 * 1.4           Sep 27, 2023    Add in constructors for Request ticket class    Chris Wang
 *
 */
/* 1.  INACTIVE_REQ:  NULL request state
 * 2.  ACTIVE_REQ : active request but also need to check associated eVTOL's "vehicleReadyToDeploy" status, 
 *     if READY_TO_DEPLOY, then set to READY_TO_DEPLOY_AND_RESERVED
 *     if IN_CHARGING or IN_DEPLOY, then skip
 *     if NOT_READY(means need to charge), then check if one charger is avaible
 *			if one charger is available, then set this eVTOL vehicle is "IN_CHARGING", associate this charger with this REQuest, and set this REQuest to "ACTIVE_AND_CHARGING"
 *			if no charger is availble, skip
 * 3.  ACTIVE_AND_CHARGING : vehicle is in charging, only to exit to "ACTIVE_CHARGED_REQ" when battery fully charged, else stay in this state
 * 4.  ACTIVE_CHARGED_REQ : interim state 
 *			if this vehicle is pre-charged(READY_TO_DEPLOY_AND_RESERVED) then proceed to "ACTIVE_FLYING_REQ" 
 *			if this vehicle is charged, release this charger
 *     set the REQ to "ACTIVE_FLYING_REQ" and set this eVTOL vehicle to "IN_DEPLOY"
 * 5.  ACTIVE_FLYING_REQ : vehicle is in flying, only to exit to "DONE_REQ" when flight time expires
 * 6.  DONE_REQ : this Request ticket marked as DONE_REQ and accumulated the data to REPORT class,
 *      and it's associated eVTOL vechile's "vehicleReadyToDeploy" as "NOT_READY"( this vehicle will be picked by next REQ in "ACTIVE_REQ" state
 *     This request ticket then be set to "PROCESSED_REQ" and will not be activated again.
 * 7.  PROCESSED_REQ: The final state of this REQ ticket, all of the related flight informaiton were accumulated, this REQ ticket will not be activated again.
 */



enum REUEST_STATUS { INACTIVE_REQ, ACTIVE_REQ, ACTIVE_AND_CHARGING, ACTIVE_CHARGED_REQ, ACTIVE_FLYING_REQ, DONE_REQ, PROCESSED_REQ, MAX_REQS };
class eVTOLRequest {
public:

	void clearChargeTime() { chargeTime = 0; }
	int readChargeTime() { return chargeTime; }
	void incrementChargeTimeOneTick() { chargeTime += 1; }

	int readReqStatus() { return active; }
	void setReqStatus(int value) { active = value; }

	void setCompany(int val) { company = val; }
	int readCompany() { return company; }
	std::string readCompanyName() { return rEVTOL->readCompanyName(); }

	int readFlyingTime() { return flyingTime; }
	void incrementFlightTimeOneTick() { flyingTime += 1; }

	int readFlyingMiles() { return flyingMiles; }
	void setFlyingMiles(int miles) { flyingMiles = miles; }

	void setEVTOLptr(class eVTOL* reqEVTOL) { rEVTOL = reqEVTOL; }
	class eVTOL* readEVTOLPtr() { return rEVTOL; }
	void setDCChargerPtr(class DCFastCharger* charger) { dcCharger = charger; }
	class DCFastCharger* readDCChargerPtr() { return dcCharger; }

	// Polymorphism constructors
	eVTOLRequest(int act, int chargeT, int rcompany, int flyingT, int flyingM, class eVTOL* evp, class DCFastCharger* dcCharge) {
		this->active = act;
		this->chargeTime = chargeT;
		this->company = rcompany;
		this->flyingTime = flyingT;
		this->flyingMiles = flyingM;
		this->rEVTOL = evp;
		this->dcCharger = dcCharge;
	}
	eVTOLRequest() {
		this->active = 0;  
		this->chargeTime = 0;  
		this->company = 0;  
		this->flyingTime = 0;  
		this->flyingMiles = 0;
		this->rEVTOL = NULL;
		this->dcCharger = NULL;
	}
	
	~eVTOLRequest() {}; 
	

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

	template <typename T> 
	T addTwo(T a, T b) { return (a + b); }

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

