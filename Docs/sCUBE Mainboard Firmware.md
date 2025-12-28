
- Firmware Structure
- sCUBE_RXN_Base : 기본적인 TMS320F28335를 구동하는데 필요한 코드를

아래의 헤더들은 sCUBE Mainboard Peripheral 구동을 위해서 추가 작성 할 코드들

1. sCUBE_RXN_Define.h : sCUBE Mainboard에 사용되는 변수
2. sCUBE_RXN_GPIO.h : sCUBEMainboard GPIO 관련 함수(솔레노이드/다이어프램펌프/SSR/LED)와 관련된 함수
3. sCUBE_RXN_Interrupt.h : sCUBE Mainboard Interrupt(Timer, ADC, Serial) 관련 함수
4. sCUBE_RXN_Motor.h : sCUBE Mainboard Motor 관련 함수 
5. sCUBE_RXN_Process.h : sCUBE Maonboard Heater/Serial 관련 함수
6. sCUBE_RXN_Main.c : sCUBE Mainboard Main 함수
7. sCUBE_RXN_Test.c : sCUBE Mainboard 테스트 함수
