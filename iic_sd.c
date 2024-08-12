
#include   "iodefine.h"
#include   "misratypes.h"

#include    "iic_sd.h"
#include  "delay.h"


uint8_t iic_slave_adrs;  // IIC �X���[�u�A�h���X  00: 7bit�A�h���X( ��:100 0000 = 0x40 )

volatile uint8_t iic_sd_data[32];    // ���M�f�[�^
volatile uint8_t iic_sd_pt;	    // ���M�f�[�^�ʒu


volatile uint8_t  iic_sd_num;	    // ���M�f�[�^��(�X���[�u�A�h���X���܂�)
volatile uint8_t  iic_rcv_num;      // ��M�f�[�^��
volatile uint8_t  iic_com_over_fg;  // 1:STOP�R���f�B�V�����̌��o��



// IICA ���荞�ݏ����v���O�����̐擪�A�h���X���AINTIICA0(0x0002C)�֓o�^
#pragma interrupt INT_iica(vect=INTIICA0)

// IICA ���荞�ݏ��� 
//  ���荞�ݔ����^�C�~���O
// 1) WTIM0(IICCTL00���W�X�^ bit3) = 1 �@
//    9 �N���b�N�ڂ̗���������Ŋ��荞�ݗv�������B (ACK/NACK)
// 2) SPIE0(IICCTL00���W�X�^ bit4) = 0
//    �X�g�b�v�E�R���f�B�V�������o�ɂ�銄�荞�ݗv�������@�֎~
//
// �Q�l: RL78/G16 ���[�U�[�Y�}�j���A�� �n�[�h�E�F�A��
//   14.5.17 I2C ���荞�ݗv���iINTIICA0�j�̔����^�C�~���O
//    ( WTIM0��1 �ݒ� )
//

void INT_iica(void)
{
  				
      if ( ACKD0 == 0 ) {	// ACK�����o���Ă��Ȃ��ꍇ
	  SPT0 = 1;	// �X�g�b�v�E�R���f�B�V�����𐶐� 
	  IICAMK0 = 1;	// IICA���荞�݋֎~
	
	  iic_com_over_fg = 1;	// �ʐM�I���t���O�̃Z�b�g
          
	  return;
      }
      
      
      if ( iic_sd_pt < iic_sd_num ) {   // ���M�f�[�^������ꍇ
      
	  IICA0 = iic_sd_data[iic_sd_pt];  // �f�[�^�̑��M
	
	  iic_sd_pt++;			 // ���M�ʒu�̍X�V
      }
      else {			// �S�đ��M�ς�
	  SPT0 = 1;	// �X�g�b�v�E�R���f�B�V�����𐶐� 
	  IICAMK0 = 1;	// IICA���荞�݋֎~
	  
	  iic_com_over_fg = 1;	// �ʐM�I���t���O�̃Z�b�g
      }
      
}




//  OLED �L�����N�^�f�B�X�v���C(SO1602A)�̏�����
//  I2C�ڑ� 16���� x 2�s�@
//
void oled_so1602a_ini(void)
{
	
	oled_clear_display();	// �\���̃N���A
	
	delay_msec(2);		// 2[msec]�҂�
	
	oled_disp_on();		// �\�� ON
	
	delay_40_usec();	// 40[usec]�҂�
	
}

//
// �\���̃N���A
// �󔒂̏�������
void oled_clear_display(void)
{

	iic_sd_data[0] = ( iic_slave_adrs << 1 ) ;  // �X���[�u�A�h���X�֏�������
	iic_sd_data[1] = 0x00;          // �R�}���h 
	iic_sd_data[2] = 0x01;		//  Clear display
	iic_master_send (3);		// �}�X�^���M

	while( iic_com_over_fg != 1 ) {    // ���M�����҂�
	} 
	
}

//
//  �\���@ON
//  Display ON, cursor OFF, blink OFF
//
void oled_disp_on(void)
{
	
	iic_sd_data[0] = ( iic_slave_adrs << 1 ) ;  // �X���[�u�A�h���X�֏�������
	iic_sd_data[1] = 0x00;          // �R�}���h 
	
	iic_sd_data[2] = 0x0c;		//  Display ON, cursor OFF, blink OFF
	
	iic_master_send (3);		// �}�X�^���M

	while( iic_com_over_fg != 1 ) {    // ���M�����҂�
	} 
}



