
#include   "iodefine.h"
#include   "misratypes.h"

#include "ctsu.h"

#include "seq_prog.h"
#include "key.h"

// �X�C�b�`���͊֌W�̍\����
//  
struct SW_info
{
	uint8_t status;		// ����� �^�b�`�L��(Low=0),��^�b�`(High=1)��� (10msec��)

	uint8_t pre_status;	// �O��(10msec�O)�� �^�b�`�L��(Low=0),��^�b�`(High=1)��� 
	
	uint8_t low_cnt;	// �^�b�`�L��̉�

	uint8_t one_push;	// 0:�L�[���͏����v���Ȃ� 1:�L�[���͏����v��(1�x����)�@�i�L�[���͏������0�N���A�j
	
	uint8_t edge_flg;	// 1: �����茟�o�ς�
	
	uint16_t threshold;	// ON�Ɣ��肷�� CTSU�̃J�E���g�l 

};


volatile struct  SW_info  Key_sw[CTSU_CH_NUM];	// �X�C�b�`�����̏��i�[�̈�


uint8_t teaching_flg;		// teaching���[�h = 1, �ʏ탂�[�h = 0
uint8_t run_flg;        	// ���s�� = 1, ��~��=0 
			        // �ʏ탂�[�h�ŁA���s���F�v���O�����^�]����
                                // teachig���[�h�ŁA���s��: �e�B�[�`���O�f�[�^�̊i�[
uint8_t pre_run_flg;		// 100msec�O��run_flg
				
uint8_t rotate_clock_cnt;	// ���v���(SW4)��ON�� 100msec���ɃJ�E���g
uint8_t rotate_c_clock_cnt;	// �����v���(SW3)   :
uint8_t rotate_no_cnt;		// ���v���A�����v��肪������Ă��Ȃ��ꍇ�ɃJ�E���g  





// �^�b�`�L�[��LED��ON/OFF (SW3,SW4�p)
//
// ctsu_sc[0] : TS06:  Key_sw[0]: ���v���   SW4
// ctsu_sc[1] : TS08:  Key_sw[1]: �����v��� SW3
//
//  P05 : SW4 LED : (TS06)
//  P07 : SW3 LED : (TS08)
//

void touch_led_on_off_sw3_sw4(void)
{
	 if ( Key_sw[0].one_push == 1 )  { // SW4 ON
	 	P0_bit.no5 = 1;		   // P05 = 1 ���o��
	 }
	 else{
		 P0_bit.no5 = 0;	      // P05 = 0 ���o��
	 }
	
	 if ( Key_sw[1].one_push == 1 )  { // SW3 ON
	 	P0_bit.no7 = 1;		      // P07 = 1 ���o��
	 }
	 else{
		 P0_bit.no7 = 0;	      // P07 = 0 ���o��
	 }
}



//
//   �X�e�b�s���O���[�^�ւ̃p���X�o��
//
// ctsu_sc[0] : TS06:  Key_sw[0]: ���v���   SW4
// ctsu_sc[1] : TS08:  Key_sw[1]: �����v��� SW3
//

void motor_pulse_out(void)
{
	if ( Key_sw[0].one_push == 1 ) {	// ���v���(SW4) ON

	     TOE0L_bit.no4 = 1;     		// �^�C�}�o�͋����W�X�^  ch4(TO04) �o�͋��@(�X�e�b�s���O���[�^ �p���X�o��)
	 
	     P1_bit.no6 = 0;      		// P16 = 0���o�́@(�X�e�b�s���O���[�^�@��������)
		
	     rotate_clock_cnt++;	        //  ���v���(SW4)��ON�񐔂̃C���N�������g
	}

	else if ( Key_sw[1].one_push == 1 ) {	// �����v���(SW3) ON
	  
             TOE0L_bit.no4 = 1;     		// �^�C�}�o�͋����W�X�^  ch4(TO04) �o�͋��@(�X�e�b�s���O���[�^ �p���X�o��)
		
	      P1_bit.no6 = 1;       		// P16 = 1���o�́@(�X�e�b�s���O���[�^�@��������)
		
	      rotate_c_clock_cnt++;	      	//  �����v���(SW3)��ON�񐔂̃C���N�������g
	}
	
	else {
	
            TOE0L_bit.no4 = 0;     		// �^�C�}�o�͋����W�X�^  ch4(TO04) �o�͋֎~�@(�X�e�b�s���O���[�^ �p���X�o�͂Ȃ�) 
          
	    TO0L = TO0L & 0xef;			// �`�����l��4 �^�C�}�o�� = 0
	 
	    P1_bit.no6 = 0;       		// P16 = 0���o�́@(�X�e�b�s���O���[�^�@��������)
	
	    rotate_no_cnt++;			//  ���v���A�����v��肪������Ă��Ȃ��񐔂̃C���N�������g
	 
	}

}

