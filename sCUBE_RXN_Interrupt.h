/*
 * sCUBE_RXN_Interrupt.h
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "DSP2833x_Device.h"     				// DSP2833x Header file Include File
#include "DSP2833x_Examples.h"   				// DSP2833x Examples Include File
#include "sCUBE_RXN_Define.h"

void Init_Timer_Setup(void);         			// 타이머 초기화 함수
void Init_Scia_Setup(void);						// 시리얼A 초기화 함수
void Init_ADC_Setup(void);            			// ADC 초기화 함수

interrupt void cpu_timer0_isr(void); 			// 타이머0 ISR
interrupt void scia_tx_isr(void);				// 시리얼A 송신 ISR
interrupt void scia_rx_isr(void);				// 시리얼A 수신 ISR
interrupt void adc_isr(void);         			// ADC ISR

#define R_BUF_SIZE 256  // 버퍼 크기 (2의 승수가 연산 효율에 좋음)
#define T_BUF_SIZE 256

// 수신(RX) 링버퍼
volatile char RxBuf[R_BUF_SIZE];
volatile unsigned int RxHead = 0;
volatile unsigned int RxTail = 0;

// 송신(TX) 링버퍼
volatile char TxBuf[T_BUF_SIZE];
volatile unsigned int TxHead = 0;
volatile unsigned int TxTail = 0;

volatile unsigned long g_TimerCount = 0; 		// 1ms마다 증가할 카운터
volatile Uint16 g_TempRaw = 0;					// 가열로 온도 ADC 원본값 (0 ~ 4095)


/* =========================================================================
 * Timer 초기화 함수 (Init_Timer_Setup)
 * ========================================================================= */
void Init_Timer_Setup(void)
{
    EALLOW;

    // [Step 1] 타이머 기본 초기화
    // - DSP2833x_CpuTimers.c에 정의된 표준 함수 사용
    // - Timer 0, 1, 2의 레지스터 포인터를 연결하고 초기화함
    InitCpuTimers();

    // [Step 2] 타이머 주기 설정 (ConfigCpuTimer 함수 사용)
    // - 파라미터: (&타이머구조체, CPU클럭MHz, 주기uS)
    // - 여기서는 150MHz CPU에서 1000uS(1ms) 주기로 설정
    ConfigCpuTimer(&CpuTimer0, 150, 1000);

    // [Step 3] 인터럽트 벡터 연결
    // - Timer 0 인터럽트가 발생하면 'cpu_timer0_isr' 함수로 점프하도록 설정
    PieVectTable.TINT0 = &cpu_timer0_isr;

    // [Step 4] 타이머 시작 및 인터럽트 활성화
    // - ConfigCpuTimer는 설정을 마친 후 타이머를 멈춰 둡니다.
    // - 아래 설정을 통해 인터럽트를 켜고 타이머를 시작해야 합니다.

    CpuTimer0Regs.TCR.all = 0x4001; // bit 4(TSS)=1:Stop, bit 5(TRB)=1:Reload, bit 14(TIE)=1:Enable Interrupt

    CpuTimer0Regs.TCR.bit.TSS = 0;  // 0: Start Timer (타이머 시작)

    // [Step 5] PIE 및 CPU 인터럽트 허용
    // - Timer 0는 PIE Group 1, INT 7에 할당되어 있음
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;  // PIE Group 1, INT 7 (TINT0) 활성화

    // - CPU Core 인터럽트 INT1 활성화 (TINT0는 INT1에 연결됨)
    IER |= M_INT1;

    EDIS;
}

/* =========================================================================
 * SCIA FIFO 및 인터럽트 초기화 함수 (Init_Scia_Setup)
 * ========================================================================= */
