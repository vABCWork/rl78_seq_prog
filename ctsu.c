
#include "iodefine.h"
#include "misratypes.h"
#include "ctsu.h"
#include "delay.h"


						   // CTSUWR 割り込みで設定される値(タッチセンサ毎の、オフセットやゲイン等を設定)
volatile uint16_t ctsu_ssc[CTSU_CH_NUM];	   // CTSU 高域ノイズ低減スペクトラム拡散制御レジスタ (CTSUSSC)
volatile uint16_t ctsu_so0[CTSU_CH_NUM];	   // センサオフセットレジスタ0 (CTSUSO0) (計測回数,電流オフセット)
volatile uint16_t ctsu_so1[CTSU_CH_NUM];           // センサオフセットレジスタ1 (CTSUSO1) (レファランス電流,ベースクロック,ゲイン)


volatile uint16_t ctsu_sc[CTSU_CH_NUM];	        // CTSU センサカウンタ（CTSUSC）
volatile uint16_t ctsu_rc[CTSU_CH_NUM];         // CTSU リファレンスカウンタ（CTSURC）

volatile uint16_t ctsu_errs;			// CTSU エラーステータスレジスタ（CTSUERRS）
volatile uint8_t ctsu_st;			// CTSU ステータスレジスタ（CTSUST)
			
volatile uint8_t  ctsu_data_index;              // ctsu_sc[]等の格納位置	





//
// CTSU チャネル毎の設定レジスタ書き込み 完了割り込み(INTCTSUWR)
//
#pragma interrupt INT_ctsuwr(vect=INTCTSUWR) 

void INT_ctsuwr(void)
{
		  
	CTSUSSC = ctsu_ssc[ctsu_data_index];	//高域ノイズ低減スペクトラム拡散制御			
	
	CTSUSO0 = ctsu_so0[ctsu_data_index];      //CTSU センサオフセットレジスタ0
	
	CTSUSO1 = ctsu_so1[ctsu_data_index];     // CTSU センサオフセットレジスタ1
		
}



//
// CTSU 計測データ転送完了割り込み(INTCTSURD)
//
//  各タッチセンサの読出しカウント値を格納
//  TSの若い順から計測される。
//
#pragma interrupt INT_ctsurd(vect=INTCTSURD) 

void INT_ctsurd(void)
{
	ctsu_sc[ctsu_data_index] = CTSUSC;	//センサカウンタ
			
	ctsu_rc[ctsu_data_index] = CTSURC; 	//リファレンスカウンタ
			
	ctsu_data_index++;		// +1 格納場所
	
}


//
// CTSU計測終了割り込み(INTCTSUFN)
//  全タッチセンサの読み出し完了
//  
//  ctsu_sc[0] : TS06 : SW4
//  ctsu_sc[1] : TS08 : SW3
//  ctsu_sc[2] : TS10 : SW2
//  ctst_sc[3] : TS13 : SW1
//
//  4ch分の計測時間(レジスタ書き込み割り込み(INTCTSUWR)から終了割り込み(INTCTSUFN)まで） 
//                = 1.22 [msec]
//   条件: ベースクロック周波数(センサドライブパルス)= 1[MHz] , 計測回数 = 1 回
//
#pragma interrupt INT_ctsufn(vect=INTCTSUFN) 

void INT_ctsufn(void)
{
	
	
	ctsu_st = CTSUST;		// CTSU ステータスレジスタ（CTSUST) bit5(CTSUSOVF)=1 :オーバフローあり
	ctsu_errs = CTSUERRS;		// CTSU エラーステータスレジスタ（CTSUERRS）bit15(CTSUICOMP)=1 :TSCAP 電圧異常
		
	ctsu_data_index = 0;		// センサ読出しカウント値の格納場所をクリア

}



//
//  自己容量マルチスキャンモードと割り込みの設定
//
void ctsu_self_set(void)
{
	ctsu_ini();			// 静電容量式タッチセンサ(CTSU)の初期化
	
	set_ctsuwr_data();		// CTSUWR割り込み時の書き込みデータ設定
	
	ctsu_self_reg_set_multi();	//  CTSUレジスタ設定 ( 4ch計測 TS06,S08,TS10,TS13)
	
	CTSUWRPR0 = 0;			// CTSUWR 割り込みレベル = 0　(レベル0～3:0が最高)
	CTSUWRPR1 = 0;
	CTSUWRMK = 0;			// CTSUWR割り込み許可
	
	CTSURDPR0 = 0;			// CTSURD 割り込みレベル = 0　(レベル0～3:0が最高)
	CTSURDPR1 = 0;
	CTSURDMK = 0;			// CTSURD割り込み許可
	
	CTSUFNPR0 = 0;			// CTSUFN 割り込みレベル = 0　(レベル0～3:0が最高)
	CTSUFNPR1 = 0;
	CTSUFNMK = 0;			// CTSUFN割り込み許可
	
	
}


