#pragma once

#ifndef BLECONTROLLER_H 
#define BLECONTROLLER_H

#include <NimBLEDevice.h>
#include "GlobalPins.h"

#define CommsServiceUUID "4ac188e4-9067-44d6-83a7-0177ee59f924"

#define CommsOutgoingCharacteristicUUID "4b600abc-6896-4f24-938c-a477b9f8365e"
#define CommsIncomingCharacteristicUUID "38d79bf1-0cf7-4d22-a652-8b765755f0ae"

class BLEController
{
public:

    //ServicePointers

    NimBLEService* pCommsService;
    NimBLECharacteristic* pCommsOutgoingCharacteristic;
    NimBLECharacteristic* pCommsIncomingCharacteristic;

    void SetupCommsService();

    void StartServices();
    void StartAdvertising();
    void NotifyCommsService();

    void BLEsetup();

    void WriteOutgoingCommsCharacteristic(std::string valToWrite);
    int ReadIncomingCommsCharacteristic();

    int CheckConnected();
};

#endif