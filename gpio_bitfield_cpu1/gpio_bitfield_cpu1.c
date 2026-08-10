//#############################################################################
//
// GPIO example
// This code control 3 GPIO pins (GPIO22, GPIO52, GPIO97) using both CPUs.
// CPU1 controls GPIO22 and GPIO97
// CPU2 controls GPIO52
//
//#############################################################################

//
// Included Files
//
#include "F28x_Project.h"
// #include "driverlib.h"
// #include "device.h"

//
// Main
//
void main(void)
{
    // Initialize system - clock, PLL etc
    InitSysCtrl();

    // Configure GPIO pins
    // Some registers are protected by write protection. 
    //  EALLOW, together with EDIS, allow changing them.
    EALLOW; 
   
    // GPIO22
    GpioCtrlRegs.GPAMUX2.bit.GPIO22 = 0; //Set the pin to behave as GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO22 = 1; //Set the pin as output

    // GPIO52
    // Even though will be controlled by CPU, only CPU1 can change these registers
    GpioCtrlRegs.GPBMUX2.bit.GPIO52 = 0; //Set the pin to behave as GPIO
    GpioCtrlRegs.GPBDIR.bit.GPIO52 = 1; //Set the pin as output
    GpioCtrlRegs.GPBCSEL3.bit.GPIO52 = 2; //Set the pin to be controlled by CPU2
    
    // GPIO97
    GpioCtrlRegs.GPDMUX1.bit.GPIO97 = 0; //Set the pin to behave as GPIO
    GpioCtrlRegs.GPDDIR.bit.GPIO97 = 1; //Set the pin as output

    EDIS;

    // Change the default state of CPU1 GPIO pins
    // GPIO22
    GpioDataRegs.GPASET.bit.GPIO22 = 1; //Set pin state to ON

    // GPIO97
    GpioDataRegs.GPDSET.bit.GPIO97 = 1;

    //Infinite loop
    for (;;) 
    {
        DELAY_US(500000);
        GpioDataRegs.GPDTOGGLE.bit.GPIO97 = 1;
        GpioDataRegs.GPATOGGLE.bit.GPIO22 = 1;
    }

}

//
// End of File 
//
