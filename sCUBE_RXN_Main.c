/*
 * main.c
 *
 * Created on: 2024. 12. 28.
 * Author: griffon0
 */

#include "DSP2833x_Device.h"				   	// DSP2833x Header file Include File
#include "DSP2833x_Examples.h"					// DSP2833x Examples Include File

#include "sCUBE_RXN_Define.h"					// sCUBE Mainboard Variable
#include "sCUBE_RXN_GPIO.h"						// sCUBE Mainboard GPIO Function(Solenoid/Diaphragm/SSR/LED)
#include "sCUBE_RXN_Interrupt.h"				// sCUBE Mainboard Interrupt Function(Timer, ADC, Serial)
// #include "sCUBE_RXN_Motor.h"					// sCUBE Mainboard Motor Function
// #include "sCUBE_RXN_Process.h"				// sCUBE Maonboard Heater/Serial Function
#include "sCUBE_RXN_Test.h"						// sCUBE Mainboard Test Function

extern void InitXintf(void);

void main(void)
{
	// [Step 1] 시스템 클럭 및 기본 초기화
	InitSysCtrl();								// PLL, WatchDog, Peripheral Clocks 초기화

	// [Step 2] GPIO 및 주변장치 하드웨어 설정
	InitSciaGpio();								// SCI-A GPIO 및 통신 설정 (Baudrate:115200bps)
	Init_Sol_Gpio_Setup();						// 솔레노이드 밸브 및 LED 초기화
	Init_SSR_Gpio_Setup();						// 히터 SSR 초기화
	Init_Pump_Gpio_Setup();						// 펌프 제어(PWM) 초기화
	InitXintf();								// 8255 제어를 위한 XINTF 초기화 (주의!! 시리얼통신 단자와 연관 있음)
	
	// [Step 3] 인터럽트 초기화 (기본 작업)
	DINT;               						// 전체 인터럽트 비활성화
	InitPieCtrl();      						// PIE 제어 레지스터 초기화
	IER = 0x0000;       						// CPU 인터럽트 비활성화
	IFR = 0x0000;       						// CPU 플래그 클리어
	InitPieVectTable(); 						// 기본 벡터 테이블 복사

	// [Step 4] Interrupt Vector Remapping (사용자 ISR 연결)
	EALLOW;  									// 보호된 레지스터(PIE RAM) 쓰기 허용

	PieVectTable.TINT0 = &cpu_timer0_isr;		// CPU Timer 0 인터럽트 (Group 1, INT 7) -> cpu_timer0_isr 연결
	PieVectTable.ADCINT = &adc_isr; 			// ADC 인터럽트 (Group 1, INT 6) -> adc_isr 연결
	PieVectTable.SCIRXINTA = &scia_rx_isr;		// SCI-A 수신 인터럽트 (Group 9, INT 1) -> scia_rx_isr 연결
	PieVectTable.SCITXINTA = &scia_tx_isr;		// SCI-A 송신 인터럽트 (Group 9, INT 2) -> scia_tx_isr 연결

	EDIS;    									// 보호된 레지스터 쓰기 금지

	// [Step 5] 개별 인터럽트 활성화 및 모듈 시작
	Init_Scia_Setup();
	Init_ADC_Setup();
	Init_Timer_Setup();

	// [Step 6] 전역 인터럽트 활성화
	EINT;   // Enable Global interrupt INTM
	ERTM;   // Enable Global realtime interrupt DBGM

	// [Step 7] 메인 루프
	while(1)
	{
		// 메인 루프
		Test_Scia_Sequence_Run();	
	}
}