//  CTSUWR 割り込み(INTCTSUWR)で設定する値
//
// 1)ctsu_ssc[] : CTSUSSC
//  0x0000:   4 [MHz] ≦ ベースクロック周波数
//  0x0100:   2[MHz] ≦ ベースクロック周波数 < 4[MHz]
//  0x0300:   1[MHz] ≦ ベースクロック周波数 < 1.33[MHz]               
//  0x0700:  0.5[MHz]≦ ベースクロック周波数 < 0.57[MHz]
//
// 2) ctsu_ctsuso0[]: CTSUSO0の設定値
//   0000 00 00 0000 0000 (0x00): 計測回数=1, 電流オフセット量= 0
//   0000 01 00 0000 0000 (0x40): 計測回数=2, 電流オフセット量= 0
//
// 3) ctsu_ctsuso1[]: CTSUSO1の設定値 ゲイン(100[%])とベースクロック周波数(センサドライブパルス)の設定
//    0x0000            : CTSU動作クロックの2分周 ( = 16/2 = 8[NHz] )
//    0x0100            : CTSU動作クロックの4分周 ( = 16/4 = 4[NHz] )
//    0x0300            : CTSU動作クロックの8分周 ( = 16/8 = 2[MHz] )
//    0x0700            : CTSU動作クロックの16分周 ( = 16/16 = 1[MHz] )
//    0x0f00            : CTSU動作クロックの32分周( = 16/32 = 0.5[NHz] )
//  (CTSU動作クロック = 16[MHz]
//
// 注: ベースクロック周波数変更時には、ctsu_ssc[]の値を変更が必要
//
void set_ctsuwr_data(void)
{
	uint8_t i;
		

	 for ( i = 0;  i < CTSU_CH_NUM; i++ ) {	 
		//ctsu_ssc[i] = 0x0700;          //　CTSU 高域ノイズ低減スペクトラム拡散制御 ( ベースクロック= 0.5[MHz]の場合 ) 
		ctsu_ssc[i] = 0x0300;            //　CTSU 高域ノイズ低減スペクトラム拡散制御 ( ベースクロック= 1[MHz]の場合 ) 
		//ctsu_ssc[i] = 0x0100;          //　CTSU 高域ノイズ低減スペクトラム拡散制御 ( ベースクロック= 2[MHz]の場合 ) 
		//ctsu_ssc[i] = 0x0000;          //　CTSU 高域ノイズ低減スペクトラム拡散制御 ( ベースクロック ≧ 4[MHz]の場合 
	 }
					
	for ( i = 0;  i < CTSU_CH_NUM; i++ ) {	
		 ctsu_so0[i] =  0x00;		//　計測回数=1回, 電流オフセット量= 0
	}
					       
	for ( i = 0;  i < CTSU_CH_NUM; i++ ) {
		//ctsu_so1[i] =  0x0f00;	// CTSUベースクロック 0.5[MHz] ( 16/32 [MHz])
		
		 ctsu_so1[i] =  0x0700;		// CTSUベースクロック 1[MHz] ( 16/16 [MHz])
		
		//ctsu_so1[i] =  0x0300;	// CTSUベースクロック 2[MHz] ( 16/8 [MHz])
		//ctsu_so1[i] =  0x0100;	// CTSUベースクロック 4[MHz] ( 16/4 [MHz])
		//ctsu_so1[i] =  0x0000;	// CTSUベースクロック 8[MHz] ( 16/2 [MHz])
	}
	
}



