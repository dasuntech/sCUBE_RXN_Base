/*
 * sCUBE_RXN_GPIO.h
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "DSP2833x_Device.h"     // DSP2833x Headerfile Include File
#include "DSP2833x_Examples.h"   // DSP2833x Examples Include File

/* =========================================================================
 * 솔레노이드, LED, 8255 리셋 설정 함수
 * ========================================================================= */

void Init_Sol_Gpio_Setup(void)
{
    EALLOW; // 보호 레지스터 쓰기 허용

    /* =============================================================
     * 1. Port A 설정 (GPIO 06 ~ GPIO 11 : SOL_01 ~ SOL_06)
     * ============================================================= */

    // GPIO06 : SOL_01
    GpioCtrlRegs.GPAMUX1.bit.GPIO6 = 0;      // 0:GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO6 = 1;       // 1:Output
    GpioDataRegs.GPACLEAR.bit.GPIO6 = 1;     // 초기값 Low (Close)

    // GPIO07 : SOL_02
    GpioCtrlRegs.GPAMUX1.bit.GPIO7 = 0;
    GpioCtrlRegs.GPADIR.bit.GPIO7 = 1;
    GpioDataRegs.GPACLEAR.bit.GPIO7 = 1;

    // GPIO08 : SOL_03
    GpioCtrlRegs.GPAMUX1.bit.GPIO8 = 0;
    GpioCtrlRegs.GPADIR.bit.GPIO8 = 1;
    GpioDataRegs.GPACLEAR.bit.GPIO8 = 1;

    // GPIO09 : SOL_04
    GpioCtrlRegs.GPAMUX1.bit.GPIO9 = 0;
    GpioCtrlRegs.GPADIR.bit.GPIO9 = 1;
    GpioDataRegs.GPACLEAR.bit.GPIO9 = 1;

    // GPIO10 : SOL_05
    GpioCtrlRegs.GPAMUX1.bit.GPIO10 = 0;
    GpioCtrlRegs.GPADIR.bit.GPIO10 = 1;
    GpioDataRegs.GPACLEAR.bit.GPIO10 = 1;

    // GPIO11 : SOL_06
    GpioCtrlRegs.GPAMUX1.bit.GPIO11 = 0;
    GpioCtrlRegs.GPADIR.bit.GPIO11 = 1;
    GpioDataRegs.GPACLEAR.bit.GPIO11 = 1;

    /* =============================================================
     * 2. Port B 설정 (LED, SOL_07 ~ SOL_16, 8255_RESET)
     * ============================================================= */

    // --- LEDs (GPIO 48, 49) ---

    // GPIO48 : STATUS_LED
    GpioCtrlRegs.GPBMUX2.bit.GPIO48 = 0;     // 0:GPIO
    GpioCtrlRegs.GPBDIR.bit.GPIO48 = 1;      // 1:Output
    GpioDataRegs.GPBCLEAR.bit.GPIO48 = 1;    // 초기값 Low (Off 가정)

    // GPIO49 : POWER_LED
    GpioCtrlRegs.GPBMUX2.bit.GPIO49 = 0;
    GpioCtrlRegs.GPBDIR.bit.GPIO49 = 1;
    GpioDataRegs.GPBCLEAR.bit.GPIO49 = 1;    // 초기값 Low

    // --- Solenoid Valves 07 ~ 16 (GPIO 50 ~ 59) ---

    // GPIO50 : SOL_07
    GpioCtrlRegs.GPBMUX2.bit.GPIO50 = 0;
    GpioCtrlRegs.GPBDIR.bit.GPIO50 = 1;
    GpioDataRegs.GPBCLEAR.bit.GPIO50 = 1;

    // GPIO51 : SOL_08
    GpioCtrlRegs.GPBMUX2.bit.GPIO51 = 0;
    GpioCtrlRegs.GPBDIR.bit.GPIO51 = 1;
    GpioDataRegs.GPBCLEAR.bit.GPIO51 = 1;

    // GPIO52 : SOL_09
    GpioCtrlRegs.GPBMUX2.bit.GPIO52 = 0;
    GpioCtrlRegs.GPBDIR.bit.GPIO52 = 1;
    GpioDataRegs.GPBCLEAR.bit.GPIO52 = 1;

    // GPIO53 : SOL_10
    GpioCtrlRegs.GPBMUX2.bit.GPIO53 = 0;
    GpioCtrlRegs.GPBDIR.bit.GPIO53 = 1;
    GpioDataRegs.GPBCLEAR.bit.GPIO53 = 1;

    // GPIO54 : SOL_11
    GpioCtrlRegs.GPBMUX2.bit.GPIO54 = 0;
    GpioCtrlRegs.GPBDIR.bit.GPIO54 = 1;
    GpioDataRegs.GPBCLEAR.bit.GPIO54 = 1;

    // GPIO55 : SOL_12
    GpioCtrlRegs.GPBMUX2.bit.GPIO55 = 0;
    GpioCtrlRegs.GPBDIR.bit.GPIO55 = 1;
    GpioDataRegs.GPBCLEAR.bit.GPIO55 = 1;

    // GPIO56 : SOL_13
    GpioCtrlRegs.GPBMUX2.bit.GPIO56 = 0;
    GpioCtrlRegs.GPBDIR.bit.GPIO56 = 1;
    GpioDataRegs.GPBCLEAR.bit.GPIO56 = 1;

    // GPIO57 : SOL_14
    GpioCtrlRegs.GPBMUX2.bit.GPIO57 = 0;
    GpioCtrlRegs.GPBDIR.bit.GPIO57 = 1;
    GpioDataRegs.GPBCLEAR.bit.GPIO57 = 1;

    // GPIO58 : SOL_15
    GpioCtrlRegs.GPBMUX2.bit.GPIO58 = 0;
    GpioCtrlRegs.GPBDIR.bit.GPIO58 = 1;
    GpioDataRegs.GPBCLEAR.bit.GPIO58 = 1;

    // GPIO59 : SOL_16
    GpioCtrlRegs.GPBMUX2.bit.GPIO59 = 0;
    GpioCtrlRegs.GPBDIR.bit.GPIO59 = 1;
    GpioDataRegs.GPBCLEAR.bit.GPIO59 = 1;

    // --- 8255 Reset (GPIO 61) ---

    // GPIO61 : 8255_RESET
    GpioCtrlRegs.GPBMUX2.bit.GPIO61 = 0;     // 0:GPIO
    GpioCtrlRegs.GPBDIR.bit.GPIO61 = 1;      // 1:Output
    GpioDataRegs.GPBCLEAR.bit.GPIO61 = 1;    // 초기값 Low (Reset Inactive)
    // 8255는 일반적으로 RESET핀이 Active High이므로, 평소에는 Low로 유지해야 동작합니다.

    EDIS; // 보호 레지스터 쓰기 금지
}

