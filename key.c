
#include   "iodefine.h"
#include   "misratypes.h"

#include "ctsu.h"

#include "seq_prog.h"
#include "key.h"

// スイッチ入力関係の構造体
//  
struct SW_info
{
	uint8_t status;		// 今回の タッチ有り(Low=0),非タッチ(High=1)状態 (10msec毎)

	uint8_t pre_status;	// 前回(10msec前)の タッチ有り(Low=0),非タッチ(High=1)状態 
	
	uint8_t low_cnt;	// タッチ有りの回数

	uint8_t one_push;	// 0:キー入力処理要求なし 1:キー入力処理要求(1度押し)　（キー入力処理後に0クリア）
	
	uint8_t edge_flg;	// 1: 立下り検出済み
	
	uint16_t threshold;	// ONと判定する CTSUのカウント値 

};


volatile struct  SW_info  Key_sw[CTSU_CH_NUM];	// スイッチ個数分の情報格納領域


uint8_t teaching_flg;		// teachingモード = 1, 通常モード = 0
uint8_t run_flg;        	// 実行中 = 1, 停止中=0 
			        // 通常モードで、実行中：プログラム運転動作
                                // teachigモードで、実行中: ティーチングデータの格納
uint8_t pre_run_flg;		// 100msec前のrun_flg
				
uint8_t rotate_clock_cnt;	// 時計回り(SW4)のON回数 100msec毎にカウント
uint8_t rotate_c_clock_cnt;	// 反時計回り(SW3)   :
uint8_t rotate_no_cnt;		// 時計回り、反時計回りが押されていない場合にカウント  





// タッチキーのLEDをON/OFF (SW3,SW4用)
//
// ctsu_sc[0] : TS06:  Key_sw[0]: 時計回り   SW4
// ctsu_sc[1] : TS08:  Key_sw[1]: 反時計回り SW3
//
//  P05 : SW4 LED : (TS06)
//  P07 : SW3 LED : (TS08)
//

void touch_led_on_off_sw3_sw4(void)
{
	 if ( Key_sw[0].one_push == 1 )  { // SW4 ON
	 	P0_bit.no5 = 1;		   // P05 = 1 を出力
	 }
	 else{
		 P0_bit.no5 = 0;	      // P05 = 0 を出力
	 }
	
	 if ( Key_sw[1].one_push == 1 )  { // SW3 ON
	 	P0_bit.no7 = 1;		      // P07 = 1 を出力
	 }
	 else{
		 P0_bit.no7 = 0;	      // P07 = 0 を出力
	 }
}



//
//   ステッピングモータへのパルス出力
//
// ctsu_sc[0] : TS06:  Key_sw[0]: 時計回り   SW4
// ctsu_sc[1] : TS08:  Key_sw[1]: 反時計回り SW3
//

void motor_pulse_out(void)
{
	if ( Key_sw[0].one_push == 1 ) {	// 時計回り(SW4) ON

	     TOE0L_bit.no4 = 1;     		// タイマ出力許可レジスタ  ch4(TO04) 出力許可　(ステッピングモータ パルス出力)
	 
	     P1_bit.no6 = 0;      		// P16 = 0を出力　(ステッピングモータ　方向制御)
		
	     rotate_clock_cnt++;	        //  時計回り(SW4)のON回数のインクリメント
	}

	else if ( Key_sw[1].one_push == 1 ) {	// 反時計回り(SW3) ON
	  
             TOE0L_bit.no4 = 1;     		// タイマ出力許可レジスタ  ch4(TO04) 出力許可　(ステッピングモータ パルス出力)
		
	      P1_bit.no6 = 1;       		// P16 = 1を出力　(ステッピングモータ　方向制御)
		
	      rotate_c_clock_cnt++;	      	//  反時計回り(SW3)のON回数のインクリメント
	}
	
	else {
	
            TOE0L_bit.no4 = 0;     		// タイマ出力許可レジスタ  ch4(TO04) 出力禁止　(ステッピングモータ パルス出力なし) 
          
	    TO0L = TO0L & 0xef;			// チャンネル4 タイマ出力 = 0
	 
	    P1_bit.no6 = 0;       		// P16 = 0を出力　(ステッピングモータ　方向制御)
	
	    rotate_no_cnt++;			//  時計回り、反時計回りが押されていない回数のインクリメント
	 
	}

}

//  　
//    モード変更　と　キー処理要求フラグのクリア
//
// ctsu_sc[2] : TS10:  key_se[2]: SW2: 開始スイッチ(RUN/STOP)                                                      
// ctsu_sc[3] : TS13:  key_sw[3[: SW1: モード切り換え (通常 /ティーチング) 
//