//  �@
//    ���[�h�ύX�@�Ɓ@�L�[�����v���t���O�̃N���A
//
// ctsu_sc[2] : TS10:  key_se[2]: SW2: �J�n�X�C�b�`(RUN/STOP)                                                      
// ctsu_sc[3] : TS13:  key_sw[3[: SW1: ���[�h�؂芷�� (�ʏ� /�e�B�[�`���O) 
//

void mode_chg(void)
{
	uint8_t fl_err;
	
	
	pre_run_flg = run_flg;			// �ύX�O�̏�Ԃ��i�[
	
	if ( Key_sw[3].one_push == 1 ) {	// SW1 ON
	     if ( teaching_flg == 1 ) {		// �e�B�[�`���O���[�h�̏ꍇ�A
	     	teaching_flg = 0;		// �ʏ탂�[�h�ɂ���
		                                // �e�B�[�`���O���[�h����ʏ탂�[�h�ւ̕ύX���ɁASW���샌�R�[�h���t���b�V���֕ۑ�
						
		copy_sw_record_to_flash();	// SW�̓��샌�R�[�h ���f�[�^�t���b�V���p�̃o�b�t�@�փR�s�[
		fl_err = write_flash_sw_record();	// �t���b�V���֕ۑ�
		
	     }
	     else {				// �ʏ탂�[�h�̏ꍇ
	     	teaching_flg = 1;		// �e�B�[�`���O���[�h�ɂ���
		
	     }
	     					// �v���O�����^�]���܂��̓e�B�[�`���O���s���� SW1�������ꂽ�ꍇ�A
	    run_flg = 0;			// ��~���[�h (STOP)�ɂ���
	   
	     Key_sw[3].one_push = 0;		// �L�[���͏����v��(1�x����)�̃N���A
	}  
	
	else if ( Key_sw[2].one_push == 1 ) {	// SW2 ON
	     if ( teaching_flg == 0 ) {		// ���݁A�ʏ탂�[�h�̏ꍇ
	     	   if ( run_flg == 0 ) {        // Stop�̏ꍇ
		   	run_flg = 1;	        // �v���O�����^�]���[�h (RUN)�ɂ���
			step_number = 0;	//  step�ԍ��̃N���A
		   }
		   else {			// Run�̏ꍇ
		   	run_flg = 0;	        // �v���O�����^�]��~���[�h (STOP)�ɂ���
		   }
	      }
	      else {				// ���݁A�e�B�[�`���O���[�h�̏ꍇ
	     	   if ( run_flg == 0 ) {        // stop�̏ꍇ
		   	run_flg = 1;	  	// ���s���[�h �ɂ���
			clear_sw_status_record();	// SW ���R�[�h�̃N���A
		   }
		   else {			// �e�B�[�`���O���s���[�h�̏ꍇ
		   	run_flg = 0;            // ��~���[�h (STOP)�ɂ���
		   }
	     }
	     Key_sw[2].one_push = 0;		// �L�[���͏����v��(1�x����)�̃N���A
	}
}


//
//  �e�L�[�̃^�b�`�L�蔻��l�̃Z�b�g
// 
void touch_key_on_val(void)
{
	uint8_t i;
	
	for( i = 0; i < CTSU_CH_NUM ; i++ ) {
	
		Key_sw[i].threshold = TOUCH_ON_VALUE;
	}
	
}

