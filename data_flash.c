#include   "misratypes.h"
#include   "data_flash.h"

//
//    ������g���ƃZ���t�E�v���O���~���O�E���[�h�̐ݒ�
//    (�f�[�^�t���b�V������������/���� �Ƃ���)
//
//
#pragma inline_asm  flash_clock_mode_set

void flash_clock_mode_set(void)
{
	
	MOV	!FSSET,#0x0F	; �t���b�V���E�������E�V�[�P���T�̓���N���b�N = 16[MHz]
	MOV	!FLPMC,#0x22	; �f�[�^�t���b�V�� ��������/���� ��)
	
}


//
//  �w�肵���u���b�N�̃f�[�^�t���b�V���̃f�[�^������
// 
// ����: block �ԍ�(0�`1)
// �@�@�@block 0 : 0x09000�`0x091FF  
//       �@�@�@1 : 0x09200�`0x093FF
//
// ���^�[���l: �G���[��� (FSASTL�̒l)
//  (�����́A1block(512�o�C�g)�P�ʂōs���B)
//

#pragma inline_asm  flash_erase_data

uint8_t flash_erase_data(uint8_t blk_num)
{
	CMP0 A		  ; A=0�� A=0�̏ꍇ Z=1
	BNZ  $ERASE_BLK_1  ; A��0(Z=0)�̏ꍇ branch
	
			   
	MOV !FLAPL,#0x00  ; block 0�p	     
	MOV !FLAPH,#0x00	   

	MOV !FLSEDL,#0xFC   
	MOV !FLSEDH,#0x01
	
	BR	$ERASE_DF
	
ERASE_BLK_1:	          
	MOV !FLAPL,#0x00  ; block 1�p 	     
	MOV !FLAPH,#0x02	   

	MOV !FLSEDL,#0xFC    
	MOV !FLSEDH,#0x03

ERASE_DF:
	MOV !FSSQ,#0x84	  ;�f�[�^�t���b�V�� block Erase cmd

ERASE_WAIT:
	MOV A,!FSASTH     ;
	AND A,#0x40
	SKNZ		  ;FASASTH=0x40�Ȃ��Skip(=����I��)
	BR  $ERASE_WAIT   ;����I���҂���

	MOV !FSSQ,#0x00 ; cmd clear

ERASE_CMD_CLR_WAIT:
	MOV A,!FSASTH
	AND A,#0x40
	SKZ		  ;SQEND�r�b�g�̃N���A�҂�
	BR  $ERASE_CMD_CLR_WAIT
	
	MOV A,!FSASTL     ; �G���[���i�[�@0�Ȃ�΃G���[�Ȃ�

}

	
//  �f�[�^��������
// 
// 1) ����: �������ݐ�A�h���X ( uint16_t ) 0x09000�`0x093FF�̉���16�r�b�g�w��
//       �������݃f�[�^ (uint32_t)
//
//       AX   = wr_adrs (�������݃A�h���X)
//       DE BC = wr_data (�������݃f�[�^)
//
//
//  22.6.1.1 �t���b�V���E�A�h���X�E�|�C���^�E���W�X�^H, L�iFLAPH�^FLAPL�j
//  22.6.1.2 �t���b�V���E�G���h�E�A�h���X�w�背�W�X�^H, L�iFLSEDH�^FLSEDL�j
//     �\ 22�|9 FLAPH/L, FLSEDH/L ���W�X�^�̐ݒ���@
// FLAPH/L ���W�X�^�Fbit14-10��ALL�g0�h
//                   bit9-0�������ݑΏۃA�h���X��bit9-0
// FLSEDH/L ���W�X�^�F bit14-0��All�g0�h�i���ݒ�ł��j
//
//
// 2)���^�[���l: �G���[��� (FSASTL�̒l)
//  (�������݂́A32bit(4�o�C�g)�P�ʂōs���B)
//
// ���W�X�^ FSASTL �̒l:
// bit0: ERER: �u���b�N�����R�}���h�̃G���[ 0:�G���[������, 1:�G���[����
// bit1: WRER: �������݃R�}���h�̃G���[ 0:�G���[������, 1:�G���[����
// bit4: SEQER:�t���b�V���E�������E�V�[�P���T�̃G���[   0:�G���[������, 1:�G���[����
//
// 
/// �����uRL78/G16�@���[�U�[�Y�}�j���A�� �n�[�h�E�F�A�ҁv(Rev.1.10 2023.08)
//        22.6.1.7 �t���b�V���E�������E�V�[�P���T�E�X�e�[�^�X�E���W�X�^H, L�iFSASTH�^FSASTL�j
//

#pragma inline_asm  flash_write_data

uint8_t flash_write_data( uint16_t wr_addrs, uint32_t wr_data)
{
			//�������ݐ擪�A�h���X High�o�C�g�� (A)�̃}�X�N 
			//���W�X�^FLAPH�ƃ��W�X�^FLAPL�Fbit14-10��ALL�g0�h, bit9-0�������ݑΏۃA�h���X��bit9-0 �Ƃ��邽��
	AND A,#0x03
	MOV !FLAPH,A

	MOV A,X
	MOV !FLAPL,A	;�������ݐ擪�A�h���X Low�o�C�g��  
	
	MOV !FLSEDL,#0x00   
	MOV !FLSEDH,#0x00
	
	MOV A,C
	MOV !FLWLL,A	;�������݃f�[�^ (b0�`b7)   
	MOV A,B
	MOV !FLWLH,A	;�������݃f�[�^ (b8�`b15) 
	MOV A,E
	MOV !FLWHL,A	;�������݃f�[�^ (b16�`b23) 
	MOV A,D
	MOV !FLWHH,A    ;�������݃f�[�^ (b24�`b31) 
	
	MOV !FSSQ,#0x81	  ;�f�[�^�t���b�V�� write cmd

FL_WR_WAIT:
	MOV A,!FSASTH     ; 0x41�ɂȂ�ꍇ������
	AND A,#0x40
	SKNZ		  ;FASASTH=0x40,0x41�Ȃ��Skip(=����I��)
	BR  $FL_WR_WAIT   ;����I���҂���

	MOV !FSSQ,#0x00 ; cmd clear

FL_WR_CMD_CLR_WAIT:
	MOV A,!FSASTH
	AND A,#0x40
	SKZ		  ;SQEND�r�b�g�̃N���A�҂�
	BR  $FL_WR_CMD_CLR_WAIT
	
	MOV A,!FSASTL     ; �G���[���i�[�@0�Ȃ�΃G���[�Ȃ�

}


//
// �@�f�[�^�ǂݏo��
//�@�ǂݏo���A�h���X����32bit�̃f�[�^��ǂݏo���B
//
// ����: �ǂݏo���A�h���X ( uint16_t ) 0xF9000�`0xF93FF�̉���16�r�b�g�w��
//       �~���[�̈悩��ǂݏo��
//   AX  = rd_adrs (�ǂݏo���A�h���X)
//
// ���^�[���l: �ǂݏo���f�[�^ 32bit
//   BC AX = 32bit�f�[�^
//  
//�@�E�����A�߂�l�ɂ��ẮA�uCC-RL �R���p�C�� ���[�U�Y�}�j���A���v(R20UT3123JJ0113 Rev.1.13)�@
//     9 �֐��Ăяo���d�l���Q��
//
#pragma inline_asm  flash_read_data

uint32_t flash_read_data( uint16_t rd_addrs )
{
	MOVW HL,AX	; HL=�ǂݏo���A�h���X
	
	MOVW AX,[HL+2]  ; AX=�ǂݏo���f�[�^ b16-b31
	MOVW BC,AX      ; BC=�ǂݏo���f�[�^ b16-b31

	MOVW AX,[HL]	; AX=�ǂݏo���f�[�^ b0-b15
}



//	
// iodefine.h ���C���N���[�h����ꍇ�́C�A�Z���u���L�q�֐���iodefine.h �̃C���N���[�h���O�ɋL�q���Ă��������B
//  CC-RL �R���p�C�� ���[�U�[�Y�}�j���A�� (R20UT3123JJ0113 Rev.1.13) page 382
//
#include   "iodefine.h"
