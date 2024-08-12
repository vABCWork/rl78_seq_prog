
#include   "iodefine.h"
#include   "misratypes.h"

#include "interval_timer_12bit.h"
#include "ctsu.h"
#include "key.h"

#include "data_flash.h"
#include "seq_prog.h"

#include  "debug_port.h"


uint8_t step_number;	// 実行中のステップ番号 ( 0～59 )

uint8_t status_rcd_pt;          	// 保存位置を示す、index
uint8_t sw_status_record[REC_MAX];	// SWの動作レコード (1秒毎) 

uint32_t flash_sw_record[REC_MAX_FLASH];	// データフラッシュへの書込み、読出し用


uint8_t flash_blk0_err;
uint8_t flash_blk1_err;
uint8_t flash_wr_err;

//
//  プログラムステップ動作
//  パルス出力とLED点灯
//
//   sw_status_record[]の内容を、1秒毎に実行
//　データ:  動作内容
//　 0x00:   SW3=OFF, SW4=OFF
//   0x01:   SW3=OFF, SW4=ON
//   0x02:   SW3=ON,  SW4=OFF
//
//
//  P05 : SW4 LED : (TS06)
//  P07 : SW3 LED : (TS08)
//

void  exec_prog_step(void)
{
	uint8_t pd;
	
	pd = sw_status_record[step_number];
	
	if ( pd == 0x01 ) {	    // 時計回り(SW4 ON) 
	     
	     TOE0L_bit.no4 = 1;     // タイマ出力許可レジスタ  ch4(TO04) 出力許可　(ステッピングモータ パルス出力)
	     
	     P1_bit.no6 = 0;        // P16 = 0を出力　(ステッピングモータ　方向制御)
	     
	     P0_bit.no5 = 1;	    // P05 = 1 を出力
	     
	     
	}
	
	else if ( pd == 0x02 ){	      // 反時計回り(SW3 ON)
	
	        TOE0L_bit.no4 = 1;    // タイマ出力許可レジスタ  ch4(TO04) 出力許可　(ステッピングモータ パルス出力)
		
		P1_bit.no6 = 1;       // P16 = 1を出力　(ステッピングモータ　方向制御)
		
		P0_bit.no7 = 1;		// P07 = 1 を出力
		
	}
	
	else {			// 両方OFF(SW3:OFF, SW4:OFF)
	   
	  TOE0L_bit.no4 = 0;     // タイマ出力許可レジスタ  ch4(TO04) 出力禁止　(ステッピングモータ パルス出力なし) 
          
	  TO0L = TO0L & 0xef;	// チャンネル4 タイマ出力 = 0
	  
	  P1_bit.no6 = 0;       // P16 = 0を出力　(ステッピングモータ　方向制御)
	  
	   P0_bit.no5 = 0;	      // P05 = 0 を出力
	   
	   P0_bit.no7 = 0;	      // P07 = 0 を出力
	
	}
	
	if ( step_number > REC_MAX - 1 ) {  // 最終ステップ実行済みならば、
		 step_number = 0;       	 // step番号のクリア
		 run_flg = 0;   	         // Stopモード にする 
	 }
	
}



//
//  動作レコードへSWのON/OFF状態を記録する。( 1秒毎 )   
//  10[msec]毎の SW3のON回数、SW4のON回数、OFF回数の内、
//  最大なものを1秒間の動作として保存する。
//   SW4 ON : 0x01
//   SW3 ON : 0x02
//    OFF   : 0x00
//
//  rotate_clock_cnt    : 時計回り(SW4)のON回数 10msec毎にカウント
//  rotate_c_clock_cnt  : 反時計回り(SW3)   :
//  rotate_no_cnt	: 時計回り、反時計回りが押されていない場合にカウント  
//

void touch_record_store(void)
{
	uint8_t  rcd_val;
	
	if (  rotate_c_clock_cnt < rotate_clock_cnt ) {   //  反時計回り(SW3) <　時計回り(SW4)
	   if ( rotate_no_cnt < rotate_clock_cnt ) {      //  両方OFF < 時計回り(SW4)
	         rcd_val = 0x01;
	   }
	   else {			   // 両方OFF > 時計回り(SW4)
	         rcd_val = 0x00;
	   }
	
	}
	else {				  //  反時計回り(SW3) >= 時計回り(SW4)
	  if ( rotate_no_cnt < rotate_c_clock_cnt ) {   // 両方OFF < 反時計回り(SW3)
	         rcd_val = 0x02;
	   }
	   else {			   // 両方OFF > 反時計回り(SW3)
	         rcd_val = 0x00;
	   }
	}
	
	sw_status_record[status_rcd_pt] = rcd_val; // レコードの格納
	
	

	if ( status_rcd_pt > REC_MAX - 1  ) {    // レコードの上限に達した場合
		status_rcd_pt = 0;	// ストア位置のクリア
		run_flg = 0;   	        // Stopモード にする 
	}
	
	rotate_clock_cnt = 0;		// SW状態カウンタのクリア
	rotate_c_clock_cnt = 0;
	rotate_no_cnt = 0;
	
}

