#pragma once
/*
 * Name:  DCFastCharger.h
 * Description:  Includes DC Fast charging station class definition.
 * Revision:     Date:           Reason                               Author
 * 1.0           Sep 16, 2023    Original                             Chris Wang* 1.0           
 *								 DCCharger class definition
 *
 */

#define NUMBER_OF_CHARGER 3
enum chargerID { CRUZ_ID, MARINA_ID, CARLOS_ID};
enum charegerMutex { READY, IN_USE }; // Mutex of this charger, could be expanded to complicate

class DCFastCharger {
public:
	int readChargerMutex() { return chargerMutex;  }
	void setChargerMutex(int value) { chargerMutex = value;  }
	int readChargerID() { return chargerID; }
	std::string readChargerName() { return name; }
	void addTotalCharingTime(int chargingTime) { totalChargingTime += chargingTime;  }

	DCFastCharger(int chgID, int mutex, std::string sname, int chargeTime) {
		chargerMutex = mutex;
		chargerID = chgID;
		name = sname;
		totalChargingTime = chargeTime;
	}
	~DCFastCharger() {}
private:
	int chargerID;
	int chargerMutex;
	std::string name;
	int totalChargingTime;   // how much time this charger in-use


};