//  �\���ʒu�̎w��
//  ��̍s 0x00�`0x0f
//  ���̍s 0x20�`0x2f
//
void oled_set_ddram_address(uint8_t ac_val)
{
	iic_sd_data[0] = ( iic_slave_adrs << 1 ) ;  // �X���[�u�A�h���X�֏�������
	iic_sd_data[1] = 0x00;                  // Instruction  Write operation
	iic_sd_data[2] = (0x80 | ac_val);	// Set DDRAM address instruction
	
	iic_master_send (3);	// �}�X�^���M
	
	while( iic_com_over_fg != 1 ) {    // ���M�����҂�

	} 
	
	delay_40_usec();		// 40[usec]�҂� (SO1602A �R�}���h��������)
}



//
// ������̕\��
//
// ����:   pt_str       : �\�������񂪊i�[����Ă���擪�A�h���X
//          num         :�@�\�������� (1�`8)
//
void oled_disp_str( char *pt_str, uint8_t num)
{
	uint8_t i;
	
	iic_sd_data[0] = ( iic_slave_adrs << 1 ) ;  // �X���[�u�A�h���X�֏�������
	iic_sd_data[1] = 0x40;		// Data Write operation
	
	for ( i = 0 ; i < num ; i++ ) {
	    iic_sd_data[2 + i] = *pt_str;    // �\�����镶���R�[�h���i�[
	    
	     pt_str++;
	}
	
	iic_master_send (2 + num);	// �}�X�^���M
	
	while( iic_com_over_fg != 1 ) {    // ���M�����҂�
	
	} 
	
	delay_40_usec();		// 40[usec]�҂@ (SO1602A �R�}���h��������)
	
}



//  �v���O�������s����step�ԍ��A�e�B�[�`���O�f�[�^�̊i�[�ʒu��\���@(0�`59)
//   ����: step�ԍ��܂��͊i�[�ʒu�@(0�`59)
//  
void oled_disp_step_num( uint8_t step )
{
	uint8_t keta_2;
	uint8_t keta_1;
	
	keta_2 = step / 10;		// 10�̌��̒l
	keta_1 = step - (keta_2 * 10);  // 1�̌��̒l
	
		
	oled_set_ddram_address(0x2c);   // �\���J�n�ʒu 2�s�� col=7
	
	
	iic_sd_data[0] = ( iic_slave_adrs << 1 ) ;  // �X���[�u�A�h���X�֏�������
	iic_sd_data[1] = 0x40;		// Data Write operation
	
	iic_sd_data[2] = keta_2 + 0x30;       // 10�̌��̕\�����镶���R�[�h���i�[
	iic_sd_data[3] = keta_1 + 0x30;       // 1�̌��̕\�����镶���R�[�h���i�[
	
	iic_master_send (4);	// �}�X�^���M
	
	while( iic_com_over_fg != 1 ) {    // ���M�����҂�
	
	} 
	
	delay_40_usec();		// 40[usec]�҂@ (SO1602A �R�}���h��������)
	
}









// IIC �}�X�^���M
//   �X���[�u�ց@���M�o�b�t�@�@iic_sd_data[]�̃f�[�^�� sd_num�o�C�g���M����B
// ����: sd_num  ���M�o�C�g���@
// 
//   
void iic_master_send ( uint8_t sd_num)
{
	iic_sd_num = sd_num;		// ���M�f�[�^��
	
	iic_sd_start();			// IICA ���M�J�n
}



//  IIC ���M�J�n
// MSTS0 : IICA �X�e�[�^�X�E���W�X�^0�iIICS0�j�� bit7
//
void iic_sd_start(void)
{
	
	iic_sd_pt = 0;			// ���M�f�[�^�ʒu�@�N���A
	iic_com_over_fg = 0;	        // �ʐM�����t���O�̃N���A
	
	
	while ( IICBSY0 == 1 ) {	// �o�X����҂� , IICBSY0 : IICA �t���O�E���W�X�^0�iIICF0�j�� bit6 
	}

	IICAMK0 = 0;		// IICA���荞�݃}�X�N�̃N���A
	
	STT0 = 1;		// �X�^�[�g�R���f�B�V�����̔��s  (�}�X�^���M�̊J�n)  IICCTL00��bit1
	
//	�X�^�[�g�E�R���f�B�V�������������ꂽ���܂��͋��₳�ꂽ���́ASTCF0�iIICF0 ���W�X�^�̃r�b�g7�j�Ŋm�F�ł�
//�܂��BSTT0��1 �Ƃ��Ă���STCF0 ���Z�b�g�i1�j�����܂�5 �N���b�N�̎��Ԃ�������܂��̂ŁA�\�t�g�E�F�A�ɂ��
//���̎��Ԃ��m�ۂ��Ă������� p650
	
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	
					// �X�^�[�g�R���f�B�V�������s�ł��Ȃ��ꍇ�A���[�v
	while( STCF0 == 1 ) {		// �X�^�[�g�E�R���f�B�V�������s�ł����ASTT0 �t���O�E�N���A�BSTCF0�iIICF0 ���W�X�^�̃r�b�g7�j
	}
	
	while( STD0 == 0 ) {		// �X�^�[�g�E�R���f�B�V���������o�҂�, IICA �X�e�[�^�X�E���W�X�^0�iIICS0�j�� bit1
	}
	
        IICA0 = iic_sd_data[iic_sd_pt];  // �X���[�u�A�h���X�̑��M
	
        iic_sd_pt++;			 // ���M�ʒu�̍X�V
					 // �X���[�u�����ACK�����ŁA���荞�݂ɓ��� 
	
}



