
#include "iodefine.h"
#include "misratypes.h"

#include "key.h"
#include "seq_prog.h"

#include "timer.h"




volatile uint8_t  flg_1_sec;

//
// 1�b���̃C���^�[�o���^�C�}
//�@ �^�C�}�E�`���l��00 �̃J�E���g����
//
#pragma interrupt INT_tm00(vect= INTTM00) 


void INT_tm00(void)
{
	flg_1_sec = 1;		// �t���O 1 sec�̃Z�b�g	

 	if ( teaching_flg == 1 ) {   // �e�B�[�`���O���[�h�̏ꍇ
	
	    status_rcd_pt = status_rcd_pt + 1;	  // �X�g�A�ʒu�̍X�V
 	}
	
 	else{
	    step_number = step_number + 1;       // step�ԍ��̃C���N�������g
 	}


}

// �^�C�}�A���C���j�b�g�̏�����
//  ���̓N���b�N�����ƕ�����ݒ�
//    CK00 = 500[KHz]  
//           16MHz/ 2^5 = 16000 KHz/32 = 500 [KHz]
//
//    CK01 = 1.95[KHz]
//            16MHz/2^13 = 16000 KHz/8192 = 1.953 [KHz]
//

void timer_array_ini(void)
{

	TAU0EN = 1;		// �^�C�}�A���C���j�b�g�ւ̃N���b�N���� , ���ӃC�l�[�u���E���W�X�^0�iPER0�j��bit0
	
	TPS0 = 0x00d5;		// CK01 = 1.95[KHz], CK00 =  500[KHz]
}
				



//
//  1�b�Ԋu�̃C���^�[�o���^�C�}
// �^�C�}�A���C���j�b�g �`�����l��0�̏�����
//�@�J�E���g�p�N���b�N CK01 = 1.95[KHz]�@
//
void timer_ch0_ini(void)
{
	
				
	TMR00 = 0x8000;		// ����N���b�N=CK01,�X�^�[�g�g���K:�\�t�g�E�F�A
				// ���샂�[�h:�C���^�[�o���^�C�}���[�h, (�J�E���g�J�n���Ƀ^�C�}���荞�݂𔭐����Ȃ�)
	
				// 1sec�̃C���^�[�o���^�C�}�p�̃^�C�}�f�[�^���W�X�^�l�i�o�͕��`�g�̎����́A4[msec])
	TDR00 = 1953 - 1;	// �^�C�}�p�N���b�N=1.953[KHz], 1 count = 1/1.953 [msec],   (1/1.953) * �^�C�}�f�[�^���W�X�^�l�@= 1000msec

	
	TMPR000 = 1;		// �^�C�}�E�`���l��00 ���荞�݃��x�� = 3 (���x��0�`3 :0���ō�)
	TMPR001 = 1;
	
	TMMK00 = 0;		// �^�C�}�E�`���l��00 ���荞�݋��� (���荞�݃}�X�N�N���A)
				
}




//
// �^�C�}�A���C���j�b�g �`�����l��4�̏�����
//  �J�E���g�p�N���b�N CK00 = 500[KHz]
// 
//  TO04 ����̏o�͕��`�g�̎���= �J�E���g�E�N���b�N�̎��� x ( TDR04 �̐ݒ�l + 1) x 2
//    4[msec] = 0.002[msec] x ( TDR04 �̐ݒ�l + 1) x 2
//    ( TDR04 �̐ݒ�l + 1 ) =  4 / 0.004

void timer_ch4_ini(void)
{
				
	TMR04 = 0x0000;		// ����N���b�N=CK00,�X�^�[�g�g���K:�\�t�g�E�F�A
				// ���샂�[�h:�C���^�[�o���^�C�}���[�h, ���`�g�o�� (�J�E���g�J�n���Ƀ^�C�}���荞�݂𔭐����Ȃ�)
	
				// 2msec�̃C���^�[�o���^�C�}�p�̃^�C�}�f�[�^���W�X�^�l�i�o�͕��`�g�̎����́A4[msec])
	TDR04 = 1000 - 1;	// �^�C�}�p�N���b�N=500[KHz], 1 count = 0.002[msec],   0.002 * �^�C�}�f�[�^���W�X�^�l�@= 2msec

	
	TOM0L = TOM0L & 0xef;	// �`�����l��4 : �}�X�^�E�`���l���o�̓��[�h�i�^�C�}���荞�ݗv���M���iINTTMmn�j�ɂ��g�O���o�͂��s���j
	
	TO0L = TO0L & 0xef;	// �`�����l��4 :�^�C�}�o�͂̃o�b�t�@�E���W�X�^ (�����o�̓��x��) = Low
	
	TOE0L_bit.no4 = 0;     // �^�C�}�o�͋����W�X�^  ch4(TO04) �o�͋֎~�@
	
	p17_to04();		// P17 = TO04(ch4�^�C�}�o��) �Ƃ��鏈��
	
	
	TS0L_bit.no4 = 1;	// �`�����l��4�J�E���g�J�n
				
}




// 
//   P17 = TO04(ch4�^�C�}�o��) �Ƃ��鏈��
//
//   PIOR (����I/O ���_�C���N�V�����E���W�X�^):  PIOR1 bit3 = 0,  PIOR1 bit4 = 1
//   POM (�|�[�g�o�̓��[�h�E���W�X�^):  POM1 bit7 = 0 (�ʏ�o�̓��[�h)
//   PMC (�|�[�g�E���[�h�E�R���g���[���E���W�X�^):�@don't care
//   PM (�|�[�g�E���[�h�E���W�X�^) : PM1 bit7= 0 ( �o�̓��[�h�i�o�̓o�b�t�@�E�I���j)
//   P (�|�[�g�E���W�X�^) : P1 bit7 = 0 (0���o��)
//   TSSEL(�^�b�`�[�q�@�\�I�����W�X�^) : TSSEL0 bit3(TSSEL03) = 0 (�^�b�`�[�q�@�\�ȊO�i���p�@�\�j�Ƃ��Ďg�p)
//
//   ����: �uRL78/G16 ���[�U�[�Y�}�j���A�� �n�[�h�E�F�A�ҁv( R01UH0980JJ0110 Rev.1.10 )
//        �u4.5.3 �g�p����|�[�g�@�\����ь��p�@�\�̃��W�X�^�ݒ��v���
//

void p17_to04(void)
{

	PIOR1 = PIOR1 & 0xf7;   // PIOR1 bit3 = 0
	PIOR1 = PIOR1 | 0x10;   // PIOR1 bit4 = 1
	
	POM1_bit.no7 = 0;	// P17�́A�ʏ�o�̓��[�h
	
	PM1_bit.no7 = 0;	// P17�́A�o�̓��[�h�i�o�̓o�b�t�@�E�I���j
	P1_bit.no7 = 0;		// 0���o��
	
	TSSEL0_bit.no2 = 0;	// �^�b�`�[�q�@�\�ȊO�i���p�@�\�j�Ƃ��Ďg�p (P17�̓^�b�`�L�[��TS02�ƌ��p)
	
}


//
//  P16���o�͂Ƃ��鏈��
//�@P16:�@�X�e�b�s���O���[�^�̉�]��������p

void p16_out_mode(void)
{
	POM1_bit.no6 = 0;	// P16�́A�ʏ�o�̓��[�h
	
	PM1_bit.no6 = 0;	// P16�́A�o�̓��[�h
	
	P1_bit.no6 = 0;		// 0���o��
}