void mode_chg(void)
{
	uint8_t fl_err;
	
	
	pre_run_flg = run_flg;			// 変更前の状態を格納
	
	if ( Key_sw[3].one_push == 1 ) {	// SW1 ON
	     if ( teaching_flg == 1 ) {		// ティーチングモードの場合、
	     	teaching_flg = 0;		// 通常モードにする
		                                // ティーチングモードから通常モードへの変更時に、SW動作レコードをフラッシュへ保存
						
		copy_sw_record_to_flash();	// SWの動作レコード をデータフラッシュ用のバッファへコピー
		fl_err = write_flash_sw_record();	// フラッシュへ保存
		
	     }
	     else {				// 通常モードの場合
	     	teaching_flg = 1;		// ティーチングモードにする
		
	     }
	     					// プログラム運転中またはティーチング実行中に SW1を押された場合、
	    run_flg = 0;			// 停止モード (STOP)にする
	   
	     Key_sw[3].one_push = 0;		// キー入力処理要求(1度押し)のクリア
	}  
	
	else if ( Key_sw[2].one_push == 1 ) {	// SW2 ON
	     if ( teaching_flg == 0 ) {		// 現在、通常モードの場合
	     	   if ( run_flg == 0 ) {        // Stopの場合
		   	run_flg = 1;	        // プログラム運転モード (RUN)にする
			step_number = 0;	//  step番号のクリア
		   }
		   else {			// Runの場合
		   	run_flg = 0;	        // プログラム運転停止モード (STOP)にする
		   }
	      }
	      else {				// 現在、ティーチングモードの場合
	     	   if ( run_flg == 0 ) {        // stopの場合
		   	run_flg = 1;	  	// 実行モード にする
			clear_sw_status_record();	// SW レコードのクリア
		   }
		   else {			// ティーチング実行モードの場合
		   	run_flg = 0;            // 停止モード (STOP)にする
		   }
	     }
	     Key_sw[2].one_push = 0;		// キー入力処理要求(1度押し)のクリア
	}
}


//
//  各キーのタッチ有り判定値のセット
// 
void touch_key_on_val(void)
{
	uint8_t i;
	
	for( i = 0; i < CTSU_CH_NUM ; i++ ) {
	
		Key_sw[i].threshold = TOUCH_ON_VALUE;
	}
	
}

//
//   各キーのタッチ有り判定値と比較して、タッチの有無を判定する。
//
// ctsu_sc[0] : TS06:  Key_sw[0]: 時計回り   SW4
// ctsu_sc[1] : TS08:  Key_sw[1]: 反時計回り SW3
// ctsu_sc[2] : TS10:  key_se[2]: RUN/STOP 切り換え(通常モード時),動作データの保存 (ティーチングモード時) SW2                                                      
// ctsu_sc[3] : TS13:  key_sw[3[: モード切り換え (通常モード /ティーチングモード) SW1
//
void touch_key_status_check(void)
{
	uint8_t i;
	
	for( i = 0; i < CTSU_CH_NUM ; i++ ) {
	
	  if ( ctsu_sc[i] > Key_sw[i].threshold ) {  //  タッチ有り(ON)判定値より大きい場合、
	  
	  	Key_sw[i].status = 0;		// タッチ有り (Low=0)
	  }
	  else {
	  	Key_sw[i].status = 1;		// タッチ無し (High=1)
	  }
	  
	}
}


//
//  モード切り換え(SW1)、RUN/STOP,Save(SW2) 用 キー入力の確定処理  (10msec毎、タイマ割り込み内で実行)
//    立下り検出後、3回連続してタッチ有り判定ならば、SWオン確定フラグをセットする。
//
//  入力: id = 2 ～ 3
//
// ctsu_sc[2] : TS10:  key_se[2]: RUN/STOP 切り換え(通常モード時),動作データの保存 (ティーチングモード時) SW2                                                      
// ctsu_sc[3] : TS13:  key_sw[3[: モード切り換え (通常モード /ティーチングモード) SW1
//

void sw1_sw2_input_check( uint8_t id ) 
{
	
	 if ( Key_sw[id].status == 0 ) {     // 今回 タッチ状態
	
	     if ( Key_sw[id].pre_status == 1 ) {  // 前回 非タッチ状態 (立下がりの発生)
	     	   Key_sw[id].edge_flg = 1;	  // 立下り検出フラグのセット
	           Key_sw[id].low_cnt =  1;       // Lowカウント = 1 セット
	     }
	     else{				// 前回 タッチ状態
	      	  if ( Key_sw[id].edge_flg == 1 ) {   // 立下り検出フラグがセットされている場合
		     if ( Key_sw[id].low_cnt > 2 ) {   // 3回以上　タッチ検出の場合
			    Key_sw[id].one_push = 1;      // キー入力処理要求(1度押し)
			    Key_sw[id].edge_flg = 0;	  // 立下り検出フラグのクリア
		      }
		      else {
		  	Key_sw[id].low_cnt++;         // Lowカウントのインクリメント
		      }
		  }
	     }
	  }      // 今回タッチ状態
	  
	  else {	// 今回 非タッチ状態
	  
	  	Key_sw[id].low_cnt = 0;		//  Lowカウントのクリア
	  }
	  
	  Key_sw[id].pre_status = Key_sw[id].status;   // 現在の状態を、一つ前の状態へコピー
}




//
//  時計回り(SW4)、反時計回り(SW3) 用 キー入力の確定処理  (10msec毎、タイマ割り込み内で実行)
//    タッチ有り判定ならば、SWオン確定フラグをセットする。
//
//  入力: id = 0 ～ 1
//
// ctsu_sc[0] : TS06:  Key_sw[0]: 時計回り   SW4
// ctsu_sc[1] : TS08:  Key_sw[1]: 反時計回り SW3
//

void sw3_sw4_input_check( uint8_t id ) 
{
	if ( Key_sw[id].status == 0 ) {     // 今回 タッチ状態
	    
	      Key_sw[id].one_push = 1;	    // SWオン確定フラグをセット
	}
	else {				    // 今回 タッチ無し
	      
	       Key_sw[id].one_push = 0;	    // SWオン確定フラグをクリア
	}
}