//
//  IICA�̏����ݒ�
//
//  �g�p�[�q: SCLA0 = P60
//            SDAA0 = P61
//
void iic_ini(void)
{
	IICA0EN = 1;		// �V���A���E�C���^�[�t�F�C�X IICA �ւ̃N���b�N�����@(���ӃC�l�[�u���E���W�X�^0�iPER0�j��bit4=1)
	IICE0 = 0;		// �����~  (IICCTL00���W�X�^ bit7 )�̐ݒ�
	IICAMK0 = 1;		// IICA���荞�݋֎~ (MK1L���W�X�^�� bit4)
	IICAIF0 = 0;		// IICA���荞�ݗv���t���O�̃N���A (IF1L���W�X�^�� bit4)
	
	IICAPR00 = 1;		// IICA ���荞�݃��x�� = 3 (���x��0�`3 :0���ō�)
	IICAPR10 = 1;
	
	PIOR3 = PIOR3 & 0xf3;   // �g�p�[�q�̐ݒ� PIOR33=0,PIOR32=0 (SCLA0 = P60, SDAA0 = P61)
	
				
	SMC0 = 1;		// �t�@�[�X�g�E���[�h�œ���i�ő�]�����[�g�F400 kbps�j, IICA �R���g���[���E���W�X�^01�iIICCTL01�j��bit3
	
	DFC0 = 1;		// �f�W�^���E�t�B���^�E�I�� , IICA �R���g���[���E���W�X�^01�iIICCTL01�j��bit3
	
	SVA0 = 32;		// ���ǃA�h���X�i�}�X�^�ʐM�ł͎g�p���Ȃ�)
	
	IICWL0 = 21;		// �]���N���b�N=400[KHz]�ݒ� �@Low���x����
	IICWH0 = 20;		// High���x����
	
				// IICA �t���O�E���W�X�^0�iIICF0�j
	STCEN0 = 1;		// �X�g�b�v�E�R���f�B�V���������o�����ɁA�X�^�[�g�E�R���f�B�V�����𐶐�
	
	IICRSV0 = 1;		// �ʐM�\��֎~ IICA �t���O�E���W�X�^0�iIICF0�j�� bit0
	
	SPIE0 = 0;		// �X�g�b�v�E�R���f�B�V�������o�Ŋ��荞�ݔ������Ȃ��@IICCTL00���W�X�^��bit4
	
	WTIM0 = 1;		// �N���b�N�E�X�g���b�`����ъ��荞�ݗv�������̐��� 
				// 9 �N���b�N�ڂ̗���������Ŋ��荞�ݗv�������BIICCTL00���W�X�^��bit3
				// �}�X�^�̏ꍇ�F9 �N���b�N�o�͌�A�N���b�N�o�͂����E�E���x���ɂ����܂܃N���b�N�E�X�g���b�`
				
	ACKE0 = 1;		// �A�N�m���b�W�����B9 �N���b�N���Ԓ���SDAA0 ���C�������E�E���x���ɂ���BIICCTL00���W�X�^��bit2
	
	IICAMK0 = 0;		// IICA���荞�݋��� (MK1L���W�X�^�� bit4)
	
	IICE0 = 1;		// ���싖�@IICCTL00���W�X�^��bit7
	
	LREL0 = 1;		// �ҋ@��� IICCTL00���W�X�^��bit6
	
	PM6_bit.no0 = 0;	// P60:�o�̓��[�h
	PM6_bit.no1 = 0;        // P61:�o�̓��[�h
	
	                         // P60 = 1, P61 = 1�ɂ���ƃR�}���h�����s����Ȃ��B
	P6_bit.no0 = 0;		// P60: �o�̓��b�` = 0
	P6_bit.no1 = 0;         // P61: �o�̓��b�` = 0
	
}


