
#include "iodefine.h"
#include "misratypes.h"
#include "ctsu.h"
#include "delay.h"


						   // CTSUWR ���荞�݂Őݒ肳���l(�^�b�`�Z���T���́A�I�t�Z�b�g��Q�C������ݒ�)
volatile uint16_t ctsu_ssc[CTSU_CH_NUM];	   // CTSU ����m�C�Y�ጸ�X�y�N�g�����g�U���䃌�W�X�^ (CTSUSSC)
volatile uint16_t ctsu_so0[CTSU_CH_NUM];	   // �Z���T�I�t�Z�b�g���W�X�^0 (CTSUSO0) (�v����,�d���I�t�Z�b�g)
volatile uint16_t ctsu_so1[CTSU_CH_NUM];           // �Z���T�I�t�Z�b�g���W�X�^1 (CTSUSO1) (���t�@�����X�d��,�x�[�X�N���b�N,�Q�C��)


volatile uint16_t ctsu_sc[CTSU_CH_NUM];	        // CTSU �Z���T�J�E���^�iCTSUSC�j
volatile uint16_t ctsu_rc[CTSU_CH_NUM];         // CTSU ���t�@�����X�J�E���^�iCTSURC�j

volatile uint16_t ctsu_errs;			// CTSU �G���[�X�e�[�^�X���W�X�^�iCTSUERRS�j
volatile uint8_t ctsu_st;			// CTSU �X�e�[�^�X���W�X�^�iCTSUST)
			
volatile uint8_t  ctsu_data_index;              // ctsu_sc[]���̊i�[�ʒu	





//
// CTSU �`���l�����̐ݒ背�W�X�^�������� �������荞��(INTCTSUWR)
//
#pragma interrupt INT_ctsuwr(vect=INTCTSUWR) 

void INT_ctsuwr(void)
{
		  
	CTSUSSC = ctsu_ssc[ctsu_data_index];	//����m�C�Y�ጸ�X�y�N�g�����g�U����			
	
	CTSUSO0 = ctsu_so0[ctsu_data_index];      //CTSU �Z���T�I�t�Z�b�g���W�X�^0
	
	CTSUSO1 = ctsu_so1[ctsu_data_index];     // CTSU �Z���T�I�t�Z�b�g���W�X�^1
		
}



//
// CTSU �v���f�[�^�]���������荞��(INTCTSURD)
//
//  �e�^�b�`�Z���T�̓Ǐo���J�E���g�l���i�[
//  TS�̎Ⴂ������v�������B
//
#pragma interrupt INT_ctsurd(vect=INTCTSURD) 

void INT_ctsurd(void)
{
	ctsu_sc[ctsu_data_index] = CTSUSC;	//�Z���T�J�E���^
			
	ctsu_rc[ctsu_data_index] = CTSURC; 	//���t�@�����X�J�E���^
			
	ctsu_data_index++;		// +1 �i�[�ꏊ
	
}


//
// CTSU�v���I�����荞��(INTCTSUFN)
//  �S�^�b�`�Z���T�̓ǂݏo������
//  
//  ctsu_sc[0] : TS06 : SW4
//  ctsu_sc[1] : TS08 : SW3
//  ctsu_sc[2] : TS10 : SW2
//  ctst_sc[3] : TS13 : SW1
//
//  4ch���̌v������(���W�X�^�������݊��荞��(INTCTSUWR)����I�����荞��(INTCTSUFN)�܂Łj 
//                = 1.22 [msec]
//   ����: �x�[�X�N���b�N���g��(�Z���T�h���C�u�p���X)= 1[MHz] , �v���� = 1 ��
//
#pragma interrupt INT_ctsufn(vect=INTCTSUFN) 

void INT_ctsufn(void)
{
	
	
	ctsu_st = CTSUST;		// CTSU �X�e�[�^�X���W�X�^�iCTSUST) bit5(CTSUSOVF)=1 :�I�[�o�t���[����
	ctsu_errs = CTSUERRS;		// CTSU �G���[�X�e�[�^�X���W�X�^�iCTSUERRS�jbit15(CTSUICOMP)=1 :TSCAP �d���ُ�
		
	ctsu_data_index = 0;		// �Z���T�Ǐo���J�E���g�l�̊i�[�ꏊ���N���A

}



