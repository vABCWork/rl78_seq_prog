
#include "iodefine.h"
#include "misratypes.h"

#include "ctsu.h"
#include "key.h"
#include "interval_timer_12bit.h"


volatile uint8_t  cnt_10_msec;

volatile uint8_t  flg_100_msec;


// intera_timerをベクタアドレス(INIT=0x0038)に登録
#pragma interrupt INT_interval_timer(vect=INTIT)

// インターバルタイマ割り込み処理 (10msec毎に発生)
// 用途:
//      1) CTSUの計測開始トリガ信号(10msec毎)
//      2) 100msec経過フラグのセット
//	3) タッチされたキーの判定とキー入力確定
//      4) ステッピングモータへのパルス出力
//
void INT_interval_timer(void)
{
	
	cnt_10_msec = cnt_10_msec + 1;
	
	if ( cnt_10_msec > 9 ) {	// 100 msec経過
	   cnt_10_msec = 0;		// 10msecカウンタ　クリア
	   flg_100_msec = 1;		// フラグ 100msecのセット
	}
	
	
        touch_key_status_check();	//  各キーのタッチ有り判定値と比較して、タッチの有無を判定
	
	sw1_sw2_input_check( 3 );       // モード切り換え(SW1) 入力判定
	
	sw1_sw2_input_check( 2 );       // RUN/STOP,Save(SW2) 入力判定
	
	
	if (( run_flg == 0 )||( teaching_flg == 1)) {	  // 停止またはTeachingモードの場合
	
	    sw3_sw4_input_check( 1 );	// 反時計回り(SW3) 入力判定
	
	    sw3_sw4_input_check( 0 );	// 時計回り(SW4) 入力判定
	
	    motor_pulse_out();		// ステッピングモータへのパルス出力とティーチングデータの格納

	    touch_led_on_off_sw3_sw4();	// LEDのON/OFF (SW3,SW4用)
	}
	
	
	
}


// インターバルタイマ 12bitの初期設定 (10msec) 　CTSU計測開始のトリガ信号
//　
void interval_timer_12bit_ini(void)
{
	OSMC = 0x10;		// 周辺機能へのサブシステム・クロック供給許可, 低速オンチップ・オシレータ・クロック（fIL）(15 [KHz])をインターバルタイマへ供給　

	TMKAEN = 1;		// 12 ビット・インターバル・タイマへ入力クロック供給 (周辺イネーブル・レジスタ0（PER0）のビット 7 )
	
	ITMC = 149;            //  150 - 1 count, 10msec , ( 15[KHz], 1clock=1/15 [msec] , 150 count/ 15 = 10 [msec])
	
	ITMC = ITMC | 0x8000;      // RINTE:カウンタ動作開始
	
	ITMK = 0;		// 12 ビット・インターバル・タイマ割り込みのマスクをクリア
	
} 