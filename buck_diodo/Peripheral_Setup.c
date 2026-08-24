/*
 * Peripheral_Setup.c
 *
 *  Created on: 23 de jul de 2020
 *      Author: waner
 */
#include "Peripheral_Setup.h"
#include "F28x_Project.h"
#include "F2837xD_device.h"
#include "F2837xD_Examples.h"

void Setup_GPIO(void)
{
    EALLOW;

    //This combination of mux register makes GPIO6 as EPWM4A
    GpioCtrlRegs.GPAGMUX1.bit.GPIO6 = 0; 
    GpioCtrlRegs.GPAMUX1.bit.GPIO6 = 1; 

    //This combination of mux register makes GPIO6 as EPWM4B
    GpioCtrlRegs.GPAGMUX1.bit.GPIO7 = 0; 
    GpioCtrlRegs.GPAMUX1.bit.GPIO7 = 1; 

    // LED no GPIO34
    GpioCtrlRegs.GPBMUX1.bit.GPIO34 = 0;
    GpioCtrlRegs.GPBDIR.bit.GPIO34 = 1;

    // GPIO2 pra testar adc
    GpioCtrlRegs.GPAGMUX1.bit.GPIO2 = 0;
    GpioCtrlRegs.GPADIR.bit.GPIO2  = 1;     //define como saida


    EDIS;
}

void Setup_PWM1(void)
{
    EALLOW;
    // Set freq of ePWM clock (must be <= 100 MHz)
    ClkCfgRegs.PERCLKDIVSEL.bit.EPWMCLKDIV = 1;
    
    // Assign ePWM modules to CPUs
    DevCfgRegs.CPUSEL0.bit.EPWM4 = 0; //assigned to cpu1

    // Enable the clocks to ePWM modules of this CPU
    CpuSysRegs.PCLKCR2.bit.EPWM4 = 1;

    // Stop the timer counters for ePWM of this CPU
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0; 


    //ePWM 4
    //FREE_SOFT makes PWM run independet from debugger pauses
    EPwm4Regs.TBCTL.bit.FREE_SOFT = 2;

    // 100 MHz from EPWMCLKDIV. (TBCLK = EPWMCLK/(HSPCLKDIV * CLKDIV)
    // TBCLK = 100M / (1 * 1) = 100M Hz
    // Even if its default, we dont know if it was changed before or not
    EPwm4Regs.TBCTL.bit.CLKDIV = 0;
    EPwm4Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;        // Clock ratio to SYSCLKOUT
    EPwm4Regs.TBCTL.bit.PRDLD = 0; //Load in shadow mode/mirrored register
    EPwm4Regs.TBCTL.bit.CTRMODE = 2; //Up-down count mode for triangular carrier


    //The Clock period is 1/100M = 0.01us
    //We want 50kHz triangular, we need a period of 1/50k = 0.02m = 20us
    //However, now one period is up and down count
    //So we need to count to half of that, 10us
    //With 0.01 us steps we need 1000 counts to arrive at 10us
    //Thus we need to load the value 1000 in TBPRD
    //In up-down mode you dont need to deduct one
    EPwm4Regs.TBPRD = 1000;

    // Here we load a value from 0 to TBPRD corresponding to duty cycle
    EPwm4Regs.CMPA.bit.CMPA = 1000 * 0.2737;        //Starting duty cycle
    EPwm4Regs.TBCTR = 0x0000;                       // Clear counter
    EPwm4Regs.TBPHS.bit.TBPHS = 0;                  // Phase is 0

    EPwm4Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_DISABLE;     // DEFASADO DA REFERENCIA EPWM1
    EPwm4Regs.TBCTL.bit.PHSEN = TB_DISABLE;         // Disable phase loading
    EPwm4Regs.TBCTL.bit.PHSDIR = TB_DOWN;           // Disable phase loading
    
    EPwm4Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;     // Load registers every ZERO
    EPwm4Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO_PRD; //Update twice per switching period (once at the peak, once at the valley)
    EPwm4Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;     // Load registers every ZERO
    EPwm4Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO_PRD;

     // ePWM4 - TRIANGULAR
    EPwm4Regs.AQCTLA.bit.CAD = AQ_SET;              // Turn ON on the way down
    EPwm4Regs.AQCTLA.bit.CAU = AQ_CLEAR;            // Turn OFF on the way up
    EPwm4Regs.AQCTLA.bit.ZRO = AQ_NO_ACTION;            
    EPwm4Regs.AQCTLA.bit.PRD = AQ_NO_ACTION;        
    EPwm4Regs.AQCTLA.bit.CBU = AQ_NO_ACTION;        

    EPwm4Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;       // Active Hi complementary
    EPwm4Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;  // enable Dead-band module
    EPwm4Regs.DBFED.bit.DBFED = 10;                 // FED = 20 TBCLKs
    EPwm4Regs.DBRED.bit.DBRED = 10;                 // RED = 20 TBCLKs

    EPwm4Regs.ETSEL.bit.SOCAEN = 1;                 // Enable SOC on A group
    EPwm4Regs.ETSEL.bit.SOCASEL = 3;                // 3 = ET_CTR_PRDZERO (Triggers on both Zero and PRD)
    EPwm4Regs.ETPS.bit.SOCAPRD = 1;                 // Trigger on every event (ET_1ST)

    
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;
}

void Setup_ADC(void)
{
    uint16_t i = 0;
    EALLOW;
    CpuSysRegs.PCLKCR13.bit.ADC_A = 1;   // habilita clock ADCA
    AdcaRegs.ADCCTL2.bit.PRESCALE = 6;
    AdcSetMode(0, 0, 0);
    AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 1;               // set pulse um ciclo antes do resultado
    AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1;                  // liga o adc
    for(i = 0; i < 1000; i++){}
    //DELAY_US(1000);                                     // tempo pra dar de ligar

    AdcaRegs.ADCSOC0CTL.bit.CHSEL = 4;                  // canal A4
    AdcaRegs.ADCSOC0CTL.bit.ACQPS = 14;                 // Tempo de aquisicao 14 pulso de clk eu acho
    AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = 11;             


    AdcaRegs.ADCSOC1CTL.bit.CHSEL = 1;                  // // canal A1
    AdcaRegs.ADCSOC1CTL.bit.ACQPS = 14;                 // Tempo de aquisicao 14 pulso de clk eu acho
    AdcaRegs.ADCSOC1CTL.bit.TRIGSEL = 11;           

    AdcaRegs.ADCSOC2CTL.bit.CHSEL = 2;                  // canal A2
    AdcaRegs.ADCSOC2CTL.bit.ACQPS = 14;                 // Tempo de aquisicao 14 pulso de clk eu acho
    AdcaRegs.ADCSOC2CTL.bit.TRIGSEL = 11;             

    AdcaRegs.ADCINTSEL1N2.bit.INT1SEL = 2; // fim do SOC2 gera ADCINT1
    AdcaRegs.ADCINTSEL1N2.bit.INT1E = 1;                // habilita ADCINT1
    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;              // limpa flag ADCINT1
    EDIS;

     

}
