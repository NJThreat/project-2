//=====[Libraries]=============================================================

#include "mbed.h"
#include "arm_book_lib.h"

//=====[Defines]===============================================================

#define HEADLIGHT_OFF_LEVEL     0.33
#define HEADLIGHT_AUTO_LEVEL    0.66
#define HEADLIGHT_ON_LEVEL      1.00
#define LDR_DAWN_DUSK_DIVIDE    0.5

//=====[Declaration and initialization of public global objects]===============

DigitalIn driverSeatSensor (D0);
DigitalIn ignitionButton (D1);
DigitalIn highBeamSetting(D2);

DigitalOut ignitionLed (LED1);
DigitalOut engineLed (LED2);
DigitalOut leftLowBeam (D4);
DigitalOut leftHighBeam (D5);
DigitalOut rightLowBeam (D6);
DigitalOut rightHighBeam (D7);

UnbufferedSerial uartUsb(USBTX, USBRX, 115200);

AnalogIn lightSensor (A0);
AnalogIn potentiometer(A1);

//=====[Declaration and initialization of public global variables]=============

bool ignitionState = OFF;
bool engineState = OFF;
bool headlightOffState = OFF;
bool headlightOnState = OFF;
bool headlightAutoState = OFF;
bool highBeamState = OFF;
bool driverWelcomeMessageSent = false;

float potentiometerReading = 0.0;
float photoRReadingA = 0;
float photoRReadingB = 0;

//=====[Declarations (prototypes) of public functions]=========================

void inputsInit();
void outputsInit();
void driverWelcomeUpdate();
void ignitionActivationUpdate();
void engineActivationUpdate();
void headlightStateUpdate();
void headlightActivationUpdate();

//=====[Main function, the program entry point after power on or reset]========

int main(){
    inputsInit();
    outputsInit();
    while (true)
    {
        driverWelcomeUpdate();
        ignitionActivationUpdate();
        engineActivationUpdate();
        headlightStateUpdate();
        headlightActivationUpdate();
    }
}

//=====[Implementations of public functions]===================================

//Activates Inputs Declared Above
void inputsInit()
{
    driverSeatSensor.mode(PullDown);
    ignitionButton.mode(PullDown);
    highBeamSetting.mode(PullDown);
}

//Activates Outputs Declared Above
void outputsInit() 
{
    ignitionLed = OFF;
    engineLed = OFF;
}

/*Controls the driver welcome message. Boolean "driverWelcomeMessageSent" prevents
infinite output of message. 
*/
void driverWelcomeUpdate(){
    if (driverSeatSensor && !engineState){
        if (!driverWelcomeMessageSent) {
            uartUsb.write ("Welcome to BMW, the ultimate driving machine.\r\n", 48);
            driverWelcomeMessageSent = true;
        }
    }
    else if (!driverSeatSensor){
        driverWelcomeMessageSent = false;
    }
}

//Modifies ignition state if specified conditions are met. Controls ignitionLed.
void ignitionActivationUpdate() 
{
    if (driverSeatSensor && !engineState){
        ignitionState = ON;
    }
    else if (!driverSeatSensor || engineState){
        ignitionState = OFF;
    }
    ignitionLed = ignitionState;
}

//Modifies engine state if specifed conditions are met. Controls engineLed and siredPin.
void engineActivationUpdate(){
    if (ignitionState && ignitionButton && !engineState){
        ignitionState = OFF;
        engineState = ON;
        uartUsb.write("Engine started.\r\n", 17);
    }
    else if (!ignitionState && ignitionButton & !engineState){
        uartUsb.write("Ignition inhibited\r\n", 20);
        if (!driverSeatSensor){
            uartUsb.write("Driver seat not occupied.\r\n", 27);
        }
    }
    else if (engineState && ignitionButton){
        engineState = OFF;
    }
    ignitionLed = ignitionState;
    engineLed = engineState;
}
//Modifies headlight state if specifed conditions are met. changes the state of the Headlight to be read by headlightActivationUpdate.
void headlightStateUpdate(){
    if (engineState){
        potentiometerReading = potentiometer.read();
        if (potentiometerReading <= HEADLIGHT_OFF_LEVEL){
            headlightOffState = ON;
            headlightAutoState = OFF;
            headlightOnState = OFF;
        }
        if (HEADLIGHT_OFF_LEVEL < potentiometerReading && potentiometerReading <= HEADLIGHT_AUTO_LEVEL){
            headlightAutoState = ON;
            photoRReadingA = lightSensor.read();
            delay(1000);
            photoRReadingB = lightSensor.read();
            if (abs(photoRReadingA - photoRReadingB) < 0.05)
            {
                if (photoRReadingB > LDR_DAWN_DUSK_DIVIDE){
                    delay(1000);
                    photoRReadingA = lightSensor.read();
                    if (abs(photoRReadingA - photoRReadingB) < 0.05)
                    {
                        headlightOffState = ON;
                        headlightOnState = OFF;
                    }
                }
                else{
                    headlightOffState = OFF;
                    headlightOnState = ON;
                }
            }
        }
        if (HEADLIGHT_AUTO_LEVEL < potentiometerReading && potentiometerReading <= HEADLIGHT_ON_LEVEL){
            headlightOffState = OFF;
            headlightAutoState = OFF;
            headlightOnState = ON;
        }
        }
    else {
        headlightOffState = ON;
        headlightAutoState = OFF;
        headlightOnState = OFF;
    }
}
// activates the headlights based on the headLight stae. Controls the high beam and low beam LEDs.
void headlightActivationUpdate(){
    if (highBeamSetting){
        if (headlightOnState){
            leftLowBeam = ON;
            rightLowBeam = ON;
            leftHighBeam = ON;
            rightHighBeam = ON;
        }
        if (headlightOffState){
            leftLowBeam = OFF;
            rightLowBeam = OFF;
            leftHighBeam = OFF;
            rightHighBeam = OFF;
        }
    }
    else if (!highBeamSetting){
        if (headlightOnState){
            leftLowBeam = ON;
            rightLowBeam = ON;
            leftHighBeam = OFF;
            rightHighBeam = OFF;
        }
        if (headlightOffState){
            leftLowBeam = OFF;
            rightLowBeam = OFF;
            leftHighBeam = OFF;
            rightHighBeam = OFF;
        }
    }
}
