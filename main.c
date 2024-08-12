
#include   "iodefine.h"
#include   "misratypes.h"

#include "timer.h"
#include  "ctsu.h"
#include "key.h"
#include "seq_prog.h"
#include  "uart.h"
#include  "interval_timer_12bit.h"
#include  "delay.h"
#include  "led_port.h"
#include   "iic_sd.h"

#include   "data_flash.h"
#include   "debug_port.h"

void mode_disp(void);
// メイン処理
// 
void main(void)
{
	
	DEBUG_PORT_PM = 0;	// P13 出力モード (デバック用)
	
	uart_1_ini();		// UART1 通信の初期化
	
	interval_timer_12bit_ini();	// 12bit インターバルタイマの起動
	
	ctsu_self_set();	//  静電容量式タッチセンサ(CTSU)の設定

	timer_array_ini();	//  入力クロック供給と分周比設定
	timer_ch0_ini();        // インターバルタイマの設定
	timer_ch4_ini();        // ステッピングモータ パルス設定
	
	p16_out_mode();		// ステッピングモータ 回転方向設定

	touch_led_port();	// タッチキー用 LEDポート設定
	uart_1_led_port();      // TXD1,RXD1用 LEDポート設定
	alarm_led_port();	// アラーム用 LEDポート設定
	
	
	read_flash_sw_record();         // データフラッシュのSWの動作レコードを読み出し(4 byte毎)
	copy_sw_record_from_flash();	// データフラッシュ用のバッファ(4 byte単位)から、SWの動作レコード(1 byte単位)へ
	
	
	flash_clock_mode_set();   // データフラッシュの書き込み許可
	
	iic_ini();			// IICAの初期設定
	
	__EI();			// 割り込み許可
	
	
	iic_slave_adrs = 0x3c;    	//  スレーブアドレス = 0x3c ( SO1602 OLED キャラクタディスプレイ 16文字x2行)
	oled_so1602a_ini();		// OLED モジュール SO1602 初期化 (表示 ON)
	 
	touch_key_on_val();		//   各キーのタッチ有り判定値のセット
	
	CTSUSTRT = 1; 		 // CTSU計測動作開始
	
	
	
	while(1)
	{
	  WDTE = 0xac;		// ウオッチドック　カウンタのクリア
		
	  if ( flg_100_msec == 1 ) {   // 100msec経過
		
	  	mode_chg();		// モード変更
		
		mode_disp();		// モード表示
	
		
		if ( run_flg == 1 ) {
		   if ( pre_run_flg == 0 ) {   // stopからrunへの変更時
		       TS0L_bit.no0 = 1;	// チャンネル0カウント開始( 1秒 タイマ )

		       if ( teaching_flg == 1 ) {    // ティーチングモードの場合
		            status_rcd_pt = 0;	     // ストア位置の初期化
			    touch_record_store();    //  動作レコードの作成処理
			}
			else {			     // 通常モードの場合
			    step_number = 0;         // step番号の初期化
			    exec_prog_step();	     // プログラムのステップ実行
			}
			
		   }
		}
		else {			// stopの場合
			TT0L_bit.no0 = 1;	// チャンネル0カウント停止
		}
		
		flg_100_msec = 0;	// フラグ 100msecのクリア
	  }
	
	  
	  if ( flg_1_sec == 1 ) {    // 1秒経過フラグ ON
	    
	    if ( run_flg == 1 ) {    // 実行の場合
	       
	       if ( teaching_flg == 1 ) {   // ティーチングモードの場合
	           touch_record_store();   //  動作レコードの作成処理(1秒毎)
	       }
	       else {    		// 通常モードの場合
	    	  exec_prog_step();	// プログラムのステップ実行
	       }
	    }
	    
	     flg_1_sec = 0;		// 1秒経過フラグのクリア
	   }
	 
	  
	  if ( rcv_over == 1 ) {	// コマンド受信完了
		comm_cmd();		// レスポンス作成、送信
	   	rcv_over = 0;		// コマンド受信フラグのクリア
		rcv_cnt  = 0;		//  受信バイト数の初期
		
	   }
	
	}
	
}

//
//  モードの表示
// 　実行時間: 0.7[msec]
//
void mode_disp(void)
{
	
	if ( teaching_flg == 0 ) {		// 通常モードの場合、
	
           oled_set_ddram_address(0x00);   // 表示開始位置 1行目先頭
	   
	   oled_disp_str("ﾋﾞｰﾙ ｦ ﾄﾞｳｿﾞ",12);     //  ﾋﾞｰﾙ ｦ ﾄﾞｳｿﾞ 表示
	   
           oled_set_ddram_address(0x20);   // 表示開始位置 2行目先頭
	   if ( run_flg == 1 ) {		   //  RUNの場合
	     oled_disp_str("run    step",11);         // Run 表示
	     
	     oled_disp_step_num(step_number);    // step番号の表示
	   }
	   else {				// プログラム運転停止モード (STOP)の場合
	     oled_disp_str("stop          ",14);        // Stop 表示
	   }
	   
	}
	else{					// ティーチングモードの場合
	 
	   oled_set_ddram_address(0x00);   // 表示開始位置 1行目先頭
	   oled_disp_str("ﾃｨｰﾁﾝｸﾞ     ",12);      // ﾃｨｰﾁﾝｸﾞ     表示
	   
	   oled_set_ddram_address(0x20);   // 表示開始位置 2行目先頭
	   if ( run_flg == 1 ) {	   // RUNの場合
	     oled_disp_str("run    step",11);      // Run 表示
	     
	     oled_disp_step_num(status_rcd_pt);    // 格納位置の表示
	   }
	   else {		           // 停止モード (STOP)の場合
	     oled_disp_str("stop          ",14);      // Stop 表示
	   }
	   
	}
	
}


//
// 未使用な割り込みに、登録される関数
//  「リンク・オプション→出力コード→ベクタ・テーブル空き領域のアドレス」で _Dummy とする。
//
//　( コンパイル後、RETI(61 FC))　
//
#pragma interrupt Dummy
void Dummy(void)
{
}