//
//  ���ȗe�ʃ}���`�X�L�������[�h�Ɗ��荞�݂̐ݒ�
//
void ctsu_self_set(void)
{
	ctsu_ini();			// �Ód�e�ʎ��^�b�`�Z���T(CTSU)�̏�����
	
	set_ctsuwr_data();		// CTSUWR���荞�ݎ��̏������݃f�[�^�ݒ�
	
	ctsu_self_reg_set_multi();	//  CTSU���W�X�^�ݒ� ( 4ch�v�� TS06,S08,TS10,TS13)
	
	CTSUWRPR0 = 0;			// CTSUWR ���荞�݃��x�� = 0�@(���x��0�`3:0���ō�)
	CTSUWRPR1 = 0;
	CTSUWRMK = 0;			// CTSUWR���荞�݋���
	
	CTSURDPR0 = 0;			// CTSURD ���荞�݃��x�� = 0�@(���x��0�`3:0���ō�)
	CTSURDPR1 = 0;
	CTSURDMK = 0;			// CTSURD���荞�݋���
	
	CTSUFNPR0 = 0;			// CTSUFN ���荞�݃��x�� = 0�@(���x��0�`3:0���ō�)
	CTSUFNPR1 = 0;
	CTSUFNMK = 0;			// CTSUFN���荞�݋���
	
	
}


//  CTSUWR ���荞��(INTCTSUWR)�Őݒ肷��l
//
// 1)ctsu_ssc[] : CTSUSSC
//  0x0000:   4 [MHz] �� �x�[�X�N���b�N���g��
//  0x0100:   2[MHz] �� �x�[�X�N���b�N���g�� < 4[MHz]
//  0x0300:   1[MHz] �� �x�[�X�N���b�N���g�� < 1.33[MHz]               
//  0x0700:  0.5[MHz]�� �x�[�X�N���b�N���g�� < 0.57[MHz]
//
// 2) ctsu_ctsuso0[]: CTSUSO0�̐ݒ�l
//   0000 00 00 0000 0000 (0x00): �v����=1, �d���I�t�Z�b�g��= 0
//   0000 01 00 0000 0000 (0x40): �v����=2, �d���I�t�Z�b�g��= 0
//
// 3) ctsu_ctsuso1[]: CTSUSO1�̐ݒ�l �Q�C��(100[%])�ƃx�[�X�N���b�N���g��(�Z���T�h���C�u�p���X)�̐ݒ�
//    0x0000            : CTSU����N���b�N��2���� ( = 16/2 = 8[NHz] )
//    0x0100            : CTSU����N���b�N��4���� ( = 16/4 = 4[NHz] )
//    0x0300            : CTSU����N���b�N��8���� ( = 16/8 = 2[MHz] )
//    0x0700            : CTSU����N���b�N��16���� ( = 16/16 = 1[MHz] )
//    0x0f00            : CTSU����N���b�N��32����( = 16/32 = 0.5[NHz] )
//  (CTSU����N���b�N = 16[MHz]
//
// ��: �x�[�X�N���b�N���g���ύX���ɂ́Actsu_ssc[]�̒l��ύX���K�v
//
void set_ctsuwr_data(void)
{
	uint8_t i;
		

	 for ( i = 0;  i < CTSU_CH_NUM; i++ ) {	 
		//ctsu_ssc[i] = 0x0700;          //�@CTSU ����m�C�Y�ጸ�X�y�N�g�����g�U���� ( �x�[�X�N���b�N= 0.5[MHz]�̏ꍇ ) 
		ctsu_ssc[i] = 0x0300;            //�@CTSU ����m�C�Y�ጸ�X�y�N�g�����g�U���� ( �x�[�X�N���b�N= 1[MHz]�̏ꍇ ) 
		//ctsu_ssc[i] = 0x0100;          //�@CTSU ����m�C�Y�ጸ�X�y�N�g�����g�U���� ( �x�[�X�N���b�N= 2[MHz]�̏ꍇ ) 
		//ctsu_ssc[i] = 0x0000;          //�@CTSU ����m�C�Y�ጸ�X�y�N�g�����g�U���� ( �x�[�X�N���b�N �� 4[MHz]�̏ꍇ 
	 }
					
	for ( i = 0;  i < CTSU_CH_NUM; i++ ) {	
		 ctsu_so0[i] =  0x00;		//�@�v����=1��, �d���I�t�Z�b�g��= 0
	}
					       
	for ( i = 0;  i < CTSU_CH_NUM; i++ ) {
		//ctsu_so1[i] =  0x0f00;	// CTSU�x�[�X�N���b�N 0.5[MHz] ( 16/32 [MHz])
		
		 ctsu_so1[i] =  0x0700;		// CTSU�x�[�X�N���b�N 1[MHz] ( 16/16 [MHz])
		
		//ctsu_so1[i] =  0x0300;	// CTSU�x�[�X�N���b�N 2[MHz] ( 16/8 [MHz])
		//ctsu_so1[i] =  0x0100;	// CTSU�x�[�X�N���b�N 4[MHz] ( 16/4 [MHz])
		//ctsu_so1[i] =  0x0000;	// CTSU�x�[�X�N���b�N 8[MHz] ( 16/2 [MHz])
	}
	
}



