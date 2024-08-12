
#include   "misratypes.h"


#pragma inline_asm  delay_msec

//
//  N[msec] 待つ関数   (インラインアセンブラ)
// 高速オンチップオシレータ = 16 [MHz] 
//  1 clock = 1 / 16[MHz] = 0.0625 [usec]
//
// 入力: AX = n_msec
//
// 　(RETはコンパイル時に挿入される。)
//
void delay_msec( uint16_t n_msec )
{
	CMPW AX,#0x0000 ; AX - 0x0000
	SKNZ		; AX≠0 ならば 次をSKIP
	BR    $DELAY_N_MS_EXIT

DLAY_N_MS_LOOP_1:	
	MOVW BC,AX	;  BC← AX,  BC = n_msec
	
	MOVW AX,#0x0A66	; 1 msecループ用 (0x0A66 = 2662)
	
DLAY_N_MS_LOOP_2:
	
	NOP		
	
	SUBW	AX,#0x0001         ; AX = AX - 1 , 1 clock
	
	BNZ	$DLAY_N_MS_LOOP_2  ; AX≠0　ならば Loop (条件成立時:4 clock, 条件不成立時:2 clock) 
	
	MOVW    AX,BC		   ; AX← BC,  AX = n_msec
	
	SUBW	AX,#0x0001         ; AX = AX - 1 , 1 clock
	
	BNZ	$DLAY_N_MS_LOOP_1  ; AX≠0　ならば Loop (条件成立時:4 clock, 条件不成立時:2 clock) 
	
DELAY_N_MS_EXIT:	
	
}




#pragma inline_asm  delay_1_msec

//
//  1[msec] 待つ関数    (インラインアセンブラ)
// 高速オンチップオシレータ = 16 [MHz] 
//  1 clock = 1 / 16[MHz] = 0.0625 [usec]
//
//　(RETはコンパイル時に挿入される。)
//
void delay_1_msec(void)
{
	MOVW AX,#0x0A66		; 0x0A66 = 2662
	
DLAY_1_MS_LOOP:
	
	NOP			; 1clock
	
	SUBW	AX,#0x0001		; AX = AX - 1 , 1 clock
	
	BNZ	$DLAY_1_MS_LOOP  ; AX≠0　ならば Loop (条件成立時:4 clock, 条件不成立時:2 clock) 
	
	
}







#pragma inline_asm  delay_40_usec

//
//  30[usec] 待つ関数    (インラインアセンブラ)
// 高速オンチップオシレータ = 16 [MHz] 
//  1 clock = 1 / 16[MHz] = 0.0625 [usec]
//
//　(RETはコンパイル時に挿入される。)
//
void delay_40_usec(void)
{
	MOVW AX,#0x0068		; 0x0068 = 104
	
DLAY_40_US_LOOP:
	
	NOP			; 1clock
	
	SUBW	AX,#0x0001	; AX = AX - 1 , 1 clock
	
	BNZ	$DLAY_40_US_LOOP  ; AX≠0　ならば Loop (条件成立時:4 clock, 条件不成立時:2 clock) 
	
	
}





#pragma inline_asm  delay_5_usec

//
//  5[usec] 待つ関数    (インラインアセンブラ)
// 高速オンチップオシレータ = 16 [MHz] 
//  1 clock = 1 / 16[MHz] = 0.0625 [usec]
//
//　(RETはコンパイル時に挿入される。)
//
void delay_5_usec(void)
{
	MOVW AX,#0x000d		; 0x000d = 13
	
DLAY_5_US_LOOP:
	
	NOP			; 1clock
	
	SUBW	AX,#0x0001	; AX = AX - 1 , 1 clock
	
	BNZ	$DLAY_5_US_LOOP  ; AX≠0　ならば Loop (条件成立時:4 clock, 条件不成立時:2 clock) 
	
	
}




