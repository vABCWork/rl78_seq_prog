
void flash_clock_mode_set(void);

uint8_t flash_erase_data(uint8_t blk_num);

uint8_t flash_write_data( uint16_t wr_addrs, uint32_t wr_data);

uint32_t flash_read_data( uint16_t rd_addrs );
