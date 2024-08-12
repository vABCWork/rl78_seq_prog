
#include "iodefine.h"
#include "misratypes.h"
#include "led_port.h"



//
//  SW3�`SW4 LED �o�̓|�[�g�ݒ�
//  
//  P05 : SW4 LED : (TS06)
//  P07 : SW3 LED : (TS08)
//
// 1) PCM (�|�[�g�o�̓��[�h�E���W�X�^): CMOS �o�́^N-ch �I�[�v���E�h���C���o�͂�1 �r�b�g�P�ʂŐݒ�
//    0: �ʏ�o�̓��[�h 
//    1: N-ch �I�[�v���E�h���C���o�́iVDD �ψ��j���[�h
//    ���Z�b�g�M���̔����ɂ��A00H
//
// 2)PMC(�|�[�g�E���[�h�E�R���g���[���E���W�X�^): �f�W�^�����o�́^�A�i���O���͂� 1 �r�b�g�P�ʂŐݒ�
//   0:�f�W�^�����o�́i�A�i���O���͈ȊO�̌��p�@�\�j
//   1:�A�i���O����
//  ���Z�b�g�M���̔����ɂ��AFFH
//
// 3)PM(�|�[�g�E���[�h�E���W�X�^): �|�[�g�̓��́^�o�͂� 1 �r�b�g�P�ʂŐݒ�
//   0:�o�̓��[�h�i�o�̓o�b�t�@�E�I���j
//   1:���̓��[�h�i�o�̓o�b�t�@�E�I�t�j
//���Z�b�g�M���̔����ɂ��AFFH
//
//  4)P(�|�[�g�E���W�X�^) : �|�[�g�̏o�̓��b�`�̒l��ݒ�
//   0: 0 ���o�́i�o�̓��[�h���j
//   1: 1 ���o�́i�o�̓��[�h���j
//���Z�b�g�M���̔����ɂ��AP13 �͕s��A���̑���00H
//
//  5)TSSELt(�^�b�`�[�q�@�\�I�����W�X�^):�^�b�`�[�q�@�\�ȊO�i���p�@�\�j�^�^�b�`�[�q�@�\�̂ǂ���Ŏg�p���邩��ݒ�
//   0: �^�b�`�[�q�@�\�ȊO�i���p�@�\�j�Ƃ��Ďg�p
//   1: �^�b�`�[�q�@�\�Ƃ��Ďg�p
//  ���Z�b�g�M���̔����ɂ��A00H
//
/// �����uRL78/G16�@���[�U�[�Y�}�j���A�� �n�[�h�E�F�A�ҁv(Rev.1.10 2023.08)
//         4.5.3 �g�p����|�[�g�@�\����ь��p�@�\�̃��W�X�^�ݒ��
//
void touch_led_port(void)
{
	POM0_bit.no5 = 0;	// P05�́A�ʏ�o�̓��[�h
	PMC0_bit.no5 = 0;       // P05�́A�f�W�^�����o��
	PM0_bit.no5 = 0;	// P05�́A�o�̓��[�h
	P0_bit.no5 = 0;		// P05 = 0 ���o��
	
	POM0_bit.no7 = 0;	// P07�́A�ʏ�o�̓��[�h
	PMC0_bit.no7 = 0;       // P07�́A�f�W�^�����o��
	PM0_bit.no7 = 0;	// P07�́A�o�̓��[�h
	P0_bit.no7 = 0;		// P07 = 0 ���o��
	
}

//
//  TXD1,RXD1 LED �o�̓|�[�g�ݒ�
//   P11 : RXD1_LED
//   P12 : TXD1_LED
//
void uart_1_led_port(void)
{
	PM1_bit.no1 = 0;	// P11�́A�o�̓��[�h
	P1_bit.no1 = 0;		// P11 = 0 ���o��

	PM1_bit.no2 = 0;	// P12�́A�o�̓��[�h
	P1_bit.no2 = 0;		// P12 = 0 ���o��
}


//
//  Alarm LED �o�̓|�[�g�ݒ�
//   P10 : ALM_LED
//
void alarm_led_port(void)
{

	PM1_bit.no0 = 0;	// P10�́A�o�̓��[�h
	P1_bit.no0 = 0;		// P10 = 0 ���o��
}
