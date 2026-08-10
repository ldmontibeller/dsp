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
    // EPWM4A no GPIO0
    GpioCtrlRegs.GPAMUX1.bit.GPIO6 = 1;     //GPIO0 como EPWM1A
    GpioCtrlRegs.GPADIR.bit.GPIO6  = 1;     //define como saida

    // EPWM4B no GPIO0
    GpioCtrlRegs.GPAMUX1.bit.GPIO7 = 1;     //GPIO0 como EPWM1A
    GpioCtrlRegs.GPADIR.bit.GPIO7  = 1;     //define como saida

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
    CpuSysRegs.PCLKCR2.bit.EPWM4 = 1;
    CpuSysRegs.PCLKCR2.bit.EPWM8 = 1;// habilita clock EPWM1

    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;

    EPwm4Regs.TBPRD = 1999;                         // Set timer period
    EPwm4Regs.CMPA.bit.CMPA = 1999*0.2737;           // Segundo ponto de comparação
    EPwm4Regs.TBCTR = 0x0000;                       // Clear counter
    EPwm4Regs.TBPHS.bit.TBPHS = 0;                  // Phase is 0

    EPwm4Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_DISABLE;     // DEFASADO DA REFERENCIA EPWM1
    EPwm4Regs.TBCTL.bit.PHSEN = TB_DISABLE;         // Disable phase loading
    EPwm4Regs.TBCTL.bit.PHSDIR = TB_DOWN;           // Disable phase loading
    EPwm4Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;      // Count up
    EPwm4Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;        // Clock ratio to SYSCLKOUT
    EPwm4Regs.TBCTL.bit.CLKDIV = TB_DIV1;

    EPwm4Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;     // Load registers every ZERO
    EPwm4Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO_PRD;
    EPwm4Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;     // Load registers every ZERO
    EPwm4Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO_PRD;

    EPwm4Regs.AQCTLA.bit.ZRO = AQ_SET;              // Sobe exatamente em CTR=0
    EPwm4Regs.AQCTLA.bit.CAU = AQ_CLEAR;            // Desce em CMPA na subida
    EPwm4Regs.AQCTLA.bit.PRD = AQ_NO_ACTION;        // Opcional
    EPwm4Regs.AQCTLA.bit.CBU = AQ_NO_ACTION;        // nao usar CBU no A

    EPwm4Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;       // Active Hi complementary
    EPwm4Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;  // enable Dead-band module
    EPwm4Regs.DBFED.bit.DBFED = 10;                 // FED = 20 TBCLKs
    EPwm4Regs.DBRED.bit.DBRED = 10;                 // RED = 20 TBCLKs

    EPwm8Regs.TBPRD = 1999;                         // Set timer period
    EPwm8Regs.CMPA.bit.CMPA = 1999*0.5;

    EPwm8Regs.TBPHS.bit.TBPHS = 0;                  // Phase is 0
    EPwm8Regs.TBCTL.bit.SYNCOSEL = TB_CTR_ZERO;
    EPwm8Regs.TBCTR = 0x0000;                       // Clear counter
    EPwm8Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;  // Count up/down
    EPwm8Regs.TBCTL.bit.PHSEN = TB_DISABLE;         // Disable phase loading
    EPwm8Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;        // Clock ratio to SYSCLKOUT
    EPwm8Regs.TBCTL.bit.CLKDIV = TB_DIV1;

    EPwm8Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;     // Load registers every ZERO
    EPwm8Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO_PRD;
    EPwm8Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;     // Load registers every ZERO
    EPwm8Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO_PRD;

    EPwm8Regs.AQCTLA.bit.PRD = AQ_NO_ACTION;
    EPwm8Regs.AQCTLA.bit.ZRO = AQ_NO_ACTION;
    EPwm8Regs.AQCTLA.bit.CAU = AQ_CLEAR;            // set actions for EPWM1A
    EPwm8Regs.AQCTLA.bit.CAD = AQ_SET;

    //Trigger ADC
    EPwm8Regs.ETSEL.bit.SOCAEN = 1;                 // Enable SOC on A group
    EPwm8Regs.ETSEL.bit.SOCASEL = ET_CTR_PRDZERO;   // Dispara ADC no topo
    EPwm8Regs.ETPS.bit.SOCAPRD = ET_1ST;            // Trigger on every event

    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;
}

void Setup_ADC(void)
{
    uint16_t i = 0;
    EALLOW;
    CpuSysRegs.PCLKCR13.bit.ADC_A = 1;   // habilita clock ADCA
    AdcaRegs.ADCCTL2.bit.PRESCALE = 6;
    AdcSetMode(ADC_ADCA, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
    AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 1;               // set pulse um ciclo antes do resultado
    AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1;                  // liga o adc
    for(i = 0; i < 1000; i++){}
    //DELAY_US(1000);                                     // tempo pra dar de ligar

    AdcaRegs.ADCSOC0CTL.bit.CHSEL = 4;                  // canal A4
    AdcaRegs.ADCSOC0CTL.bit.ACQPS = 14;                 // Tempo de aquisicao 14 pulso de clk eu acho
    AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = 19;               // MUDADO: 19 = EPWM8 SOCA


    AdcaRegs.ADCSOC1CTL.bit.CHSEL = 1;                  // canal A2
    AdcaRegs.ADCSOC1CTL.bit.ACQPS = 14;                 // Tempo de aquisicao 14 pulso de clk eu acho
    AdcaRegs.ADCSOC1CTL.bit.TRIGSEL = 19;               // MUDADO: 19 = EPWM8 SOCA

    AdcaRegs.ADCSOC2CTL.bit.CHSEL = 2;                  // canal A2
    AdcaRegs.ADCSOC2CTL.bit.ACQPS = 14;                 // Tempo de aquisicao 14 pulso de clk eu acho
    AdcaRegs.ADCSOC2CTL.bit.TRIGSEL = 19;               // MUDADO: 19 = EPWM8 SOCA

    AdcaRegs.ADCINTSEL1N2.bit.INT1SEL = 0;              // fim do SOC0 gera ADCINT1
    AdcaRegs.ADCINTSEL1N2.bit.INT1E = 1;                // habilita ADCINT1
    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;              // limpa flag ADCINT1
    EDIS;
}