//
// �@CTSU���W�X�^�ݒ� (���ȗe�ʃ}���`�X�L�������[�h)
//
// CTSU�v���Ώ�: 
//    P06: TS06 : SW4
//    P23: TS08 : SW3
//    P21: TS10 : SW2
//    P41: TS13 : SW1
//
// CTSU �v������J�n�g���K:
//    �O���g���K�i12 �r�b�g�E�C���^�[�o���E�^�C�}�̃C���^�[�o�����荞�ݐM���j
//
void ctsu_self_reg_set_multi(void)
{
	CTSUCR1_bit.no6 = 1;    // ���ȗe�ʃ}���`�X�L�������[�h  (CTSUMD0 = 1) 
	CTSUCR1_bit.no7 = 0;    // (CTSUMD1 = 0)
	
	CTSUSDPRS_bit.no6 = 0;   // ����m�C�Y�ጸ�@�\ON (CTSUSOFF = 0)
	
	CTSUSDPRS_bit.no4 = 0;   // CTSU��{�����A��{�p���X�� 62�p���X(�����l) 
	CTSUSDPRS_bit.no5 = 1;   //  (CTSUPRMODE1 = 1) 
	
	CTSUSDPRS_bit.no0 = 1;   // CTSU�v�����ԁA�v���p���X�������r�b�g 3(�����l)
	CTSUSDPRS_bit.no1 = 1;   // (CTSUPRRATIO1 = 1)
	CTSUSDPRS_bit.no2 = 0;   // (CTSUPRRATIO2 = 0)
	CTSUSDPRS_bit.no3 = 0;   // (CTSUPRRATIO3 = 0)
	
	CTSUSST = 0x10;		 // CTSU �Z���T����҂����� (�Œ�l)
	
	CTSUCHAC0_bit.no6 = 1;   // TS06 �v���Ώ�
	
	CTSUCHAC1_bit.no0 = 1;   // TS08 �v���Ώ�
	CTSUCHAC1_bit.no2 = 1;   // TS10 �v���Ώ�
	CTSUCHAC1_bit.no5 = 1;   // TS13 �v���Ώ�
	
	CTSUDCLKC = 0x30;	// CTSU����m�C�Y�ጸ�@�\ �L��
	
	CTSUCHTRC0 = 0;		// ���ȗe�ʃ}���`�X�L�������[�h�ł́A0
	CTSUCHTRC1 = 0;	
	
	CTSUCR0_bit.no1 = 1;	// CTSU�v������J�n�g���K (12 �r�b�g�E�C���^�[�o���E�^�C�}�̃C���^�[�o�����荞�ݐM��) (CTSUCAP = 1)
	CTSUCR0_bit.no2 = 0;    // �T�X�y���h�@�\���� ( CTSUSNZ = 0 )
	
}