void Init_Scia_Setup(void)
{
	EALLOW; // 보호 레지스터 쓰기 허용

	/* ---------------------------------------------------
	 * 3. FIFO 및 인터럽트 설정 (링버퍼용 핵심)
	 * --------------------------------------------------- */

	// TX FIFO 설정
	SciaRegs.SCIFFTX.all = 0xC000;
	// bit 15 (SCIFFENA) = 1 : FIFO 기능 켜기
	// bit 14 (SCIFFTXRESET) = 1 : FIFO 리셋
	// bit 5  (TXFFIENA) = 0 : **중요** 초기에는 TX 인터럽트를 껍니다.
	// (데이터를 보낼 때만 켜야 함, 안 그러면 무한 인터럽트 발생)

	// RX FIFO 설정
	SciaRegs.SCIFFRX.all = 0x0021;
	// bit 14 (RXFFOVRCLR) = 0 : 오버플로우 플래그 클리어 안함 (초기)
	// bit 13 (RXFFRESET) = 1 : FIFO 리셋
	// bit 0-4 (RXFFIL) = 1 : **1바이트만 들어와도 인터럽트 발생** (반응성 향상)

	// FIFO 제어 레지스터
	SciaRegs.SCIFFCT.all = 0x00; // Auto-baud 비활성화

	/* ---------------------------------------------------
	 * 4. 동작 시작 및 PIE 벡터 연결
	 * --------------------------------------------------- */

	// SCI 모듈 리셋 해제 (동작 시작)
	SciaRegs.SCICTL1.bit.SWRESET = 1;

	// 인터럽트 벡터 테이블 매핑 (ISR 함수 주소 연결)
	PieVectTable.SCIRXINTA = &scia_rx_isr;
	PieVectTable.SCITXINTA = &scia_tx_isr;

	// PIE(Peripheral Interrupt Expansion) 그룹 9 활성화
	PieCtrlRegs.PIEIER9.bit.INTx1 = 1; // SCIRXINTA (RX 인터럽트)
	PieCtrlRegs.PIEIER9.bit.INTx2 = 1; // SCITXINTA (TX 인터럽트)

	// CPU 인터럽트(INT9) 활성화
	IER |= M_INT9;

	EDIS; // 보호 레지스터 쓰기 금지
}

/* =========================================================================
 * ADC 초기화 함수 (Init_ADC_Setup)
 * ========================================================================= */
void Init_ADC_Setup(void)
{
    // ADC 전원 및 캘리브레이션 (TI 라이브러리 사용)
    InitAdc();

    EALLOW;

    // ADC 클럭 및 샘플링 윈도우 설정
    // - ADCCLK = HSPCLK(예: 75MHz) / (2 * ADCCLKPS)
    // - ACQ_PS: LM324 OP-Amp를 거치므로 임피던스 매칭을 위해 샘플링 윈도우를 넉넉하게(16클럭) 설정
    AdcRegs.ADCTRL3.bit.ADCCLKPS = 6;    // 클럭 분주 (HSPCLK/12) -> 약 12.5MHz (안정적)
    AdcRegs.ADCTRL1.bit.CPS = 0;         // Pre-scaler 0
    // - ACQ_PS (Acquisition Window Size) 설정 // - LM324 OP-Amp를 거쳐 들어오므로 임피던스를 고려해 윈도우를 충분히 확보(16클럭)
    AdcRegs.ADCTRL1.bit.ACQ_PS = 0xF;

    // 시퀀서 설정 (Cascaded Mode) : SEQ1 하나만 사용하여 간단하게 제어
    AdcRegs.ADCTRL1.bit.SEQ_CASC = 1;        // 1: Cascaded Mode

    // 채널 선택 (ADCINA0 - 가열로 온도 센서)
    // - 나머지 ADCINA1 ~ ADCINA7은 사용하지 않으므로 설정 불필요
    AdcRegs.ADCCHSELSEQ1.bit.CONV00 = 0x0;   // CONV00 -> ADCINA0 선택
    // - CONV00 하나만 변환하므로 MAX_CONV1 = 0
    AdcRegs.ADCMAXCONV.bit.MAX_CONV1 = 0;

    // 인터럽트 설정 (핵심)
    // - SEQ1 변환이 끝나면 인터럽트 발생하도록 설정
    AdcRegs.ADCTRL2.bit.INT_ENA_SEQ1 = 1;    // SEQ1 인터럽트 활성화
    AdcRegs.ADCTRL2.bit.INT_MOD_SEQ1 = 0;    // 0: 매 EOS마다 인터럽트 발생
    AdcRegs.ADCTRL2.bit.RST_SEQ1 = 1;        // 시퀀서 리셋
    AdcRegs.ADCST.bit.INT_SEQ1_CLR = 1;      // 인터럽트 플래그 클리어

    // PIE 벡터 테이블에 ISR 주소 매핑
    PieVectTable.ADCINT = &adc_isr;

    // PIE 및 CPU 인터럽트 활성화 - PIE Group 1, INT 6 (ADCINT)
    PieCtrlRegs.PIEIER1.bit.INTx6 = 1;

    // - CPU INT 1 활성화
    IER |= M_INT1;

    EDIS;
}

/* =========================================================================
 * 타이머 0 인터럽트 서비스 루틴 (cpu_timer0_isr)
 * - 1ms마다 이 함수가 실행됩니다.
 * ========================================================================= */
