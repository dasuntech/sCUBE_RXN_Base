/*
 * sCUBE_RXN_Test.h
 *
 * Created on: 2024. 12. 28.
 * Author: griffon0
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "DSP2833x_Device.h"     // DSP2833x Header file Include File
#include "DSP2833x_Examples.h"   // DSP2833x Examples Include File

// sCUBE_RXN_Interrupt.h 에 정의된 전역 변수 참조
extern volatile char TxBuf[];
extern volatile unsigned int TxHead;
extern volatile unsigned int TxTail;
extern volatile unsigned long g_TimerCount; // 1ms 카운터

// 솔레노이드 테스트 시퀀스 함수
void Solenoid_Test_Sequence(void)
{
    int i;
    // 솔레노이드 GPIO 핀 매핑 배열 (총 16개)
    // SOL_01~06 (Port A), SOL_07~16 (Port B)
    Uint16 sol_gpios[16] = {6, 7, 8, 9, 10, 11, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59};

    // ---------------------------------------------------
    // [단계 1] 전원 인가 후 전체 SOL ON (1초간 유지)
    // ---------------------------------------------------
    for(i = 0; i < 16; i++)
    {
        if(sol_gpios[i] < 32)
            GpioDataRegs.GPASET.all = (1UL << sol_gpios[i]);      // Port A Set
        else
            GpioDataRegs.GPBSET.all = (1UL << (sol_gpios[i]-32)); // Port B Set
    }

    DELAY_US(1000000); // 1초 대기 (1,000,000 us)

    // ---------------------------------------------------
    // [단계 2] 전체 SOL OFF (1초간 유지)
    // ---------------------------------------------------
    for(i = 0; i < 16; i++)
    {
        if(sol_gpios[i] < 32)
            GpioDataRegs.GPACLEAR.all = (1UL << sol_gpios[i]);      // Port A Clear
        else
            GpioDataRegs.GPBCLEAR.all = (1UL << (sol_gpios[i]-32)); // Port B Clear
    }

    DELAY_US(1000000); // 1초 대기

    // ---------------------------------------------------
    // [단계 3] SOL_01 ~ SOL_16 순차적 1초 ON/OFF
    // ---------------------------------------------------
    for(i = 0; i < 16; i++)
    {
        // 1. 해당 솔레노이드 ON
        if(sol_gpios[i] < 32)
            GpioDataRegs.GPASET.all = (1UL << sol_gpios[i]);
        else
            GpioDataRegs.GPBSET.all = (1UL << (sol_gpios[i]-32));

        // 2. 1초 대기
        DELAY_US(1000000);

        // 3. 해당 솔레노이드 OFF
        if(sol_gpios[i] < 32)
            GpioDataRegs.GPACLEAR.all = (1UL << sol_gpios[i]);
        else
            GpioDataRegs.GPBCLEAR.all = (1UL << (sol_gpios[i]-32));
    }
}

/* =========================================================================
 * [내부 함수] 링버퍼에 문자 1개 넣고 전송 시작 (ISR 트리거)
 * ========================================================================= */
void SCIA_Write_Char(char data)
{
    unsigned int nextHead = (TxHead + 1) % T_BUF_SIZE;

    // 버퍼가 꽉 차지 않았을 때만 저장
    if(nextHead != TxTail) {
        TxBuf[TxHead] = data;
        TxHead = nextHead;

        // TX 인터럽트 활성화 (데이터 전송 시작)
        EALLOW;
        SciaRegs.SCIFFTX.bit.TXFFIENA = 1;
        EDIS;
    }
}

/* =========================================================================
 * [메인 호출용] 시퀀스 동작 테스트 함수
 * 1. (초기화 단계에서 이미 SOL 1~16 Clear 됨)
 * 2. 1초 간격으로 "Hello TMS320F28335\n" 문자 전송
 * 3. 전송 완료 후 SOL_01 (GPIO6) 토글
 * ========================================================================= */
