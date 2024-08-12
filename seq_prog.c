
#include   "iodefine.h"
#include   "misratypes.h"

#include "interval_timer_12bit.h"
#include "ctsu.h"
#include "key.h"

#include "data_flash.h"
#include "seq_prog.h"

#include  "debug_port.h"


uint8_t step_number;	// ���s���̃X�e�b�v�ԍ� ( 0�`59 )

uint8_t status_rcd_pt;          	// �ۑ��ʒu�������Aindex
uint8_t sw_status_record[REC_MAX];	// SW�̓��샌�R�[�h (1�b��) 

uint32_t flash_sw_record[REC_MAX_FLASH];	// �f�[�^�t���b�V���ւ̏����݁A�Ǐo���p


uint8_t flash_blk0_err;
uint8_t flash_blk1_err;
uint8_t flash_wr_err;

//
//  �v���O�����X�e�b�v����
//  �p���X�o�͂�LED�_��
//
//   sw_status_record[]�̓��e���A1�b���Ɏ��s
//�@�f�[�^:  ������e
//�@ 0x00:   SW3=OFF, SW4=OFF
//   0x01:   SW3=OFF, SW4=ON
//   0x02:   SW3=ON,  SW4=OFF
//
//
//  P05 : SW4 LED : (TS06)
//  P07 : SW3 LED : (TS08)
//

void  exec_prog_step(void)
{
	uint8_t pd;
	
	pd = sw_status_record[step_number];
	
	if ( pd == 0x01 ) {	    // ���v���(SW4 ON) 
	     
	     TOE0L_bit.no4 = 1;     // �^�C�}�o�͋����W�X�^  ch4(TO04) �o�͋��@(�X�e�b�s���O���[�^ �p���X�o��)
	     
	     P1_bit.no6 = 0;        // P16 = 0���o�́@(�X�e�b�s���O���[�^�@��������)
	     
	     P0_bit.no5 = 1;	    // P05 = 1 ���o��
	     
	     
	}
	
	else if ( pd == 0x02 ){	      // �����v���(SW3 ON)
	
	        TOE0L_bit.no4 = 1;    // �^�C�}�o�͋����W�X�^  ch4(TO04) �o�͋��@(�X�e�b�s���O���[�^ �p���X�o��)
		
		P1_bit.no6 = 1;       // P16 = 1���o�́@(�X�e�b�s���O���[�^�@��������)
		
		P0_bit.no7 = 1;		// P07 = 1 ���o��
		
	}
	
	else {			// ����OFF(SW3:OFF, SW4:OFF)
	   
	  TOE0L_bit.no4 = 0;     // �^�C�}�o�͋����W�X�^  ch4(TO04) �o�͋֎~�@(�X�e�b�s���O���[�^ �p���X�o�͂Ȃ�) 
          
	  TO0L = TO0L & 0xef;	// �`�����l��4 �^�C�}�o�� = 0
	  
	  P1_bit.no6 = 0;       // P16 = 0���o�́@(�X�e�b�s���O���[�^�@��������)
	  
	   P0_bit.no5 = 0;	      // P05 = 0 ���o��
	   
	   P0_bit.no7 = 0;	      // P07 = 0 ���o��
	
	}
	
	if ( step_number > REC_MAX - 1 ) {  // �ŏI�X�e�b�v���s�ς݂Ȃ�΁A
		 step_number = 0;       	 // step�ԍ��̃N���A
		 run_flg = 0;   	         // Stop���[�h �ɂ��� 
	 }
	
}



//
//  ���샌�R�[�h��SW��ON/OFF��Ԃ��L�^����B( 1�b�� )   
//  10[msec]���� SW3��ON�񐔁ASW4��ON�񐔁AOFF�񐔂̓��A
//  �ő�Ȃ��̂�1�b�Ԃ̓���Ƃ��ĕۑ�����B
//   SW4 ON : 0x01
//   SW3 ON : 0x02
//    OFF   : 0x00
//
//  rotate_clock_cnt    : ���v���(SW4)��ON�� 10msec���ɃJ�E���g
//  rotate_c_clock_cnt  : �����v���(SW3)   :
//  rotate_no_cnt	: ���v���A�����v��肪������Ă��Ȃ��ꍇ�ɃJ�E���g  
//

void touch_record_store(void)
{
	uint8_t  rcd_val;
	
	if (  rotate_c_clock_cnt < rotate_clock_cnt ) {   //  �����v���(SW3) <�@���v���(SW4)
	   if ( rotate_no_cnt < rotate_clock_cnt ) {      //  ����OFF < ���v���(SW4)
	         rcd_val = 0x01;
	   }
	   else {			   // ����OFF > ���v���(SW4)
	         rcd_val = 0x00;
	   }
	
	}
	else {				  //  �����v���(SW3) >= ���v���(SW4)
	  if ( rotate_no_cnt < rotate_c_clock_cnt ) {   // ����OFF < �����v���(SW3)
	         rcd_val = 0x02;
	   }
	   else {			   // ����OFF > �����v���(SW3)
	         rcd_val = 0x00;
	   }
	}
	
	sw_status_record[status_rcd_pt] = rcd_val; // ���R�[�h�̊i�[
	
	

	if ( status_rcd_pt > REC_MAX - 1  ) {    // ���R�[�h�̏���ɒB�����ꍇ
		status_rcd_pt = 0;	// �X�g�A�ʒu�̃N���A
		run_flg = 0;   	        // Stop���[�h �ɂ��� 
	}
	
	rotate_clock_cnt = 0;		// SW��ԃJ�E���^�̃N���A
	rotate_c_clock_cnt = 0;
	rotate_no_cnt = 0;
	
}