//
// 　CTSUレジスタ設定 (自己容量マルチスキャンモード)
//
// CTSU計測対象: 
//    P06: TS06 : SW4
//    P23: TS08 : SW3
//    P21: TS10 : SW2
//    P41: TS13 : SW1
//
// CTSU 計測動作開始トリガ:
//    外部トリガ（12 ビット・インターバル・タイマのインターバル割り込み信号）
//
void ctsu_self_reg_set_multi(void)
{
	CTSUCR1_bit.no6 = 1;    // 自己容量マルチスキャンモード  (CTSUMD0 = 1) 
	CTSUCR1_bit.no7 = 0;    // (CTSUMD1 = 0)
	
	CTSUSDPRS_bit.no6 = 0;   // 高域ノイズ低減機能ON (CTSUSOFF = 0)
	
	CTSUSDPRS_bit.no4 = 0;   // CTSU基本周期、基本パルス数 62パルス(推奨値) 
	CTSUSDPRS_bit.no5 = 1;   //  (CTSUPRMODE1 = 1) 
	
	CTSUSDPRS_bit.no0 = 1;   // CTSU計測時間、計測パルス数調整ビット 3(推奨値)
	CTSUSDPRS_bit.no1 = 1;   // (CTSUPRRATIO1 = 1)
	CTSUSDPRS_bit.no2 = 0;   // (CTSUPRRATIO2 = 0)
	CTSUSDPRS_bit.no3 = 0;   // (CTSUPRRATIO3 = 0)
	
	CTSUSST = 0x10;		 // CTSU センサ安定待ち時間 (固定値)
	
	CTSUCHAC0_bit.no6 = 1;   // TS06 計測対象
	
	CTSUCHAC1_bit.no0 = 1;   // TS08 計測対象
	CTSUCHAC1_bit.no2 = 1;   // TS10 計測対象
	CTSUCHAC1_bit.no5 = 1;   // TS13 計測対象
	
	CTSUDCLKC = 0x30;	// CTSU高域ノイズ低減機能 有効
	
	CTSUCHTRC0 = 0;		// 自己容量マルチスキャンモードでは、0
	CTSUCHTRC1 = 0;	
	
	CTSUCR0_bit.no1 = 1;	// CTSU計測動作開始トリガ (12 ビット・インターバル・タイマのインターバル割り込み信号) (CTSUCAP = 1)
	CTSUCR0_bit.no2 = 0;    // サスペンド機能無効 ( CTSUSNZ = 0 )
	
}


//
//   静電容量式タッチセンサ(CTSU)の初期化
//
void ctsu_ini(void)
{

	   tscap_clear();		// タッチセンサ用 TSCAPの放電
	   
	   touch_port_set();		// タッチセンサのポート設定
	   
	   CTSUEN = 1;			// CTSUの入力クロックを供給（CTSUで使用するSFRへのリード／ライト可）　(周辺イネーブル・レジスタ1（PER1）のbit1=1)
	   
	   CTSUCR1_bit.no3 = 0;		// CTSU 電源能力設定 (通常出力)
	   
	   ctsu_clock();		// CTSUの動作クロック設定 (カウントソース) 
	   
	   ctsu_power_on();		// CTSU電源オン
	   
					// TSCAP端子に外付けしたLPF容量へのチャージ安定待ち
	   delay_msec(1);		//	1msec待つ   (CTSU電源安定時間 )
	  
}

// CTSU電源オン
// CTSUの電源供給とTSCAP端子へLPF容量を接続
// ( CTSUCR1レジスタのCTSUPONビット＝1, CTSUCR1レジスタのCTSUCSWビット＝1を同時に書き込み)
void ctsu_power_on(void)
{
	CTSUCR1 = CTSUCR1 | 0x03;	
}


// 
//  CTSU 動作クロック選択　: CTSUCR1レジスタのCTSUCLK1(bit5)とCTSUCLK0(bit4) 
//   CTSUCLK1 CTSUCLK0 
//      0        0      : fclk/1
//  
//  fclk = 16[MHz] (高速オンチップオシレータ)
//
void ctsu_clock(void)
{
				// CTSU動作クロック 16[MHz]( = fclk/1 = 16/1{MHz] )
	CTSUCR1_bit.no4 = 0;	// CTSUCLK0 = 0
	CTSUCR1_bit.no5 = 0;    // CTSUCLK1 = 0

}