//
//   �Ód�e�ʎ��^�b�`�Z���T(CTSU)�̏�����
//
void ctsu_ini(void)
{

	   tscap_clear();		// �^�b�`�Z���T�p TSCAP�̕��d
	   
	   touch_port_set();		// �^�b�`�Z���T�̃|�[�g�ݒ�
	   
	   CTSUEN = 1;			// CTSU�̓��̓N���b�N�������iCTSU�Ŏg�p����SFR�ւ̃��[�h�^���C�g�j�@(���ӃC�l�[�u���E���W�X�^1�iPER1�j��bit1=1)
	   
	   CTSUCR1_bit.no3 = 0;		// CTSU �d���\�͐ݒ� (�ʏ�o��)
	   
	   ctsu_clock();		// CTSU�̓���N���b�N�ݒ� (�J�E���g�\�[�X) 
	   
	   ctsu_power_on();		// CTSU�d���I��
	   
					// TSCAP�[�q�ɊO�t������LPF�e�ʂւ̃`���[�W����҂�
	   delay_msec(1);		//	1msec�҂�   (CTSU�d�����莞�� )
	  
}

// CTSU�d���I��
// CTSU�̓d��������TSCAP�[�q��LPF�e�ʂ�ڑ�
// ( CTSUCR1���W�X�^��CTSUPON�r�b�g��1, CTSUCR1���W�X�^��CTSUCSW�r�b�g��1�𓯎��ɏ�������)
void ctsu_power_on(void)
{
	CTSUCR1 = CTSUCR1 | 0x03;	
}


// 
//  CTSU ����N���b�N�I���@: CTSUCR1���W�X�^��CTSUCLK1(bit5)��CTSUCLK0(bit4) 
//   CTSUCLK1 CTSUCLK0 
//      0        0      : fclk/1
//  
//  fclk = 16[MHz] (�����I���`�b�v�I�V���[�^)
//
void ctsu_clock(void)
{
				// CTSU����N���b�N 16[MHz]( = fclk/1 = 16/1{MHz] )
	CTSUCR1_bit.no4 = 0;	// CTSUCLK0 = 0
	CTSUCR1_bit.no5 = 0;    // CTSUCLK1 = 0

}

//  �^�b�`�Z���T�p TSCAP(P02)�̕��d  
//  P02���o�̓|�[�g�Ƃ��� Low �������ԏo�͂��A���łɏ[�d����Ă���LPF �e�ʂ���d	
// P02 : �o�̓|�[�g
//   PMC (�|�[�g�E���[�h�E�R���g���[���E���W�X�^): PMC0 bit2 = 0 (�f�W�^�����o�́i�A�i���O���͈ȊO�̌��p�@�\�j)
//   PM (�|�[�g�E���[�h�E���W�X�^) : PM0 bit2= 0 ( �o�̓��[�h�i�o�̓o�b�t�@�E�I���j)
//   P (�|�[�g�E���W�X�^) : P0 bit2 = 0 (0���o��)
//
void tscap_clear(void)
{
	PMC0_bit.no2 = 0;	// P02�́A�f�W�^�����o�́i�A�i���O���͈ȊO�̌��p�@�\�j
	PM0_bit.no2 = 0;	// ( �o�̓��[�h�i�o�̓o�b�t�@�E�I���j)
	P0_bit.no2 = 0;		// 0���o��
	
	delay_msec(10);		//�@10msec�҂�  
}


