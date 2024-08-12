
#include   "iodefine.h"
#include   "misratypes.h"

#include    "iic_sd.h"
#include  "delay.h"


uint8_t iic_slave_adrs;  // IIC スレーブアドレス  00: 7bitアドレス( 例:100 0000 = 0x40 )

volatile uint8_t iic_sd_data[32];    // 送信データ
volatile uint8_t iic_sd_pt;	    // 送信データ位置


volatile uint8_t  iic_sd_num;	    // 送信データ数(スレーブアドレスを含む)
volatile uint8_t  iic_rcv_num;      // 受信データ数
volatile uint8_t  iic_com_over_fg;  // 1:STOPコンディションの検出時



// IICA 割り込み処理プログラムの先頭アドレスを、INTIICA0(0x0002C)へ登録
#pragma interrupt INT_iica(vect=INTIICA0)

// IICA 割り込み処理 
//  割り込み発生タイミング
// 1) WTIM0(IICCTL00レジスタ bit3) = 1 　
//    9 クロック目の立ち下がりで割り込み要求発生。 (ACK/NACK)
// 2) SPIE0(IICCTL00レジスタ bit4) = 0
//    ストップ・コンディション検出による割り込み要求発生　禁止
//
// 参考: RL78/G16 ユーザーズマニュアル ハードウェア編
//   14.5.17 I2C 割り込み要求（INTIICA0）の発生タイミング
//    ( WTIM0＝1 設定 )
//

void INT_iica(void)
{
  				
      if ( ACKD0 == 0 ) {	// ACKを検出していない場合
	  SPT0 = 1;	// ストップ・コンディションを生成 
	  IICAMK0 = 1;	// IICA割り込み禁止
	
	  iic_com_over_fg = 1;	// 通信終了フラグのセット
          
	  return;
      }
      
      
      if ( iic_sd_pt < iic_sd_num ) {   // 送信データがある場合
      
	  IICA0 = iic_sd_data[iic_sd_pt];  // データの送信
	
	  iic_sd_pt++;			 // 送信位置の更新
      }
      else {			// 全て送信済み
	  SPT0 = 1;	// ストップ・コンディションを生成 
	  IICAMK0 = 1;	// IICA割り込み禁止
	  
	  iic_com_over_fg = 1;	// 通信終了フラグのセット
      }
      
}




//  OLED キャラクタディスプレイ(SO1602A)の初期化
//  I2C接続 16文字 x 2行　
//
void oled_so1602a_ini(void)
{
	
	oled_clear_display();	// 表示のクリア
	
	delay_msec(2);		// 2[msec]待つ
	
	oled_disp_on();		// 表示 ON
	
	delay_40_usec();	// 40[usec]待つ
	
}

//
// 表示のクリア
// 空白の書き込み
void oled_clear_display(void)
{

	iic_sd_data[0] = ( iic_slave_adrs << 1 ) ;  // スレーブアドレスへ書き込み
	iic_sd_data[1] = 0x00;          // コマンド 
	iic_sd_data[2] = 0x01;		//  Clear display
	iic_master_send (3);		// マスタ送信

	while( iic_com_over_fg != 1 ) {    // 送信完了待ち
	} 
	
}

//
//  表示　ON
//  Display ON, cursor OFF, blink OFF
//
void oled_disp_on(void)
{
	
	iic_sd_data[0] = ( iic_slave_adrs << 1 ) ;  // スレーブアドレスへ書き込み
	iic_sd_data[1] = 0x00;          // コマンド 
	
	iic_sd_data[2] = 0x0c;		//  Display ON, cursor OFF, blink OFF
	
	iic_master_send (3);		// マスタ送信

	while( iic_com_over_fg != 1 ) {    // 送信完了待ち
	} 
}



//  表示位置の指定
//  上の行 0x00～0x0f
//  下の行 0x20～0x2f
//
void oled_set_ddram_address(uint8_t ac_val)
{
	iic_sd_data[0] = ( iic_slave_adrs << 1 ) ;  // スレーブアドレスへ書き込み
	iic_sd_data[1] = 0x00;                  // Instruction  Write operation
	iic_sd_data[2] = (0x80 | ac_val);	// Set DDRAM address instruction
	
	iic_master_send (3);	// マスタ送信
	
	while( iic_com_over_fg != 1 ) {    // 送信完了待ち

	} 
	
	delay_40_usec();		// 40[usec]待つ (SO1602A コマンド処理時間)
}



//
// 文字列の表示
//
// 入力:   pt_str       : 表示文字列が格納されている先頭アドレス
//          num         :　表示文字数 (1～8)
//
void oled_disp_str( char *pt_str, uint8_t num)
{
	uint8_t i;
	
	iic_sd_data[0] = ( iic_slave_adrs << 1 ) ;  // スレーブアドレスへ書き込み
	iic_sd_data[1] = 0x40;		// Data Write operation
	
	for ( i = 0 ; i < num ; i++ ) {
	    iic_sd_data[2 + i] = *pt_str;    // 表示する文字コードを格納
	    
	     pt_str++;
	}
	
	iic_master_send (2 + num);	// マスタ送信
	
	while( iic_com_over_fg != 1 ) {    // 送信完了待ち
	
	} 
	
	delay_40_usec();		// 40[usec]待つ　 (SO1602A コマンド処理時間)
	
}



