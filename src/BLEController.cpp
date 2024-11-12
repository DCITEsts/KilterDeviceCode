#include "BLEController.h"
#include "CharacteristicCallbacks.h"
#include "DescriptorCallbacks.h"
#include "ServerCallbacks.h"

void BLEController::BLEsetup() {
    Serial.println(F("Starting NimBLE Server"));

    /** sets device name */
    NimBLEDevice::init(DeviceName);
    BLEDevice::setMTU(256);

    NimBLEDevice::setPower(ESP_PWR_LVL_P9); /** +9db */


    /** Set the IO capabilities of the device, each option will trigger a different pairing method.
     *  BLE_HS_IO_DISPLAY_ONLY    - Passkey pairing
     *  BLE_HS_IO_DISPLAY_YESNO   - Numeric comparison pairing
     *  BLE_HS_IO_NO_INPUT_OUTPUT - DEFAULT setting - just works pairing
     */
    //NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY); // use passkey
    //NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_YESNO); //use numeric comparison

    /** 2 different ways to set security - both calls achieve the same result.
     *  no bonding, no man in the middle protection, secure connections.
     *
     *  These are the default values, only shown here for demonstration.
     */
    NimBLEDevice::setSecurityAuth(false, false, false);
    //NimBLEDevice::setSecurityAuth(/*BLE_SM_PAIR_AUTHREQ_BOND | BLE_SM_PAIR_AUTHREQ_MITM |*/ BLE_SM_PAIR_AUTHREQ_SC);

    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());
    Serial.println(F("Nimble server started"));
    StartServices();
    StartAdvertising();

}

void BLEController::SetupCommsService()
{
    pCommsService = pServer->createService(CommsServiceUUID);
    pCommsOutgoingCharacteristic = pCommsService->createCharacteristic(
                                               CommsOutgoingCharacteristicUUID,
                                               NIMBLE_PROPERTY::READ |
                                               NIMBLE_PROPERTY::WRITE, 256
                                              );

    pCommsOutgoingCharacteristic->setValue(0);
    pCommsOutgoingCharacteristic->setCallbacks(&chrCallbacks);

    pCommsIncomingCharacteristic = pCommsService->createCharacteristic(
                                               CommsIncomingCharacteristicUUID,
                                               NIMBLE_PROPERTY::READ |
                                               NIMBLE_PROPERTY::WRITE, 256
                                              );

    pCommsIncomingCharacteristic->setValue(0);
    pCommsIncomingCharacteristic->setCallbacks(&chrCallbacks);

    pCommsService->start();
    
    
}

void BLEController::SetupPositionService()
{
    pPositionService = pServer->createService(PositionServiceUUID); //setup service

    //setup all the characteristics

    pMidbackPositionCharacteristic = pPositionService->createCharacteristic(
                                               MidbackPositionCharacteristicUUID,
                                               NIMBLE_PROPERTY::READ |
                                               NIMBLE_PROPERTY::WRITE, 256
                                              );
    pMidbackPositionCharacteristic->setValue(0);
    pMidbackPositionCharacteristic->setCallbacks(&chrCallbacks);

    pMidbackRequestPositionCharacteristic = pPositionService->createCharacteristic(
                                               MidbackRequestPositionCharacteristicUUID,
                                               NIMBLE_PROPERTY::READ |
                                               NIMBLE_PROPERTY::WRITE, 256
                                              );
    pMidbackRequestPositionCharacteristic->setValue(0);
    pMidbackRequestPositionCharacteristic->setCallbacks(&chrCallbacks);

    pSeatAngleCharacteristic = pPositionService->createCharacteristic(
                                               SeatAngleCharacteristicUUID,
                                               NIMBLE_PROPERTY::READ |
                                               NIMBLE_PROPERTY::WRITE, 256
                                              );
    pSeatAngleCharacteristic->setValue(0);
    pSeatAngleCharacteristic->setCallbacks(&chrCallbacks);

    pSeatRequestAngleCharacteristic = pPositionService->createCharacteristic(
                                               SeatRequestAngleCharacteristicUUID,
                                               NIMBLE_PROPERTY::READ |
                                               NIMBLE_PROPERTY::WRITE, 256
                                              );
    pSeatRequestAngleCharacteristic->setValue(0);
    pSeatRequestAngleCharacteristic->setCallbacks(&chrCallbacks);

    pSeatExtensionPositionCharacteristic = pPositionService->createCharacteristic(
                                               SeatExtensionPositionCharacteristicUUID,
                                               NIMBLE_PROPERTY::READ |
                                               NIMBLE_PROPERTY::WRITE, 256
                                              );
    pSeatExtensionPositionCharacteristic->setValue(0);
    pSeatExtensionPositionCharacteristic->setCallbacks(&chrCallbacks);

     pSeatRequestExtensionPositionCharacteristic = pPositionService->createCharacteristic(
                                               SeatRequestExtensionCharacteristicUUID,
                                               NIMBLE_PROPERTY::READ |
                                               NIMBLE_PROPERTY::WRITE, 256
                                              );
    pSeatRequestExtensionPositionCharacteristic->setValue(0);
    pSeatRequestExtensionPositionCharacteristic->setCallbacks(&chrCallbacks);

    pPositionService->start();
}

