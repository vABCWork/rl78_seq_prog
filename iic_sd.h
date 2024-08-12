
extern uint8_t iic_slave_adrs;

void iic_ini(void);

void iic_master_send ( uint8_t sd_num);

void iic_sd_start(void);

void oled_so1602a_ini(void);

void oled_clear_display(void);
void oled_disp_on(void);
void oled_set_ddram_address(uint8_t ac_val);

void oled_disp_str( char *pt_str, uint8_t num);

void oled_disp_step_num( uint8_t step );