interrupt void cpu_timer0_isr(void)
{
    // [기능 구현]
    // 예: 시스템 카운터 증가, LED 점멸, 주기적인 작업 트리거 등
    g_TimerCount++;

    // [예시] 500ms마다 LED 토글
    // if((g_TimerCount % 500) == 0) {
    //     GpioDataRegs.GPBTOGGLE.bit.GPIO48 = 1; // STATUS LED 깜빡임
    // }

    // [필수] 인터럽트 응답(ACK) 신호 전송
    // - 이 구문이 없으면 다음 인터럽트가 들어오지 않습니다.
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

/* =========================================================================
 * 수신(RX) 인터럽트 서비스 루틴 (scia_rx_isr)
 * - 하드웨어 FIFO에 데이터가 들어와 인터럽트가 발생하면, 즉시 모든 데이터를 꺼내 소프트웨어 링버퍼로 이동한다.
 * ========================================================================= */
interrupt void scia_rx_isr(void)
{
    // 1. 하드웨어 FIFO에 데이터가 있는 동안 계속 읽기
    while(SciaRegs.SCIFFRX.bit.RXFFST != 0)
    {
        char data = SciaRegs.SCIRXBUF.all; // 데이터 읽기

        // 2. 링버퍼에 저장 (Head 이동)
        unsigned int nextHead = (RxHead + 1) % R_BUF_SIZE;
        if(nextHead != RxTail) { // 버퍼가 꽉 차지 않았을 때만 저장
            RxBuf[RxHead] = data;
            RxHead = nextHead;
        }
        // else { 버퍼 오버플로우 처리 (옵션) }
    }

    // 3. 인터럽트 플래그 클리어 & PIE ACK
    SciaRegs.SCIFFRX.bit.RXFFOVRCLR = 1; // 오버플로우 플래그 클리어
    SciaRegs.SCIFFRX.bit.RXFFINTCLR = 1; // 인터럽트 플래그 클리어
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP9;
}

/* =========================================================================
 * 송신(TX) 인터럽트 서비스 루틴 (scia_tx_isr)
 * - 링버퍼에 보낼 데이터가 있다면, 하드웨어 FIFO가 비어있을 때 인터럽트가 걸려 데이터를 채워넣는다.
 * 데이터를 다 보내면 인터럽트를 꺼야(Disable) 한다.
 * ========================================================================= */
interrupt void scia_tx_isr(void)
{
    // 1. 하드웨어 FIFO가 가득 차지 않았고, 보낼 데이터(링버퍼)가 있는 동안 루프
    while( (SciaRegs.SCIFFTX.bit.TXFFST < 16) && (TxHead != TxTail) )
    {
        // 링버퍼에서 데이터 꺼내서 하드웨어 FIFO에 넣기
        SciaRegs.SCITXBUF = TxBuf[TxTail];
        TxTail = (TxTail + 1) % T_BUF_SIZE;
    }

    // 2. 더 이상 보낼 데이터가 없으면 TX 인터럽트 비활성화
    if(TxHead == TxTail) {
        SciaRegs.SCIFFTX.bit.TXFFIENA = 0; // 중요: 계속 인터럽트 걸리는 것 방지
    }

    // 3. 인터럽트 플래그 클리어 & PIE ACK
    SciaRegs.SCIFFTX.bit.TXFFINTCLR = 1;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP9;
}

/* =========================================================================
 * ADC 인터럽트 서비스 루틴 (adc_isr)
 * - ADC 변환이 완료되면 자동으로 호출되어 온도값을 저장한다.
 * ========================================================================= */
interrupt void adc_isr(void)
{
    // [Step A] 변환된 값 읽기 (ADCINA0 -> ADCRESULT0)
    // - F28335는 12비트 결과가 상위 4비트 시프트되어 저장됨 (>> 4 필요)
	g_TempRaw = AdcRegs.ADCRESULT0 >> 4;

    // [Step B] 다음 변환을 위한 초기화
    // - 반드시 ADC 내부 인터럽트 플래그를 클리어해야 다음 인터럽트가 발생함
    AdcRegs.ADCTRL2.bit.RST_SEQ1 = 1;       // 시퀀서 리셋
    AdcRegs.ADCST.bit.INT_SEQ1_CLR = 1;     // INT_SEQ1 플래그 클리어

    // - PIE Acknowledge (Group 1)
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

#ifdef __cplusplus
}
#endif /* extern "C" */
