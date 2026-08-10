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
#include "driverlib.h"
#include "device.h"

//
// Main
//
void main(void)
{
    // CPU2: doesnt need to initialize again
    // Change the default level of GPIO52

    // using driverlib functions (similar to Arduino)
    // GPIO_writePin(52, 1);

    // using base register + register offset
    // this is actually what driverlin functions are doing under the hood
    HWREG(GPIODATA_BASE + GPIO_O_GPBSET) = (uint32_t) 0x100000U;


    // Infinite loop
    for (;;) 
    {
        DEVICE_DELAY_US(250000);
        HWREG(GPIODATA_BASE + GPIO_O_GPBTOGGLE) = (uint32_t) 0x100000U;
    }
}

//
// End of File
//