//
//  ���샌�R�[�h�Ɗi�[�ʒu�̏�����
//
void clear_sw_status_record(void)
{
	uint8_t  i;
	
	for ( i = 0 ; i < REC_MAX; i++ ) {
		
	        sw_status_record[i] = 0x00;
	
	}
	
	status_rcd_pt = 0;	// �X�g�A�ʒu�̃N���A
}


//  
//  SW�̓��샌�R�[�h ���f�[�^�t���b�V���p�̃o�b�t�@�փR�s�[
//   sw_status_record[]  -->   flash_sw_record[]
//       0x00                    0x03020100
//       0x01
//       0x02
//       0x03
//
void copy_sw_record_to_flash(void)
{
	uint8_t   i;
	uint32_t  fd;
	
	for ( i = 0 ; i < REC_MAX_FLASH; i++ ) {
	  
	  fd = sw_status_record[i*4 + 3];
	  fd = fd << 8;
	  
	  fd = fd | sw_status_record[i*4 + 2];
	  fd = fd << 8;  
	  
	  fd = fd | sw_status_record[i*4 + 1];
	  fd = fd << 8;
	  
	  fd = 	fd | sw_status_record[i*4];
	  
	  flash_sw_record[i] = fd;	
	
	}
}



//  
//  �f�[�^�t���b�V���p�̃o�b�t�@����ASW�̓��샌�R�[�h��
// flash_sw_record[] ---> sw_status_record[] 
//    0x03020100    �@�@�@ 0x00                   
//      �@�@�@�@�@�@�@�@�@ 0x01
//       �@�@�@�@�@�@�@�@�@0x02
//      �@�@�@�@�@�@�@�@�@ 0x03
//
void copy_sw_record_from_flash(void)
{
	uint8_t   i;
	uint32_t  fd;
	
	for ( i = 0 ; i < REC_MAX_FLASH; i++ ) {
		
	  fd = 	flash_sw_record[i];
	    
	  sw_status_record[i*4] = fd;
	 
	  sw_status_record[i*4 + 1] = fd >> 8;
	 
	  sw_status_record[i*4 + 2] = fd >> 16;
	 
	  sw_status_record[i*4 + 3] = fd >> 24;
	
	}
}


//
// SW�̓��샌�R�[�h���f�[�^�t���b�V���֏�������
// �f�[�^�t���b�V���������ݒP��: 32bit
//                 �����P��:  512 byte
//
// flash_sw_record[i]    �f�[�^�t���b�V���A�h���X          
//        0                0x 9000                           
//  �@�@�@1                0x 9004 
//        2                0x 9008
//        :                    :
//       14                0x 9038
//
//  ���^�[���l: 0x00 = ����I��
//
//  ��������: 6.2[msec] (512byte�̏��� + 4byte�f�[�^��15�񏑂�����)
//
uint8_t write_flash_sw_record(void)
{
	uint8_t i;
	
//	DEBUG_PORT_OUT = 1;	// P13 = 1 (�f�o�b�N�p)
	
	__DI();			// ���荞�݋֎~ �i���荞�݋��t���O�iIE�j���N���A�i0�j�j
	
	flash_blk0_err = flash_erase_data(0);	// �f�[�^�t���b�V�� Block=0�̏��� 5.3[msec]
	
	if ( flash_blk0_err != 0 ) {		
		return flash_blk0_err;
	}
	
	
	for ( i = 0 ; i < REC_MAX_FLASH ; i++ ) {
		
	   flash_wr_err = flash_write_data( 0x9000 + i*4 , flash_sw_record[i] ); // �f�[�^�������� 
	
	   if ( flash_wr_err != 0 ) {		
		return flash_wr_err;
	   }
	}
	
	__EI();			// ���荞�݋���
	
	
//   DEBUG_PORT_OUT = 0;	// P13 = 0 (�f�o�b�N�p)
	
        return 0x00;
	
}



//
// �f�[�^�t���b�V����SW�̓��샌�R�[�h��ǂݏo��
//
// flash_sw_record[i]    �f�[�^�t���b�V���A�h���X          
//        0                0x 9000                           
//  �@�@�@1                0x 9004 
//        2                0x 9008
//        :                    :
//       14                0x 9038
//

void read_flash_sw_record(void)
{
	uint8_t i;
	
	for ( i = 0 ; i < REC_MAX_FLASH ; i++ ) {
		
	   flash_sw_record[i]  = flash_read_data( 0x9000 + i*4 );	// �f�[�^�ǂݏo�� 
	
	}
}

