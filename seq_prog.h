
#define REC_MAX        60
#define REC_MAX_FLASH  15 

extern uint8_t sw_status_record[REC_MAX];
extern uint32_t flash_sw_record[REC_MAX_FLASH];

extern uint8_t step_number;
extern uint8_t status_rcd_pt;

void  exec_prog_step(void);

void touch_record_store(void);

void clear_sw_status_record(void);

void copy_sw_record_to_flash(void);

void copy_sw_record_from_flash(void);

uint8_t write_flash_sw_record(void);

void read_flash_sw_record(void);