/* =========================================================================
 * SSR(Solid State Relay) 히터 제어 설정 함수
 * ========================================================================= */

void Init_SSR_Gpio_Setup(void)
{
    EALLOW; // 보호 레지스터 쓰기 허용

    /* =============================================================
     * SSR (Solid State Relay) 설정 - Port A
     * ============================================================= */

    // --- SSR_1_Enable (Heater 1) : GPIO0 ---
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 0;      // 0:GPIO, 1:EPWM1A, 2:Resv, 3:Resv
    GpioCtrlRegs.GPADIR.bit.GPIO0 = 1;       // 1:Output
    GpioDataRegs.GPACLEAR.bit.GPIO0 = 1;     // 초기 상태: Low (Heater Off)

    // --- SSR_2_Enable (Heater 2) : GPIO1 ---
    GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 0;      // 0:GPIO, 1:EPWM1B, 2:ECAP6, 3:Resv
    GpioCtrlRegs.GPADIR.bit.GPIO1 = 1;       // 1:Output
    GpioDataRegs.GPACLEAR.bit.GPIO1 = 1;     // 초기 상태: Low (Heater Off)

    EDIS; // 보호 레지스터 쓰기 금지
}


/* =========================================================================
 * 펌프 제어 시스템 설정 (GPIO + ePWM for L298)
 * - L298 발열 및 소음 최적화를 위해 20kHz PWM 사용
 * ========================================================================= */