//  プログラム実行時のstep番号、ティーチングデータの格納位置を表示　(0～59)
//   入力: step番号または格納位置　(0～59)
//  
void oled_disp_step_num( uint8_t step )
{
	uint8_t keta_2;
	uint8_t keta_1;
	
	keta_2 = step / 10;		// 10の桁の値
	keta_1 = step - (keta_2 * 10);  // 1の桁の値
	
		
	oled_set_ddram_address(0x2c);   // 表示開始位置 2行目 col=7
	
	
	iic_sd_data[0] = ( iic_slave_adrs << 1 ) ;  // スレーブアドレスへ書き込み
	iic_sd_data[1] = 0x40;		// Data Write operation
	
	iic_sd_data[2] = keta_2 + 0x30;       // 10の桁の表示する文字コードを格納
	iic_sd_data[3] = keta_1 + 0x30;       // 1の桁の表示する文字コードを格納
	
	iic_master_send (4);	// マスタ送信
	
	while( iic_com_over_fg != 1 ) {    // 送信完了待ち
	
	} 
	
	delay_40_usec();		// 40[usec]待つ　 (SO1602A コマンド処理時間)
	
}









// IIC マスタ送信
//   スレーブへ　送信バッファ　iic_sd_data[]のデータを sd_numバイト送信する。
// 入力: sd_num  送信バイト数　
// 
//   
void iic_master_send ( uint8_t sd_num)
{
	iic_sd_num = sd_num;		// 送信データ数
	
	iic_sd_start();			// IICA 送信開始
}



//  IIC 送信開始
// MSTS0 : IICA ステータス・レジスタ0（IICS0）の bit7
//
void iic_sd_start(void)
{
	
	iic_sd_pt = 0;			// 送信データ位置　クリア
	iic_com_over_fg = 0;	        // 通信完了フラグのクリア
	
	
	while ( IICBSY0 == 1 ) {	// バス解放待ち , IICBSY0 : IICA フラグ・レジスタ0（IICF0）の bit6 
	}

	IICAMK0 = 0;		// IICA割り込みマスクのクリア
	
	STT0 = 1;		// スタートコンディションの発行  (マスタ送信の開始)  IICCTL00のbit1
	
//	スタート・コンディションが生成されたかまたは拒絶されたかは、STCF0（IICF0 レジスタのビット7）で確認でき
//ます。STT0＝1 としてからSTCF0 がセット（1）されるまで5 クロックの時間がかかりますので、ソフトウェアにより
//この時間を確保してください p650
	
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	
					// スタートコンディション発行できない場合、ループ
	while( STCF0 == 1 ) {		// スタート・コンディション発行できず、STT0 フラグ・クリア。STCF0（IICF0 レジスタのビット7）
	}
	
	while( STD0 == 0 ) {		// スタート・コンディションを検出待ち, IICA ステータス・レジスタ0（IICS0）の bit1
	}
	
        IICA0 = iic_sd_data[iic_sd_pt];  // スレーブアドレスの送信
	
        iic_sd_pt++;			 // 送信位置の更新
					 // スレーブからのACK応答で、割り込みに入る 
	
}



//
//  IICAの初期設定
//
//  使用端子: SCLA0 = P60
//            SDAA0 = P61
//
void iic_ini(void)
{
	IICA0EN = 1;		// シリアル・インターフェイス IICA へのクロック供給　(周辺イネーブル・レジスタ0（PER0）のbit4=1)
	IICE0 = 0;		// 動作停止  (IICCTL00レジスタ bit7 )の設定
	IICAMK0 = 1;		// IICA割り込み禁止 (MK1Lレジスタの bit4)
	IICAIF0 = 0;		// IICA割り込み要求フラグのクリア (IF1Lレジスタの bit4)
	
	IICAPR00 = 1;		// IICA 割り込みレベル = 3 (レベル0～3 :0が最高)
	IICAPR10 = 1;
	
	PIOR3 = PIOR3 & 0xf3;   // 使用端子の設定 PIOR33=0,PIOR32=0 (SCLA0 = P60, SDAA0 = P61)
	
				
	SMC0 = 1;		// ファースト・モードで動作（最大転送レート：400 kbps）, IICA コントロール・レジスタ01（IICCTL01）のbit3
	
	DFC0 = 1;		// デジタル・フィルタ・オン , IICA コントロール・レジスタ01（IICCTL01）のbit3
	
	SVA0 = 32;		// 自局アドレス（マスタ通信では使用しない)
	
	IICWL0 = 21;		// 転送クロック=400[KHz]設定 　Lowレベル幅
	IICWH0 = 20;		// Highレベル幅
	
				// IICA フラグ・レジスタ0（IICF0）
	STCEN0 = 1;		// ストップ・コンディションを検出せずに、スタート・コンディションを生成
	
	IICRSV0 = 1;		// 通信予約禁止 IICA フラグ・レジスタ0（IICF0）の bit0
	
	SPIE0 = 0;		// ストップ・コンディション検出で割り込み発生しない　IICCTL00レジスタのbit4
	
	WTIM0 = 1;		// クロック・ストレッチおよび割り込み要求発生の制御 
				// 9 クロック目の立ち下がりで割り込み要求発生。IICCTL00レジスタのbit3
				// マスタの場合：9 クロック出力後、クロック出力をロウ・レベルにしたままクロック・ストレッチ
				
	ACKE0 = 1;		// アクノリッジを許可。9 クロック期間中にSDAA0 ラインをロウ・レベルにする。IICCTL00レジスタのbit2
	
	IICAMK0 = 0;		// IICA割り込み許可 (MK1Lレジスタの bit4)
	
	IICE0 = 1;		// 動作許可　IICCTL00レジスタのbit7
	
	LREL0 = 1;		// 待機状態 IICCTL00レジスタのbit6
	
	PM6_bit.no0 = 0;	// P60:出力モード
	PM6_bit.no1 = 0;        // P61:出力モード
	
	                         // P60 = 1, P61 = 1にするとコマンドが発行されない。
	P6_bit.no0 = 0;		// P60: 出力ラッチ = 0
	P6_bit.no1 = 0;         // P61: 出力ラッチ = 0
	
}