void Test_Scia_Sequence_Run(void)
{
    static unsigned long last_time = 0;     // 마지막 실행 시간
    static int idx = 0;                     // 문자 인덱스
    static int done = 0;                    // 완료 플래그
    char *msg = "Hello TMS320F28335\n";     // 보낼 메시지

    // 이미 완료되었다면 함수 종료
    if (done) return;

    // 1초(1000ms)가 지났는지 확인
    if ((unsigned long)(g_TimerCount - last_time) >= 1000)
    {
        last_time = g_TimerCount; // 시간 갱신

        // 문자열 끝이 아니면 전송
        if (msg[idx] != '\0')
        {
            SCIA_Write_Char(msg[idx]);
            idx++;
        }
        else
        {
            // [완료 동작] SOL_01 (GPIO6) 토글
            EALLOW;
            GpioDataRegs.GPATOGGLE.bit.GPIO6 = 1;
            EDIS;

            done = 1; // 완료 플래그 설정 (더 이상 실행 안 함)
        }
    }
}

// [진단용 함수] 타이머/인터럽트 없이 깡통으로 동작하는지 확인
void Test_Direct_Loop(void)
{
    // 1. 문자열 전송 (폴링 방식)
    char *msg = "System Running...\r\n";
    int i = 0;
    while(msg[i] != '\0')
    {
        // FIFO가 찰 때까지만 대기하고 바로 씀 (인터럽트 의존 X)
        while(SciaRegs.SCIFFTX.bit.TXFFST >= 16) {}
        SciaRegs.SCITXBUF = msg[i++];
    }

    // 2. 1초 지연 (인터럽트 없이 CPU가 직접 셈)
    DELAY_US(1000000);

    // 3. GPIO 6 토글 (살아있는지 확인)
    EALLOW;
    GpioDataRegs.GPATOGGLE.bit.GPIO6 = 1;
    EDIS;
}

// [최종 진단용] 모든 설정 강제 초기화 후 'A' 무한 전송
void Test_Scia_Panic(void)
{
    // 1. SCI 강제 초기화 (기존 설정 무시)
    EALLOW;
    SciaRegs.SCICTL1.bit.SWRESET = 0; // 리셋

    // 통신 설정: 8-N-1
    SciaRegs.SCICCR.all = 0x0007;

    // TX, RX Enable
    SciaRegs.SCICTL1.all = 0x0003;

    // Baudrate: 115200 ( @ LSPCLK 37.5MHz )
    // 만약 크리스탈이 달라 클럭이 150MHz가 아니라면 이 값이 틀릴 수 있음
    SciaRegs.SCIHBAUD = 0x1;
    SciaRegs.SCILBAUD = 0xE7; // 40

    // FIFO 비활성화 (가장 단순한 모드)
    SciaRegs.SCIFFTX.bit.SCIFFENA = 0;

    // 리셋 해제
    SciaRegs.SCICTL1.bit.SWRESET = 1;
    EDIS;

    // 2. 무한 루프
    while(1)
    {
        // TXRDY(송신 준비) 될 때까지 대기
        while(!(SciaRegs.SCICTL2.bit.TXRDY)) {
        	SciaRegs.SCITXBUF = 'A' & 0x00FF;
        }

        // 0.1초 지연
        DELAY_US(100000);

        // 생존 신고: SOL_01(GPIO6) 토글
        EALLOW;
        GpioDataRegs.GPATOGGLE.bit.GPIO6 = 1;
        EDIS;
    }
}

void scia_Tx(char data)												// Send data _ DSP -> PC
{																	// One character or One of the variables
	while(!(SciaRegs.SCICTL2.bit.TXRDY));
    SciaRegs.SCITXBUF = data & 0x00FF;
}

void string_Tx(const char * msg)									// Send a text
{
	char ch;
	if(msg)
		while(0 != (ch = *(msg++))){
			scia_Tx(ch);
		}
}

#ifdef __cplusplus
}
#endif /* extern "C" */
