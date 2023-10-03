/*
 * Name:  MSTest.cpp
 * Description:  Microsoft Visual Studio Unit Test C++ 
 *
 * Revision:     Date:           Reason												                      Author
 * 1.0           Oct 2, 2023     Original											                      Chris Wang 
 *                               First revision unit test on Classes' constructor and one simple method
 *                                 by "generateReqList" routine.
 */
#include "pch.h"
#include "CppUnitTest.h"
#include "../eVTOLSimulation/DCFastCharger.h"
#include "../eVTOLSimulation/eVTOLClass.h"
#include "../eVTOLSimulation/FlightTicket.h"
#include "../eVTOLSimulation/TwoStreams.h"
#include "../eVTOLSimulation/eVTOLSimulation.cpp"   // Update using your project name
#define TEST_MY_TRAIT(traitValue) TEST_METHOD_ATTRIBUTE(L"MyTrait", traitValue)

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace eVTOLSimultion
{
    TEST_CLASS(eVTOL_DCFastCharger_eVTOLRequest)
    {
    public:
        BEGIN_TEST_METHOD_ATTRIBUTE(eVTOLClassConstructor)
            TEST_OWNER(L"OwnerName")
            TEST_PRIORITY(1)
            TEST_MY_TRAIT(L"thisTraitValue")
            END_TEST_METHOD_ATTRIBUTE()

        TEST_METHOD(eVTOLClassConstructor)
        {
            Logger::WriteMessage("eVTOL Class\n");
            int status = eVTOLAlpha.readDeployStatus();
            Assert::AreEqual(status, 3);  //Alpha_Company's deploy status, READY_TO_DEPLOY =3

            int result = eVTOLAlpha.readMaxPassengers();
            Assert::AreEqual(result, 4);  //Alpha_Company's max passenger 4
        }

        TEST_METHOD(DCFastChargerConstructor)
        {
            Logger::WriteMessage("DCFastCharger Class\n");
            int cid = DCFastChargerCruz.readChargerID();
            Assert::AreEqual(cid, 0);  // Cruz DCCharger ID 0
        }

        TEST_METHOD(eVTOLRequestGeneration)
        {
            Logger::WriteMessage("eVTOLRequest Class\n");
            int result = generateRequestList();

            class DCFastCharger* dcCharger = (class DCFastCharger*)eVTOLReq[0].readDCChargerPtr();
            Assert::IsNull(dcCharger);  //After generated REQ list, dcChargerPtr is init to NULL
        }
    };
}


namespace MSTest
{
	TEST_CLASS(MSTest)
	{
	public:
		
		TEST_METHOD(TestMethod1)
		{
            std::cout << "Test MSTest\n";
		}
	};
}
