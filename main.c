
#include   "iodefine.h"
#include   "misratypes.h"

#include "timer.h"
#include  "ctsu.h"
#include "key.h"
#include "seq_prog.h"
#include  "uart.h"
#include  "interval_timer_12bit.h"
#include  "delay.h"
#include  "led_port.h"
#include   "iic_sd.h"

#include   "data_flash.h"
#include   "debug_port.h"

void mode_disp(void);
// ���C������
// 
void main(void)
{
	
	DEBUG_PORT_PM = 0;	// P13 �o�̓��[�h (�f�o�b�N�p)
	
	uart_1_ini();		// UART1 �ʐM�̏�����
	
	interval_timer_12bit_ini();	// 12bit �C���^�[�o���^�C�}�̋N��
	
	ctsu_self_set();	//  �Ód�e�ʎ��^�b�`�Z���T(CTSU)�̐ݒ�

	timer_array_ini();	//  ���̓N���b�N�����ƕ�����ݒ�
	timer_ch0_ini();        // �C���^�[�o���^�C�}�̐ݒ�
	timer_ch4_ini();        // �X�e�b�s���O���[�^ �p���X�ݒ�
	
	p16_out_mode();		// �X�e�b�s���O���[�^ ��]�����ݒ�

	touch_led_port();	// �^�b�`�L�[�p LED�|�[�g�ݒ�
	uart_1_led_port();      // TXD1,RXD1�p LED�|�[�g�ݒ�
	alarm_led_port();	// �A���[���p LED�|�[�g�ݒ�
	
	
	read_flash_sw_record();         // �f�[�^�t���b�V����SW�̓��샌�R�[�h��ǂݏo��(4 byte��)
	copy_sw_record_from_flash();	// �f�[�^�t���b�V���p�̃o�b�t�@(4 byte�P��)����ASW�̓��샌�R�[�h(1 byte�P��)��
	
	
	flash_clock_mode_set();   // �f�[�^�t���b�V���̏������݋���
	
	iic_ini();			// IICA�̏����ݒ�
	
	__EI();			// ���荞�݋���
	
	
	iic_slave_adrs = 0x3c;    	//  �X���[�u�A�h���X = 0x3c ( SO1602 OLED �L�����N�^�f�B�X�v���C 16����x2�s)
	oled_so1602a_ini();		// OLED ���W���[�� SO1602 ������ (�\�� ON)
	 
	touch_key_on_val();		//   �e�L�[�̃^�b�`�L�蔻��l�̃Z�b�g
	
	CTSUSTRT = 1; 		 // CTSU�v������J�n
	
	
	
	while(1)
	{
	  WDTE = 0xac;		// �E�I�b�`�h�b�N�@�J�E���^�̃N���A
		
	  if ( flg_100_msec == 1 ) {   // 100msec�o��
		
	  	mode_chg();		// ���[�h�ύX
		
		mode_disp();		// ���[�h�\��
	
		
		if ( run_flg == 1 ) {
		   if ( pre_run_flg == 0 ) {   // stop����run�ւ̕ύX��
		       TS0L_bit.no0 = 1;	// �`�����l��0�J�E���g�J�n( 1�b �^�C�} )

		       if ( teaching_flg == 1 ) {    // �e�B�[�`���O���[�h�̏ꍇ
		            status_rcd_pt = 0;	     // �X�g�A�ʒu�̏�����
			    touch_record_store();    //  ���샌�R�[�h�̍쐬����
			}
			else {			     // �ʏ탂�[�h�̏ꍇ
			    step_number = 0;         // step�ԍ��̏�����
			    exec_prog_step();	     // �v���O�����̃X�e�b�v���s
			}
			
		   }
		}
		else {			// stop�̏ꍇ
			TT0L_bit.no0 = 1;	// �`�����l��0�J�E���g��~
		}
		
		flg_100_msec = 0;	// �t���O 100msec�̃N���A
	  }
	
	  
	  if ( flg_1_sec == 1 ) {    // 1�b�o�߃t���O ON
	    
	    if ( run_flg == 1 ) {    // ���s�̏ꍇ
	       
	       if ( teaching_flg == 1 ) {   // �e�B�[�`���O���[�h�̏ꍇ
	           touch_record_store();   //  ���샌�R�[�h�̍쐬����(1�b��)
	       }
	       else {    		// �ʏ탂�[�h�̏ꍇ
	    	  exec_prog_step();	// �v���O�����̃X�e�b�v���s
	       }
	    }
	    
	     flg_1_sec = 0;		// 1�b�o�߃t���O�̃N���A
	   }
	 
	  
	  if ( rcv_over == 1 ) {	// �R�}���h��M����
		comm_cmd();		// ���X�|���X�쐬�A���M
	   	rcv_over = 0;		// �R�}���h��M�t���O�̃N���A
		rcv_cnt  = 0;		//  ��M�o�C�g���̏���
		
	   }
	
	}
	
}

//
//  ���[�h�̕\��
// �@���s����: 0.7[msec]
//
void mode_disp(void)
{
	
	if ( teaching_flg == 0 ) {		// �ʏ탂�[�h�̏ꍇ�A
	
           oled_set_ddram_address(0x00);   // �\���J�n�ʒu 1�s�ڐ擪
	   
	   oled_disp_str("�ް� � �޳��",12);     //  �ް� � �޳�� �\��
	   
           oled_set_ddram_address(0x20);   // �\���J�n�ʒu 2�s�ڐ擪
	   if ( run_flg == 1 ) {		   //  RUN�̏ꍇ
	     oled_disp_str("run    step",11);         // Run �\��
	     
	     oled_disp_step_num(step_number);    // step�ԍ��̕\��
	   }
	   else {				// �v���O�����^�]��~���[�h (STOP)�̏ꍇ
	     oled_disp_str("stop          ",14);        // Stop �\��
	   }
	   
	}
	else{					// �e�B�[�`���O���[�h�̏ꍇ
	 
	   oled_set_ddram_address(0x00);   // �\���J�n�ʒu 1�s�ڐ擪
	   oled_disp_str("è��ݸ�     ",12);      // è��ݸ�     �\��
	   
	   oled_set_ddram_address(0x20);   // �\���J�n�ʒu 2�s�ڐ擪
	   if ( run_flg == 1 ) {	   // RUN�̏ꍇ
	     oled_disp_str("run    step",11);      // Run �\��
	     
	     oled_disp_step_num(status_rcd_pt);    // �i�[�ʒu�̕\��
	   }
	   else {		           // ��~���[�h (STOP)�̏ꍇ
	     oled_disp_str("stop          ",14);      // Stop �\��
	   }
	   
	}
	
}


//
// ���g�p�Ȋ��荞�݂ɁA�o�^�����֐�
//  �u�����N�E�I�v�V�������o�̓R�[�h���x�N�^�E�e�[�u���󂫗̈�̃A�h���X�v�� _Dummy �Ƃ���B
//
//�@( �R���p�C����ARETI(61 FC))�@
//
#pragma interrupt Dummy
void Dummy(void)
{
}




