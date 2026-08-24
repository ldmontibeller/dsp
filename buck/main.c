#include "F28x_Project.h"
#include "driverlib.h"
#include "device.h"
#include "Peripheral_Setup.h"
#include "math.h"

uint16_t adc_sample = 0;
uint16_t adc_sample1 = 0;
uint16_t adc_sample2 = 0;
uint32_t count = 0;

float il = 0;
float vo = 0;
float f = 100000;
float perc = 0.1666;

// Selecao de malha: 1 = Corrente Pura | 0 = Cascateado (Tensao -> Corrente)
int malha = 1;
int malha_anterior = -1; // Variavel para detectar a transicao de malha

// --- GANHOS CONTINUOS DOS CONTROLES (ESCOPO GLOBAL) ---
float Kp_IL = 18.289 * 0.00054;
float Ki_IL = 18.289;

//Ganhos Leonardo
// float Kp_IL = 271.1858 * 0.001445985684522;
// float Ki_IL = 271.1858;

float Kp_Vo = 33.269 * 0.00055;
float Ki_Vo = 33.269;

// --- VARIAVEIS DA MALHA DE CORRENTE (_IL) ---
float Umin_IL = 0;
float Umax_IL = 0.2;
float v1_IL = 0;
float uIk1_IL = 0;
float uIk_IL = 0;
float w1_IL = 1.0f;
float uP_IL = 0;
float ek_IL = 0;
float U_IL = 0;

// --- VARIAVEIS DA MALHA DE TENSAO (_Vo) ---
float Umin_Vo = 0;
float Umax_Vo = 8; // Limite maximo de referencia para a corrente interna
float v1_Vo = 0;
float uIk1_Vo = 0;
float uIk_Vo = 0;
float w1_Vo = 1.0f;
float uP_Vo = 0;
float ek_Vo = 0;
float U_Vo = 0;

// Sinal de controle final aplicado ao PWM
float U = 0;

// Referencias Globais
float V_ref = 4.0f;
float I_ref = 4.0f;

__interrupt void isr_adc(void);

int main(void)
{
    InitSysCtrl();                          // Initialize System Control:

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.CPUTIMER0 = 1;
    CpuSysRegs.PCLKCR13.bit.ADC_A = 1;
    EDIS;

    DINT;                                   // Disable CPU interrupts
    InitPieCtrl();                          // Initialize the PIE control registers to their default state
    IER = 0x0000;                           // Disable CPU interrupts
    IFR = 0x0000;                           // Clear all CPU interrupt flags:
    InitPieVectTable();                     // Initialize the PIE vector table

    // Funcoes de configuracao
    Setup_GPIO();
    Setup_PWM1();
    Setup_ADC();

    // Vetor de interrupcao do ADC
    EALLOW;
    PieVectTable.ADCA1_INT = &isr_adc;          // Interrupcao do ADCA1 vai para isr_adc
    PieCtrlRegs.PIEIER1.bit.INTx1 = 1;          // Habilita interrupcao INT1.1
    EDIS;

    IER |= M_INT1;   // Habilita grupo 1
    EINT;            // Interrupcoes globais
    ERTM;            // Real-time debug

    while(1)
    {
    }
}

// ISR do ADC (RTI)
__interrupt void isr_adc(void)
{
    GpioDataRegs.GPATOGGLE.bit.GPIO2 = 1;

    adc_sample  = AdcaResultRegs.ADCRESULT0;         // Leitura TENSAO
    adc_sample1 = AdcaResultRegs.ADCRESULT1;         // Leitura CORRENTE
    adc_sample2 = AdcaResultRegs.ADCRESULT2;

    // BANCADA
     vo = adc_sample * 0.002914503635673 + 0.017757021714412;
     il = adc_sample1 *(-0.005166963322187) + 11.806738682736798;

    // TYPHON
    //vo = adc_sample * 0.001712959394298 -0.398833688772408;
    // il = adc_sample2 * 0.002067702366985 + 0.540098947195186;

    // =========================================================================
    // ESTADO INTERMEDIARIO (RESET AUTOMATICO NA COMUTACAO)
    // =========================================================================
    if (malha != malha_anterior) {
        // Zera acoes integrais, saidas e erros acumulados de ambas as malhas
        uIk1_IL = 0.0f;
        uIk_IL  = 0.0f;
        v1_IL   = 0.0f;
        U_IL    = 0.0f;
        w1_IL   = 1.0f;

        uIk1_Vo = 0.0f;
        uIk_Vo  = 0.0f;
        v1_Vo   = 0.0f;
        U_Vo    = 0.0f;
        w1_Vo   = 1.0f;

        U       = 0.0f;

        // Atualiza o estado anterior para nao repetir o reset no proximo ciclo
        malha_anterior = malha;
    }

    // =========================================================================
    // EXECUCAO DAS MALHAS DE CONTROLE
    // =========================================================================
    if (malha == 1) {
        // --- MALHA DE CORRENTE PURA ---
        float Kpd_IL = Kp_IL;
        float Kid_IL = __divf32(Ki_IL, f);

        ek_IL = I_ref - il;
        uP_IL = Kpd_IL * ek_IL;
        uIk_IL = Kid_IL * (w1_IL * ek_IL) + uIk1_IL;
        uIk1_IL = uIk_IL;
        v1_IL = uP_IL + uIk_IL;

        if (v1_IL > Umax_IL) {
            U_IL  = Umax_IL;
            w1_IL = 0.0f;
        }
        else if (v1_IL < Umin_IL) {
            U_IL  = Umin_IL;
            w1_IL = 0.0f;
        }
        else {
            U_IL  = v1_IL;
            w1_IL = 1.0f;
        }

        U = U_IL;
    }
    else {
        // --- CONTROLE CASCATEADO (TENSAO -> CORRENTE) ---

        // 1) Malha Externa de Tensao
        float Kpd_Vo = Kp_Vo;
        float Kid_Vo = __divf32(Ki_Vo, f);

        ek_Vo = V_ref - vo;
        uP_Vo = Kpd_Vo * ek_Vo;
        uIk_Vo = Kid_Vo * (w1_Vo * ek_Vo) + uIk1_Vo;
        uIk1_Vo = uIk_Vo;
        v1_Vo = uP_Vo + uIk_Vo;

        if (v1_Vo > Umax_Vo) {
            U_Vo  = Umax_Vo;
            w1_Vo = 0.0f;
        }
        else if (v1_Vo < Umin_Vo) {
            U_Vo  = Umin_Vo;
            w1_Vo = 0.0f;
        }
        else {
            U_Vo  = v1_Vo;
            w1_Vo = 1.0f;
        }

        // 2) Malha Interna de Corrente (Referencia = U_Vo)
        float Kpd_IL = Kp_IL;
        float Kid_IL = __divf32(Ki_IL, f);

        ek_IL = U_Vo - il;
        uP_IL = Kpd_IL * ek_IL;
        uIk_IL = Kid_IL * (w1_IL * ek_IL) + uIk1_IL;
        uIk1_IL = uIk_IL;
        v1_IL = uP_IL + uIk_IL;

        if (v1_IL > Umax_IL) {
            U_IL  = Umax_IL;
            w1_IL = 0.0f;
        }
        else if (v1_IL < Umin_IL) {
            U_IL  = Umin_IL;
            w1_IL = 0.0f;
        }
        else {
            U_IL  = v1_IL;
            w1_IL = 1.0f;
        }

        U = U_IL;
    }

    // Aplica o ciclo de trabalho calculado ao PWM
    EPwm4Regs.CMPA.bit.CMPA = 1000 * U;

    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;          // Limpa flag
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}
