#include   "misratypes.h"
#include   "data_flash.h"

//
//    動作周波数とセルフ・プログラミング・モードの設定
//    (データフラッシュを書き込み/消去 可とする)
//
//
#pragma inline_asm  flash_clock_mode_set

void flash_clock_mode_set(void)
{
	
	MOV	!FSSET,#0x0F	; フラッシュ・メモリ・シーケンサの動作クロック = 16[MHz]
	MOV	!FLPMC,#0x22	; データフラッシュ 書き込み/消去 可)
	
}


//
//  指定したブロックのデータフラッシュのデータを消去
// 
// 入力: block 番号(0～1)
// 　　　block 0 : 0x09000～0x091FF  
//       　　　1 : 0x09200～0x093FF
//
// リターン値: エラー情報 (FSASTLの値)
//  (消去は、1block(512バイト)単位で行う。)
//

#pragma inline_asm  flash_erase_data

uint8_t flash_erase_data(uint8_t blk_num)
{
	CMP0 A		  ; A=0か A=0の場合 Z=1
	BNZ  $ERASE_BLK_1  ; A≠0(Z=0)の場合 branch
	
			   
	MOV !FLAPL,#0x00  ; block 0用	     
	MOV !FLAPH,#0x00	   

	MOV !FLSEDL,#0xFC   
	MOV !FLSEDH,#0x01
	
	BR	$ERASE_DF
	
ERASE_BLK_1:	          
	MOV !FLAPL,#0x00  ; block 1用 	     
	MOV !FLAPH,#0x02	   

	MOV !FLSEDL,#0xFC    
	MOV !FLSEDH,#0x03

ERASE_DF:
	MOV !FSSQ,#0x84	  ;データフラッシュ block Erase cmd

ERASE_WAIT:
	MOV A,!FSASTH     ;
	AND A,#0x40
	SKNZ		  ;FASASTH=0x40ならばSkip(=動作終了)
	BR  $ERASE_WAIT   ;動作終了待ちへ

	MOV !FSSQ,#0x00 ; cmd clear

ERASE_CMD_CLR_WAIT:
	MOV A,!FSASTH
	AND A,#0x40
	SKZ		  ;SQENDビットのクリア待ち
	BR  $ERASE_CMD_CLR_WAIT
	
	MOV A,!FSASTL     ; エラー情報格納　0ならばエラーなし

}

	
//  データ書き込み
// 
// 1) 入力: 書き込み先アドレス ( uint16_t ) 0x09000～0x093FFの下位16ビット指定
//       書き込みデータ (uint32_t)
//
//       AX   = wr_adrs (書き込みアドレス)
//       DE BC = wr_data (書き込みデータ)
//
//
//  22.6.1.1 フラッシュ・アドレス・ポインタ・レジスタH, L（FLAPH／FLAPL）
//  22.6.1.2 フラッシュ・エンド・アドレス指定レジスタH, L（FLSEDH／FLSEDL）
//     表 22－9 FLAPH/L, FLSEDH/L レジスタの設定方法
// FLAPH/L レジスタ：bit14-10＝ALL“0”
//                   bit9-0＝書込み対象アドレスのbit9-0
// FLSEDH/L レジスタ： bit14-0＝All“0”（未設定でも可）
//
//
// 2)リターン値: エラー情報 (FSASTLの値)
//  (書き込みは、32bit(4バイト)単位で行う。)
//
// レジスタ FSASTL の値:
// bit0: ERER: ブロック消去コマンドのエラー 0:エラー未発生, 1:エラー発生
// bit1: WRER: 書き込みコマンドのエラー 0:エラー未発生, 1:エラー発生
// bit4: SEQER:フラッシュ・メモリ・シーケンサのエラー   0:エラー未発生, 1:エラー発生
//
// 
/// 資料「RL78/G16　ユーザーズマニュアル ハードウェア編」(Rev.1.10 2023.08)
//        22.6.1.7 フラッシュ・メモリ・シーケンサ・ステータス・レジスタH, L（FSASTH／FSASTL）
//

#pragma inline_asm  flash_write_data

uint8_t flash_write_data( uint16_t wr_addrs, uint32_t wr_data)
{
			//書き込み先頭アドレス Highバイト側 (A)のマスク 
			//レジスタFLAPHとレジスタFLAPL：bit14-10＝ALL“0”, bit9-0＝書込み対象アドレスのbit9-0 とするため
	AND A,#0x03
	MOV !FLAPH,A

	MOV A,X
	MOV !FLAPL,A	;書き込み先頭アドレス Lowバイト側  
	
	MOV !FLSEDL,#0x00   
	MOV !FLSEDH,#0x00
	
	MOV A,C
	MOV !FLWLL,A	;書き込みデータ (b0～b7)   
	MOV A,B
	MOV !FLWLH,A	;書き込みデータ (b8～b15) 
	MOV A,E
	MOV !FLWHL,A	;書き込みデータ (b16～b23) 
	MOV A,D
	MOV !FLWHH,A    ;書き込みデータ (b24～b31) 
	
	MOV !FSSQ,#0x81	  ;データフラッシュ write cmd

FL_WR_WAIT:
	MOV A,!FSASTH     ; 0x41になる場合がある
	AND A,#0x40
	SKNZ		  ;FASASTH=0x40,0x41ならばSkip(=動作終了)
	BR  $FL_WR_WAIT   ;動作終了待ちへ

	MOV !FSSQ,#0x00 ; cmd clear

FL_WR_CMD_CLR_WAIT:
	MOV A,!FSASTH
	AND A,#0x40
	SKZ		  ;SQENDビットのクリア待ち
	BR  $FL_WR_CMD_CLR_WAIT
	
	MOV A,!FSASTL     ; エラー情報格納　0ならばエラーなし

}


//
// 　データ読み出し
//　読み出しアドレスから32bitのデータを読み出す。
//
// 入力: 読み出しアドレス ( uint16_t ) 0xF9000～0xF93FFの下位16ビット指定
//       ミラー領域から読み出す
//   AX  = rd_adrs (読み出しアドレス)
//
// リターン値: 読み出しデータ 32bit
//   BC AX = 32bitデータ
//  
//　・引数、戻り値については、「CC-RL コンパイラ ユーザズマニュアル」(R20UT3123JJ0113 Rev.1.13)　
//     9 関数呼び出し仕様を参照
//
#pragma inline_asm  flash_read_data

uint32_t flash_read_data( uint16_t rd_addrs )
{
	MOVW HL,AX	; HL=読み出しアドレス
	
	MOVW AX,[HL+2]  ; AX=読み出しデータ b16-b31
	MOVW BC,AX      ; BC=読み出しデータ b16-b31

	MOVW AX,[HL]	; AX=読み出しデータ b0-b15
}



//	
// iodefine.h をインクルードする場合は，アセンブリ記述関数をiodefine.h のインクルードより前に記述してください。
//  CC-RL コンパイラ ユーザーズマニュアル (R20UT3123JJ0113 Rev.1.13) page 382
//
#include   "iodefine.h"