//
//  動作レコードと格納位置の初期化
//
void clear_sw_status_record(void)
{
	uint8_t  i;
	
	for ( i = 0 ; i < REC_MAX; i++ ) {
		
	        sw_status_record[i] = 0x00;
	
	}
	
	status_rcd_pt = 0;	// ストア位置のクリア
}


//  
//  SWの動作レコード をデータフラッシュ用のバッファへコピー
//   sw_status_record[]  -->   flash_sw_record[]
//       0x00                    0x03020100
//       0x01
//       0x02
//       0x03
//
void copy_sw_record_to_flash(void)
{
	uint8_t   i;
	uint32_t  fd;
	
	for ( i = 0 ; i < REC_MAX_FLASH; i++ ) {
	  
	  fd = sw_status_record[i*4 + 3];
	  fd = fd << 8;
	  
	  fd = fd | sw_status_record[i*4 + 2];
	  fd = fd << 8;  
	  
	  fd = fd | sw_status_record[i*4 + 1];
	  fd = fd << 8;
	  
	  fd = 	fd | sw_status_record[i*4];
	  
	  flash_sw_record[i] = fd;	
	
	}
}



//  
//  データフラッシュ用のバッファから、SWの動作レコードへ
// flash_sw_record[] ---> sw_status_record[] 
//    0x03020100    　　　 0x00                   
//      　　　　　　　　　 0x01
//       　　　　　　　　　0x02
//      　　　　　　　　　 0x03
//
void copy_sw_record_from_flash(void)
{
	uint8_t   i;
	uint32_t  fd;
	
	for ( i = 0 ; i < REC_MAX_FLASH; i++ ) {
		
	  fd = 	flash_sw_record[i];
	    
	  sw_status_record[i*4] = fd;
	 
	  sw_status_record[i*4 + 1] = fd >> 8;
	 
	  sw_status_record[i*4 + 2] = fd >> 16;
	 
	  sw_status_record[i*4 + 3] = fd >> 24;
	
	}
}


//
// SWの動作レコードをデータフラッシュへ書き込み
// データフラッシュ書き込み単位: 32bit
//                 消去単位:  512 byte
//
// flash_sw_record[i]    データフラッシュアドレス          
//        0                0x 9000                           
//  　　　1                0x 9004 
//        2                0x 9008
//        :                    :
//       14                0x 9038
//
//  リターン値: 0x00 = 正常終了
//
//  処理時間: 6.2[msec] (512byteの消去 + 4byteデータを15回書き込み)
//
uint8_t write_flash_sw_record(void)
{
	uint8_t i;
	
//	DEBUG_PORT_OUT = 1;	// P13 = 1 (デバック用)
	
	__DI();			// 割り込み禁止 （割り込み許可フラグ（IE）をクリア（0））
	
	flash_blk0_err = flash_erase_data(0);	// データフラッシュ Block=0の消去 5.3[msec]
	
	if ( flash_blk0_err != 0 ) {		
		return flash_blk0_err;
	}
	
	
	for ( i = 0 ; i < REC_MAX_FLASH ; i++ ) {
		
	   flash_wr_err = flash_write_data( 0x9000 + i*4 , flash_sw_record[i] ); // データ書き込み 
	
	   if ( flash_wr_err != 0 ) {		
		return flash_wr_err;
	   }
	}
	
	__EI();			// 割り込み許可
	
	
//   DEBUG_PORT_OUT = 0;	// P13 = 0 (デバック用)
	
        return 0x00;
	
}



//
// データフラッシュのSWの動作レコードを読み出し
//
// flash_sw_record[i]    データフラッシュアドレス          
//        0                0x 9000                           
//  　　　1                0x 9004 
//        2                0x 9008
//        :                    :
//       14                0x 9038
//

void read_flash_sw_record(void)
{
	uint8_t i;
	
	for ( i = 0 ; i < REC_MAX_FLASH ; i++ ) {
		
	   flash_sw_record[i]  = flash_read_data( 0x9000 + i*4 );	// データ読み出し 
	
	}
}

