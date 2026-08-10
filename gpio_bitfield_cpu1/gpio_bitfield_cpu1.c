//#############################################################################
//
// GPIO example
// This code control 2 GPIO pins (GPIO31 (blue builtin led), GPIO34 (red builtin led) using both CPUs.
// CPU1 controls GPIO31
// CPU2 controls GPIO34
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
   
    // GPIO31 (blue)
    GpioCtrlRegs.GPAMUX2.bit.GPIO31 = 0; //Set the pin to behave as GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO31 = 1; //Set the pin as output

    // GPIO34
    // Even though will be controlled by CPU2, only CPU1 can change these registers
    GpioCtrlRegs.GPBMUX1.bit.GPIO34 = 0; //Set the pin to behave as GPIO
    GpioCtrlRegs.GPBDIR.bit.GPIO34 = 1; //Set the pin as output
    GpioCtrlRegs.GPBCSEL1.bit.GPIO34 = 2; //Set the pin to be controlled by CPU2
    
    EDIS;

    // Change the default state of CPU1 GPIO pins
    // GPIO9
    GpioDataRegs.GPASET.bit.GPIO31= 1; //Set pin state to ON

    //Infinite loop
    for (;;) 
    {
        DELAY_US(500000);
        GpioDataRegs.GPATOGGLE.bit.GPIO31 = 1;
    }

}

//
// End of File 
//
