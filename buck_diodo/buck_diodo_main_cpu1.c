//#############################################################################
//
// ePWM4 - triangular 50 kHz, duty of 0.33
//
//#############################################################################

//
// Included Files
//
#include "F28x_Project.h"
#include "driverlib.h"
#include "device.h"

//
// Main
//
void main(void)
{
    // Init system - clocks, PLL
    InitSysCtrl();

    EALLOW; //Config registers protection unlock
    // Set freq of ePWM clock (must be <= 100 MHz)
        ClkCfgRegs.PERCLKDIVSEL.bit.EPWMCLKDIV = 1;
    
    // Assign ePWM modules to CPUs
        DevCfgRegs.CPUSEL0.bit.EPWM4 = 0; //assigned to cpu1

    // Enable the clocks to ePWM modules of this CPU
        CpuSysRegs.PCLKCR2.bit.EPWM4 = 1;
    
    // Stop the timer counters for ePWM of this CPU
        CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0; 
    EDIS; //Config registers protection lock

    // Configure the ePWM modules
        // Time Base sub-module
            //ePWM 4
            //FREE_SOFT makes PWM run independet from debugger pauses
            EPwm4Regs.TBCTL.bit.FREE_SOFT = 2;

            // 100 MHz from EPWMCLKDIV. (TBCLK = EPWMCLK/(HSPCLKDIV * CLKDIV)
            // TBCLK = 100M / (1 * 1) = 100M Hz
            // Even if its default, we dont know if it was changed before or not
            EPwm4Regs.TBCTL.bit.CLKDIV = 0;
            EPwm4Regs.TBCTL.bit.HSPCLKDIV = 0;
            EPwm4Regs.TBCTL.bit.PRDLD = 0; //Load in shadow mode/mirrored register
            EPwm4Regs.TBCTL.bit.CTRMODE = 2; //Up-down count mode for triangular carrier

            //ePWM 2 (50 kHz triangular)
            //The Clock period is 1/100M = 0.01us
            //We want 50kHz triangular, we need a period of 1/50k = 0.02m = 20us
            //However, now one period is up and down count
            //So we need to count to half of that, 10us
            //With 0.01 us steps we need 1000 counts to arrive at 10us
            //Thus we need to load the value 1000 in TBPRD
            //In up-down mode you dont need to deduct one
            EPwm4Regs.TBPRD = 1000;

        //Counter Compare sub-module
            //ePWM 4
            EPwm4Regs.CMPCTL.bit.SHDWAMODE = 0; //enable shadow mode
            EPwm4Regs.CMPCTL.bit.LOADAMODE = 0; //load values when count is equal to 0 (beginning of each cycle)

            // Here we load a value from 0 to TBPRD corresponding to duty cycle
            // for 0.33 duty ratio its about 1000 * 0.33 = 333
            EPwm4Regs.CMPA.bit.CMPA = 333;
        //Action Qualifier sub-module
            // ePWM4 - TRIANGULAR
            EPwm4Regs.AQCTLA.bit.CAD = 2;
            EPwm4Regs.AQCTLA.bit.CAU = 1;
            // ePWM4: Your Oscilloscope Sync Signal (tracks the counter bounds)
            // Go LOW when counter hits ZERO (bottom of triangle)
            EPwm4Regs.AQCTLB.bit.ZRO = 1; // 2 = Set
            // Go HIGH when counter hits PRD (top of triangle)
            EPwm4Regs.AQCTLB.bit.PRD = 2;
            
    // Config GPIOs to ePWM
    EALLOW;
        //This combination of mux register makes GPIO6 as EPWM4A
        GpioCtrlRegs.GPAGMUX1.bit.GPIO6 = 0; 
        GpioCtrlRegs.GPAMUX1.bit.GPIO6 = 1; 

        //This combination of mux register makes GPIO6 as EPWM4B
        GpioCtrlRegs.GPAGMUX1.bit.GPIO7 = 0; 
        GpioCtrlRegs.GPAMUX1.bit.GPIO7 = 1; 
    EDIS;

    // Start the timer counters for ePWM
    EALLOW;
        CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1; //start the counter for this CPU
    EDIS;

    // Infinite loop
    for(;;)
    {

    }

}

//
// End of File
//