//  タッチセンサ用 TSCAP(P02)の放電  
//  P02を出力ポートとして Low を一定期間出力し、すでに充電されているLPF 容量を放電	
// P02 : 出力ポート
//   PMC (ポート・モード・コントロール・レジスタ): PMC0 bit2 = 0 (デジタル入出力（アナログ入力以外の兼用機能）)
//   PM (ポート・モード・レジスタ) : PM0 bit2= 0 ( 出力モード（出力バッファ・オン）)
//   P (ポート・レジスタ) : P0 bit2 = 0 (0を出力)
//
void tscap_clear(void)
{
	PMC0_bit.no2 = 0;	// P02は、デジタル入出力（アナログ入力以外の兼用機能）
	PM0_bit.no2 = 0;	// ( 出力モード（出力バッファ・オン）)
	P0_bit.no2 = 0;		// 0を出力
	
	delay_msec(10);		//　10msec待つ  
}


// 　タッチセンサのポート設定
//
//    P06: TS06 : SW4
//    P23: TS08 : SW3
//    P21: TS10 : SW2
//    P41: TS13 : SW1
//
//    P02:TSCAP : タッチキー用ローパスフィルタ端子(0.01uF=10nF)
//
//
// 1) P06 = TS06
//   PM (ポート・モード・レジスタ) : PM0 bit6= 1 ( 入力モード（出力バッファ・オフ）) 
//   P (ポート・レジスタ) : P0 bit6 = 0 (ロー・レベルを入力)
//   TSSEL(タッチ端子機能選択レジスタ) : TSSEL0 bit6 = 1 (タッチ端子機能として使用)
//
// 2) P23 = TS08
//   PM (ポート・モード・レジスタ) : PM2 bit3= 1 ( 入力モード（出力バッファ・オフ）) 
//   P (ポート・レジスタ) : P2 bit3 = 0 (ロー・レベルを入力)
//   TSSEL(タッチ端子機能選択レジスタ) : TSSEL1 bit0 = 1 (タッチ端子機能として使用)
//
// 3) P21 = TS10
//   PM (ポート・モード・レジスタ) : PM2 bit1= 1 ( 入力モード（出力バッファ・オフ）) 
//   P (ポート・レジスタ) : P2 bit1 = 0 (ロー・レベルを入力)
//   TSSEL(タッチ端子機能選択レジスタ) : TSSEL1 bit2 = 1 (タッチ端子機能として使用)
//
// 4) P41 = TS13
//   PM (ポート・モード・レジスタ) : PM4 bit1= 1 ( 入力モード（出力バッファ・オフ）) 
//   P (ポート・レジスタ) : P4 bit1 = 0 (ロー・レベルを入力)
//   TSSEL(タッチ端子機能選択レジスタ) : TSSEL1 bit5 = 1 (タッチ端子機能として使用)
//
// 5) TSCAP= P02 
//   PM (ポート・モード・レジスタ) : PM0 bit2= 1 ( 入力モード（出力バッファ・オフ）) 
//   (タッチ端子機能を使用する場合（任意の TSSELxx ビット＝1 を設定した時）、P02/TSCAP 端子は自動的にTSCAP 機能になります。)
//
//
//   資料: 「RL78/G16 ユーザーズマニュアル ハードウェア編」( R01UH0980JJ0110 Rev.1.10 )
//        「4.5.3 使用するポート機能および兼用機能のレジスタ設定例」より
//

void touch_port_set(void)
{
				// TS06
	PM0_bit.no6 = 1;	// P06は、入力モード（出力バッファ・オフ）
	P0_bit.no6 = 0;		// ロー・レベルを入力
	TSSEL0_bit.no6 = 1;	// タッチ端子機能として使用
	
				// TS08
	PM2_bit.no3 = 1;	// P23は、入力モード（出力バッファ・オフ）
	P2_bit.no3 = 0;		// ロー・レベルを入力
	TSSEL1_bit.no0 = 1;	// タッチ端子機能として使用
	
				// TS10
	PM2_bit.no1 = 1;	// P21は、入力モード（出力バッファ・オフ）
	P2_bit.no1 = 0;		// ロー・レベルを入力
	TSSEL1_bit.no2 = 1;	// タッチ端子機能として使用
	
				// TS13
	PM4_bit.no1 = 1;	// P41は、入力モード（出力バッファ・オフ）
	P4_bit.no1 = 0;		// ロー・レベルを入力
	TSSEL1_bit.no5 = 1;	// タッチ端子機能として使用
	
	
	PM0_bit.no2 = 1;	// P02は、入力モード（出力バッファ・オフ）
}