// �@�^�b�`�Z���T�̃|�[�g�ݒ�
//
//    P06: TS06 : SW4
//    P23: TS08 : SW3
//    P21: TS10 : SW2
//    P41: TS13 : SW1
//
//    P02:TSCAP : �^�b�`�L�[�p���[�p�X�t�B���^�[�q(0.01uF=10nF)
//
//
// 1) P06 = TS06
//   PM (�|�[�g�E���[�h�E���W�X�^) : PM0 bit6= 1 ( ���̓��[�h�i�o�̓o�b�t�@�E�I�t�j) 
//   P (�|�[�g�E���W�X�^) : P0 bit6 = 0 (���[�E���x�������)
//   TSSEL(�^�b�`�[�q�@�\�I�����W�X�^) : TSSEL0 bit6 = 1 (�^�b�`�[�q�@�\�Ƃ��Ďg�p)
//
// 2) P23 = TS08
//   PM (�|�[�g�E���[�h�E���W�X�^) : PM2 bit3= 1 ( ���̓��[�h�i�o�̓o�b�t�@�E�I�t�j) 
//   P (�|�[�g�E���W�X�^) : P2 bit3 = 0 (���[�E���x�������)
//   TSSEL(�^�b�`�[�q�@�\�I�����W�X�^) : TSSEL1 bit0 = 1 (�^�b�`�[�q�@�\�Ƃ��Ďg�p)
//
// 3) P21 = TS10
//   PM (�|�[�g�E���[�h�E���W�X�^) : PM2 bit1= 1 ( ���̓��[�h�i�o�̓o�b�t�@�E�I�t�j) 
//   P (�|�[�g�E���W�X�^) : P2 bit1 = 0 (���[�E���x�������)
//   TSSEL(�^�b�`�[�q�@�\�I�����W�X�^) : TSSEL1 bit2 = 1 (�^�b�`�[�q�@�\�Ƃ��Ďg�p)
//
// 4) P41 = TS13
//   PM (�|�[�g�E���[�h�E���W�X�^) : PM4 bit1= 1 ( ���̓��[�h�i�o�̓o�b�t�@�E�I�t�j) 
//   P (�|�[�g�E���W�X�^) : P4 bit1 = 0 (���[�E���x�������)
//   TSSEL(�^�b�`�[�q�@�\�I�����W�X�^) : TSSEL1 bit5 = 1 (�^�b�`�[�q�@�\�Ƃ��Ďg�p)
//
// 5) TSCAP= P02 
//   PM (�|�[�g�E���[�h�E���W�X�^) : PM0 bit2= 1 ( ���̓��[�h�i�o�̓o�b�t�@�E�I�t�j) 
//   (�^�b�`�[�q�@�\���g�p����ꍇ�i�C�ӂ� TSSELxx �r�b�g��1 ��ݒ肵�����j�AP02/TSCAP �[�q�͎����I��TSCAP �@�\�ɂȂ�܂��B)
//
//
//   ����: �uRL78/G16 ���[�U�[�Y�}�j���A�� �n�[�h�E�F�A�ҁv( R01UH0980JJ0110 Rev.1.10 )
//        �u4.5.3 �g�p����|�[�g�@�\����ь��p�@�\�̃��W�X�^�ݒ��v���
//

void touch_port_set(void)
{
				// TS06
	PM0_bit.no6 = 1;	// P06�́A���̓��[�h�i�o�̓o�b�t�@�E�I�t�j
	P0_bit.no6 = 0;		// ���[�E���x�������
	TSSEL0_bit.no6 = 1;	// �^�b�`�[�q�@�\�Ƃ��Ďg�p
	
				// TS08
	PM2_bit.no3 = 1;	// P23�́A���̓��[�h�i�o�̓o�b�t�@�E�I�t�j
	P2_bit.no3 = 0;		// ���[�E���x�������
	TSSEL1_bit.no0 = 1;	// �^�b�`�[�q�@�\�Ƃ��Ďg�p
	
				// TS10
	PM2_bit.no1 = 1;	// P21�́A���̓��[�h�i�o�̓o�b�t�@�E�I�t�j
	P2_bit.no1 = 0;		// ���[�E���x�������
	TSSEL1_bit.no2 = 1;	// �^�b�`�[�q�@�\�Ƃ��Ďg�p
	
				// TS13
	PM4_bit.no1 = 1;	// P41�́A���̓��[�h�i�o�̓o�b�t�@�E�I�t�j
	P4_bit.no1 = 0;		// ���[�E���x�������
	TSSEL1_bit.no5 = 1;	// �^�b�`�[�q�@�\�Ƃ��Ďg�p
	
	
	PM0_bit.no2 = 1;	// P02�́A���̓��[�h�i�o�̓o�b�t�@�E�I�t�j
}


