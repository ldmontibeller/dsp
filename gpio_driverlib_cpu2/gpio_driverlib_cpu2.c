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
     GPIO_writePin(34, 1);




    // Infinite loop
    for (;;) 
    {
        DEVICE_DELAY_US(250000);
        GPIO_togglePin(34);
    }
}

//
// End of File
//