void BLEController::StartServices()
{
    SetupCommsService();
    SetupPositionService();
}

void BLEController::StartAdvertising()
{
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    /** Add the services to the advertisment data **/
    pAdvertising->addServiceUUID(pCommsService->getUUID());
    
    pAdvertising->setScanResponse(true);
    pAdvertising->start();

    Serial.println("Advertising Started");
}

void BLEController::NotifyCommsService()
{
        if(pServer->getConnectedCount()) {
        NimBLEService* pSvc = pServer->getServiceByUUID(CommsServiceUUID);
        if(pSvc) {
            NimBLECharacteristic* pChr = pSvc->getCharacteristic(CommsOutgoingCharacteristicUUID);
            if(pChr) {
                pChr->notify(true);
            }
            NimBLECharacteristic* pChr2 = pSvc->getCharacteristic(CommsIncomingCharacteristicUUID);
            if(pChr2) {
                pChr2->notify(true);
            }
        }
        else
        {
            pServer->startAdvertising();
        }
    }
}

void BLEController::WriteOutgoingCommsCharacteristic(std::string valToWrite)
{   
    pCommsOutgoingCharacteristic->setValue(valToWrite);

}
int BLEController::ReadIncomingCommsCharacteristic()
{
    Serial.println("Reading Requests");
    if(pCommsIncomingCharacteristic == NULL)
    {
        Serial.println("Incoming requests didn't exist to be read");
    }
    else
    {
        const char *value = pCommsIncomingCharacteristic->getValue().c_str();
        Serial.println(value);
        int intval = atoi(value);
        Serial.println("intval");
        return intval;
    }
    
    return 0;

}

void BLEController::WriteMidbackPositionCharacteristic(std::string valToWrite)
{
    pMidbackPositionCharacteristic->setValue(valToWrite);
}
void BLEController::WriteSeatAngleCharacteristic(std::string valToWrite)
{
    pSeatAngleCharacteristic->setValue(valToWrite);
}
void BLEController::WriteSeatExtensionPositionCharacteristic(std::string valToWrite)
{
    pSeatExtensionPositionCharacteristic->setValue(valToWrite);
}

int BLEController::ReadMidbackRequestPositionCharacteristic()
{
    const char *value = pMidbackRequestPositionCharacteristic->getValue().c_str();
    return atoi(value);
}
int BLEController::ReadWriteSeatRequestAngleCharacteristic()
{
    const char *value = pSeatRequestAngleCharacteristic->getValue().c_str();
    return atoi(value);
}
int BLEController::ReadSeatRequestExtensionPositionCharacteristic()
{
    const char *value = pSeatRequestExtensionPositionCharacteristic->getValue().c_str();
    return atoi(value);
}

int BLEController::CheckConnected()
{
    if(pServer->getConnectedCount() > 0)
    {
        if (pServer->getConnectedCount() > 3)
        {
            pServer->stopAdvertising();
        }
        else
        {
            StartAdvertising();
        }
        
        return 1;
    }
    else
    {
        StartAdvertising();
        return 0;
    }
}