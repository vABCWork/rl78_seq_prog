
//  
//　スタートアップ処理  (インラインアセンブラ)
//
//   1) スタックポインタの設定
//   2) 使用するRAM領域 __RAM_ADDR_START(0xFF700)～__STACK_ADDR_START(0xFFE20)の 0クリア 
//       (.bss (初期値なし変数)領域、スタック領域を含む)含む)
//   3)  .data領域(ROM上 変数の初期値)を.dataR(RAM上 初期値付き変数)へコピー 
//        (saddr領域(0xFFE20～0xFFF1F)は未使用)
//   4) main()の呼び出し
//


#pragma inline_asm  start_up

void start_up(void)
{
	MOVW	SP,#LOWW(__STACK_ADDR_START)    ; スタックポインタの設定
	
	                                        ;__RAM_ADDR_START(0xFF700)～__STACK_ADDR_START(0xFFE20)の 0クリア
						;
	MOVW	AX,#LOWW(__STACK_ADDR_START - __RAM_ADDR_START) ; AX = クリア数
	MOVW	HL,#LOWW(__RAM_ADDR_START)                      ; HL = クリアするアドレス
	
	
CLR_LOOP:	
	MOV	[HL],#0x00	;指定アドレスの0クリア
	INCW	HL              ;アドレスインクリメント
	SUBW    AX,#0x0001      ;クリア数のデクリメント
	
	SKZ			; AX=0(Z=1)ならば、次の命令をスキップ, AX>0ならば、BRへ
	BR       $CLR_LOOP     ; 
	
				; .data領域(ROM上 変数の初期値)を.dataR(RAM上 初期値付き変数)へコピー 
	                        ;
	MOV	ES,#0x00	;  ES = 00

	MOVW	BC,#LOWW(SIZEOF(.data))    ; コピーするバイト数
	MOVW    AX,BC		; AX ← BC
	CMPW    AX,#0x0000	; AX=0ならば Z=1　

	SKNZ			; AX>0(Z=0)ならば、次の命令をスキップ, AX = 0ならば、BRへ
	BR       $START_MAIN    ; 

	
	MOVW	HL,#LOWW(STARTOF(.data))   ; HL = コピー元のアドレス 下位16ビット
	MOVW    DE,#LOWW(STARTOF(.dataR))  ; DE = コピー先のアドレス 下位16ビット
	
COPY_LOOP:		
	MOV	A,ES:[HL]	; ROM領域からのデータ読み出し　
	MOV	[DE],A		; RAM領域へ格納
	
	INCW	HL		; コピー元のアドレス インクリメント
	INCW	DE		; コピー先のアドレス インクリメント
		
	DECW	BC		; コピーするバイト数 デクリメント
	MOVW    AX,BC		; AX ← BC
	CMPW    AX,#0x0000	;　AX=0ならば Z=1　
	
	SKZ			; AX=0(Z=1)ならば、次の命令をスキップ, AX>0ならば、BRへ
	BR       $COPY_LOOP     ; 
	
	
START_MAIN:	
	CALL	!!_main		;メイン main() 呼び出し
	
	
EXIT:
	BR	$EXIT		; 無限ループ

}




