#pragma once

#ifndef BLECONTROLLER_H 
#define BLECONTROLLER_H

#include <NimBLEDevice.h>
#include "GlobalPins.h"

#define DeviceName "Kilter_Tilter" //Device name can be changed if desired

#define CommsServiceUUID "CS-188e4-9067-44d6-83a7-0177ee59f924" //Service to send commands and receive device state

#define CommsOutgoingCharacteristicUUID "COC-0abc-6896-4f24-938c-a477b9f8365e" //read from this characteristic for device states. DONT WRITE TO
#define CommsIncomingCharacteristicUUID "CIC-9bf1-0cf7-4d22-a652-8b765755f0ae" //Write Commands to this characteristic

#define PositionServiceUUID "PS-93f46-e7e2-4430-ad19-228306871dd1" //service to get positions from encoders and set goal positions for actuators

#define MidbackPositionCharacteristicUUID "MPC-e115-19ca-4420-a6a8-ac877231fc7a" //Read to get the midback position. DONT WRITE TO
#define MidbackRequestPositionCharacteristicUUID "MRC-693c-575a-4e07-befc-2d5c6a8b7bdf" //Write to set a new midback goal position
#define SeatAngleCharacteristicUUID "SAC-1898-7f0b-4f34-86c2-bf3bfe610a00"//Read to get the Seat Angle. DONT WRITE TO
#define SeatRequestAngleCharacteristicUUID "SRC-1448-a77c-4ed1-85be-491c92732f37"//Write to set a new Seat angle goal position
#define SeatExtensionPositionCharacteristicUUID "SEP-0757-eff5-4bd3-a4ee-8886edf69140"//Read to get the seat extension position. DONT WRITE TO
#define SeatRequestExtensionCharacteristicUUID "SRP-a825-9ba5-45a6-b312-def5e2ddbe72"//Write to set a new seat extension goal position

#define MaxConnectionsBLE 3 //set the max BLE Connections before advertising is stopped

class BLEController
{
public:

    //Service and characteristic pointers

    NimBLEService* pCommsService;
    NimBLECharacteristic* pCommsOutgoingCharacteristic;
    NimBLECharacteristic* pCommsIncomingCharacteristic;

    NimBLEService* pPositionService;
    NimBLECharacteristic* pMidbackPositionCharacteristic;
    NimBLECharacteristic* pMidbackRequestPositionCharacteristic;
    NimBLECharacteristic* pSeatAngleCharacteristic;
    NimBLECharacteristic* pSeatRequestAngleCharacteristic;
    NimBLECharacteristic* pSeatExtensionPositionCharacteristic;
    NimBLECharacteristic* pSeatRequestExtensionPositionCharacteristic;

    void BLEsetup();// run once to setup BLE and initialize

    //Functions to write a value to the respective characteristic
    //std::string will be converted to a c style string and written
    void WriteOutgoingCommsCharacteristic(std::string valToWrite);
    int ReadIncomingCommsCharacteristic();

    void WriteMidbackPositionCharacteristic(std::string valToWrite);
    void WriteSeatAngleCharacteristic(std::string valToWrite);
    void WriteSeatExtensionPositionCharacteristic(std::string valToWrite);

    //Functions to read characteristics
    //All characteristics should only have ints written to them
    //A non integer value is just going to be ignored and return 0
    int ReadMidbackRequestPositionCharacteristic();
    int ReadWriteSeatRequestAngleCharacteristic();
    int ReadSeatRequestExtensionPositionCharacteristic();

    int CheckConnected();//checks how many devices are connected

private:

    //Functions to setup individual services
    void SetupCommsService();
    void SetupPositionService();

    //functions run to initialize BLE
    void StartServices();
    void StartAdvertising();
    void NotifyCommsService();

};

#endif