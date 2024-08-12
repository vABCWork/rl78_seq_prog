
#include "iodefine.h"
#include "misratypes.h"

#include "ctsu.h"
#include "key.h"
#include "interval_timer_12bit.h"


volatile uint8_t  cnt_10_msec;

volatile uint8_t  flg_100_msec;


// intera_timer���x�N�^�A�h���X(INIT=0x0038)�ɓo�^
#pragma interrupt INT_interval_timer(vect=INTIT)

// �C���^�[�o���^�C�}���荞�ݏ��� (10msec���ɔ���)
// �p�r:
//      1) CTSU�̌v���J�n�g���K�M��(10msec��)
//      2) 100msec�o�߃t���O�̃Z�b�g
//	3) �^�b�`���ꂽ�L�[�̔���ƃL�[���͊m��
//      4) �X�e�b�s���O���[�^�ւ̃p���X�o��
//
void INT_interval_timer(void)
{
	
	cnt_10_msec = cnt_10_msec + 1;
	
	if ( cnt_10_msec > 9 ) {	// 100 msec�o��
	   cnt_10_msec = 0;		// 10msec�J�E���^�@�N���A
	   flg_100_msec = 1;		// �t���O 100msec�̃Z�b�g
	}
	
	
        touch_key_status_check();	//  �e�L�[�̃^�b�`�L�蔻��l�Ɣ�r���āA�^�b�`�̗L���𔻒�
	
	sw1_sw2_input_check( 3 );       // ���[�h�؂芷��(SW1) ���͔���
	
	sw1_sw2_input_check( 2 );       // RUN/STOP,Save(SW2) ���͔���
	
	
	if (( run_flg == 0 )||( teaching_flg == 1)) {	  // ��~�܂���Teaching���[�h�̏ꍇ
	
	    sw3_sw4_input_check( 1 );	// �����v���(SW3) ���͔���
	
	    sw3_sw4_input_check( 0 );	// ���v���(SW4) ���͔���
	
	    motor_pulse_out();		// �X�e�b�s���O���[�^�ւ̃p���X�o�͂ƃe�B�[�`���O�f�[�^�̊i�[

	    touch_led_on_off_sw3_sw4();	// LED��ON/OFF (SW3,SW4�p)
	}
	
	
	
}


// �C���^�[�o���^�C�} 12bit�̏����ݒ� (10msec) �@CTSU�v���J�n�̃g���K�M��
//�@
void interval_timer_12bit_ini(void)
{
	OSMC = 0x10;		// ���Ӌ@�\�ւ̃T�u�V�X�e���E�N���b�N��������, �ᑬ�I���`�b�v�E�I�V���[�^�E�N���b�N�ifIL�j(15 [KHz])���C���^�[�o���^�C�}�֋����@

	TMKAEN = 1;		// 12 �r�b�g�E�C���^�[�o���E�^�C�}�֓��̓N���b�N���� (���ӃC�l�[�u���E���W�X�^0�iPER0�j�̃r�b�g 7 )
	
	ITMC = 149;            //  150 - 1 count, 10msec , ( 15[KHz], 1clock=1/15 [msec] , 150 count/ 15 = 10 [msec])
	
	ITMC = ITMC | 0x8000;      // RINTE:�J�E���^����J�n
	
	ITMK = 0;		// 12 �r�b�g�E�C���^�[�o���E�^�C�}���荞�݂̃}�X�N���N���A
	
} 