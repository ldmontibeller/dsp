//#############################################################################
//
// CPU 1 - BITFIELD, CPU 2 - DRIVERLIB
// CPU 1 - ePWM1 and ePWM2 - pins 0 to 3
// CPU 2 - ePWM4 and ePWM5 - pins 6 to 9
// ePWM1 - sawtooth 10 kHz, duty of 0.4
// ePWM2 - triangular 50 kHz, duty of 0.33
// ePWM4 - sawtooth 15 kHz, duty of 0.6
// ePWM5 - triangular 4 kHz, modulation signal from -0.97 to 0.97
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

    // Set freq of ePWM clock (must be <= 100 MHz)
    EALLOW;
        ClkCfgRegs.PERCLKDIVSEL.bit.EPWMCLKDIV = 1;
    EDIS;

    // Assign ePWM modules to CPUs
    EALLOW;
        DevCfgRegs.CPUSEL0.bit.EPWM1 = 0;
        DevCfgRegs.CPUSEL0.bit.EPWM2 = 0;
        DevCfgRegs.CPUSEL0.bit.EPWM4 = 1; //assigned to cpu2
        DevCfgRegs.CPUSEL0.bit.EPWM5 = 1; //assigned to cpu2
    EDIS;

    // Enable the clocks to ePWM modules of this CPU
    EALLOW;
        CpuSysRegs.PCLKCR2.bit.EPWM1 = 1;
     CpuSysRegs.PCLKCR2.bit.EPWM2 = 1;
    EDIS;

    // Stop the timer counters for ePWM of this CPU
    EALLOW;
        CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0; //stop the counter for this CPU
    EDIS;

    // Configure the ePWM modules
        // Time Base sub-module
            //ePWM 1
            EPwm1Regs.TBCTL.bit.FREE_SOFT = 2;
            // 100 MHz from EPWMCLKDIV. (TBCLK = EPWMCLK/(HSPCLKDIV * CLKDIV)
            // Default values are 2 for HSPCLKDIV and 1 for CLKDIV
            // TBCLK = 100M / (2 * 1) = 50M Hz
            // Even if its default, we dont know if it was changed before or not
            EPwm1Regs.TBCTL.bit.CLKDIV = 0;
            EPwm1Regs.TBCTL.bit.HSPCLKDIV = 1;
            EPwm1Regs.TBCTL.bit.PRDLD = 0; //Load in shadow mode/mirrored register
            EPwm1Regs.TBCTL.bit.CTRMODE = 0; //Up count mode for sawtooth carrier

             //ePWM 2
            EPwm2Regs.TBCTL.bit.FREE_SOFT = 2;
            // 100 MHz from EPWMCLKDIV. (TBCLK = EPWMCLK/(HSPCLKDIV * CLKDIV)
            // TBCLK = 100M / (1 * 1) = 100M Hz
            // Even if its default, we dont know if it was changed before or not
            EPwm2Regs.TBCTL.bit.CLKDIV = 0;
            EPwm2Regs.TBCTL.bit.HSPCLKDIV = 0;
            EPwm2Regs.TBCTL.bit.PRDLD = 0; //Load in shadow mode/mirrored register
            EPwm2Regs.TBCTL.bit.CTRMODE = 2; //Up-down count mode for triangular carrier

            //Period register (set frequency of PWM)
            //ePWM 1 (10 kHz sawtooth)
            //The Clock period is 1/50M = 0.02us
            //We want 10kHz, so we a period of 1/10k = 0.1m = 100us
            //With 0.02 us steps we need 5000 counts to arrive at 100us
            //Thus we need to load the value 5000 in TBPRD
            //Actually 5000-1 because we start at 0
            EPwm1Regs.TBPRD = 5000 - 1;

            //ePWM 2 (50 kHz triangular)
            //The Clock period is 1/100M = 0.01us
            //We want 50kHz triangular, we need a period of 1/50k = 0.02m = 20us
            //However, now one period is up and down count
            //So we need to count to half of that, 10us
            //With 0.01 us steps we need 1000 counts to arrive at 10us
            //Thus we need to load the value 1000 in TBPRD
            //In up-down mode you dont need to deduct one
            EPwm2Regs.TBPRD = 1000;

        //Counter Compare sub-module
            //ePWM 1
            EPwm1Regs.CMPCTL.bit.SHDWAMODE = 0; //enable shadow mode
            EPwm1Regs.CMPCTL.bit.LOADAMODE = 0; //load values when count is equal to 0 (beginning of each cycle)

            // Here we load a value from 0 to TBPRD corresponding to duty cycle
            // for 0.4 duty ratio its about 2000/5000 or  approx 1999/4999
            EPwm1Regs.CMPA.bit.CMPA = 1999;

            //ePWM 2
            EPwm2Regs.CMPCTL.bit.SHDWAMODE = 0; //enable shadow mode
            EPwm2Regs.CMPCTL.bit.LOADAMODE = 0; //load values when count is equal to 0 (beginning of each cycle)

            // Here we load a value from 0 to TBPRD corresponding to duty cycle
            // for 0.33 duty ratio its about 1000 * 0.33 = 333
            EPwm2Regs.CMPA.bit.CMPA = 333;
        //Action Qualifier sub-module
            // ePWM1 -SAWTOOTH
            // A output to be fed to the upper device of a leg
            // B output to be fed to the lower device of a leg
            // TBCTR = 0, A will be high, B will low (ZRO bits (AQCTLA/B))
            // TBCTR = CMPA. A will be low and B will be high (CAU bits -AQCTLA/B)
            EPwm1Regs.AQCTLA.bit.ZRO = 2;
            EPwm1Regs.AQCTLB.bit.ZRO = 1;
            EPwm1Regs.AQCTLA.bit.CAU = 1;
            EPwm1Regs.AQCTLB.bit.CAU = 2;

            // ePWM2 - TRIANGULAR
            EPwm2Regs.AQCTLA.bit.CAD = 2;
            EPwm2Regs.AQCTLA.bit.CAU = 1;
            // ePWM2B: Your Oscilloscope Sync Signal (tracks the counter bounds)
            // Go LOW when counter hits ZERO (bottom of triangle)
            EPwm2Regs.AQCTLB.bit.ZRO = 1; // 2 = Set
            // Go HIGH when counter hits PRD (top of triangle)
            EPwm2Regs.AQCTLB.bit.PRD = 2;
            
    // Config GPIOs to ePWM
    EALLOW;
        GpioCtrlRegs.GPAGMUX1.bit.GPIO0 = 0; 
        GpioCtrlRegs.GPAGMUX1.bit.GPIO1 = 0; 
        GpioCtrlRegs.GPAGMUX1.bit.GPIO2 = 0; 
        GpioCtrlRegs.GPAGMUX1.bit.GPIO3 = 0;

        GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1; 
        GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 1; 
        GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 1; 
        GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 1; 
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