void Init_Pump_Gpio_Setup(void)
{
    // --- A. 펌프 GPIO 핀 설정 (Enable & PWM Pin Mux) ---

    // Pump 1 Enable (GPIO 2)
    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 0;      // GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO2 = 1;       // Output
    GpioDataRegs.GPACLEAR.bit.GPIO2 = 1;     // Disable (Off)

    // Pump 1 PWM (GPIO 3 -> ePWM2B)
    GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 1;      // ePWM2B 기능 선택
    GpioCtrlRegs.GPAPUD.bit.GPIO3 = 1;       // Disable Pull-up

    // Pump 2 PWM (GPIO 4 -> ePWM3A)
    GpioCtrlRegs.GPAMUX1.bit.GPIO4 = 1;      // ePWM3A 기능 선택
    GpioCtrlRegs.GPAPUD.bit.GPIO4 = 1;       // Disable Pull-up

    // Pump 2 Enable (GPIO 5)
    GpioCtrlRegs.GPAMUX1.bit.GPIO5 = 0;      // GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO5 = 1;       // Output
    GpioDataRegs.GPACLEAR.bit.GPIO5 = 1;     // Disable (Off)

    // --- B. ePWM 모듈 설정 (20kHz 생성) ---
    // SYSCLKOUT = 150MHz 가정
    // TBCLK = 150MHz (HSPCLKDIV=1, CLKDIV=1)
    // Desired Freq = 20kHz
    // TBPRD = (150,000,000 / 20,000) - 1 = 7499

    /* ePWM2 설정 (Pump 1) */
    EPwm2Regs.TBPRD = 7499;              // 20kHz 설정
    EPwm2Regs.TBPHS.half.TBPHS = 0;      // Phase 0
    EPwm2Regs.TBCTR = 0;                 // Clear Counter

    EPwm2Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP; // Up Count Mode
    EPwm2Regs.TBCTL.bit.PHSEN = TB_DISABLE;
    EPwm2Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
    EPwm2Regs.TBCTL.bit.CLKDIV = TB_DIV1;

    // Action Qualifier (GPIO3/ePWM2B): 0에서 High, CMPB에서 Low
    EPwm2Regs.AQCTLB.bit.ZRO = AQ_SET;
    EPwm2Regs.AQCTLB.bit.CBU = AQ_CLEAR;

    EPwm2Regs.CMPB = 0; // 초기 속도 0

    /* ePWM3 설정 (Pump 2) */
    EPwm3Regs.TBPRD = 7499;              // 20kHz 설정
    EPwm3Regs.TBPHS.half.TBPHS = 0;
    EPwm3Regs.TBCTR = 0;

    EPwm3Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;
    EPwm3Regs.TBCTL.bit.PHSEN = TB_DISABLE;
    EPwm3Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
    EPwm3Regs.TBCTL.bit.CLKDIV = TB_DIV1;

    // Action Qualifier (GPIO4/ePWM3A): 0에서 High, CMPA에서 Low
    EPwm3Regs.AQCTLA.bit.ZRO = AQ_SET;
    EPwm3Regs.AQCTLA.bit.CAU = AQ_CLEAR;

    EPwm3Regs.CMPA.half.CMPA = 0; // 초기 속도 0
}

#ifdef __cplusplus
}
#endif /* extern "C" */