//
//   �e�L�[�̃^�b�`�L�蔻��l�Ɣ�r���āA�^�b�`�̗L���𔻒肷��B
//
// ctsu_sc[0] : TS06:  Key_sw[0]: ���v���   SW4
// ctsu_sc[1] : TS08:  Key_sw[1]: �����v��� SW3
// ctsu_sc[2] : TS10:  key_se[2]: RUN/STOP �؂芷��(�ʏ탂�[�h��),����f�[�^�̕ۑ� (�e�B�[�`���O���[�h��) SW2                                                      
// ctsu_sc[3] : TS13:  key_sw[3[: ���[�h�؂芷�� (�ʏ탂�[�h /�e�B�[�`���O���[�h) SW1
//
void touch_key_status_check(void)
{
	uint8_t i;
	
	for( i = 0; i < CTSU_CH_NUM ; i++ ) {
	
	  if ( ctsu_sc[i] > Key_sw[i].threshold ) {  //  �^�b�`�L��(ON)����l���傫���ꍇ�A
	  
	  	Key_sw[i].status = 0;		// �^�b�`�L�� (Low=0)
	  }
	  else {
	  	Key_sw[i].status = 1;		// �^�b�`���� (High=1)
	  }
	  
	}
}


//
//  ���[�h�؂芷��(SW1)�ARUN/STOP,Save(SW2) �p �L�[���͂̊m�菈��  (10msec���A�^�C�}���荞�ݓ��Ŏ��s)
//    �����茟�o��A3��A�����ă^�b�`�L�蔻��Ȃ�΁ASW�I���m��t���O���Z�b�g����B
//
//  ����: id = 2 �` 3
//
// ctsu_sc[2] : TS10:  key_se[2]: RUN/STOP �؂芷��(�ʏ탂�[�h��),����f�[�^�̕ۑ� (�e�B�[�`���O���[�h��) SW2                                                      
// ctsu_sc[3] : TS13:  key_sw[3[: ���[�h�؂芷�� (�ʏ탂�[�h /�e�B�[�`���O���[�h) SW1
//

void sw1_sw2_input_check( uint8_t id ) 
{
	
	 if ( Key_sw[id].status == 0 ) {     // ���� �^�b�`���
	
	     if ( Key_sw[id].pre_status == 1 ) {  // �O�� ��^�b�`��� (��������̔���)
	     	   Key_sw[id].edge_flg = 1;	  // �����茟�o�t���O�̃Z�b�g
	           Key_sw[id].low_cnt =  1;       // Low�J�E���g = 1 �Z�b�g
	     }
	     else{				// �O�� �^�b�`���
	      	  if ( Key_sw[id].edge_flg == 1 ) {   // �����茟�o�t���O���Z�b�g����Ă���ꍇ
		     if ( Key_sw[id].low_cnt > 2 ) {   // 3��ȏ�@�^�b�`���o�̏ꍇ
			    Key_sw[id].one_push = 1;      // �L�[���͏����v��(1�x����)
			    Key_sw[id].edge_flg = 0;	  // �����茟�o�t���O�̃N���A
		      }
		      else {
		  	Key_sw[id].low_cnt++;         // Low�J�E���g�̃C���N�������g
		      }
		  }
	     }
	  }      // ����^�b�`���
	  
	  else {	// ���� ��^�b�`���
	  
	  	Key_sw[id].low_cnt = 0;		//  Low�J�E���g�̃N���A
	  }
	  
	  Key_sw[id].pre_status = Key_sw[id].status;   // ���݂̏�Ԃ��A��O�̏�ԂփR�s�[
}




//
//  ���v���(SW4)�A�����v���(SW3) �p �L�[���͂̊m�菈��  (10msec���A�^�C�}���荞�ݓ��Ŏ��s)
//    �^�b�`�L�蔻��Ȃ�΁ASW�I���m��t���O���Z�b�g����B
//
//  ����: id = 0 �` 1
//
// ctsu_sc[0] : TS06:  Key_sw[0]: ���v���   SW4
// ctsu_sc[1] : TS08:  Key_sw[1]: �����v��� SW3
//

void sw3_sw4_input_check( uint8_t id ) 
{
	if ( Key_sw[id].status == 0 ) {     // ���� �^�b�`���
	    
	      Key_sw[id].one_push = 1;	    // SW�I���m��t���O���Z�b�g
	}
	else {				    // ���� �^�b�`����
	      
	       Key_sw[id].one_push = 0;	    // SW�I���m��t���O���N���A
	}
}

