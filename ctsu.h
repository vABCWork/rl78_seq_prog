
#define CTSU_CH_NUM   4     // タッチセンサの個数

#define TOUCH_ON_VALUE   30000    // ONとするカウント値, ベースクロック周波数(センサドライブパルス)= 1[MHz] 



extern volatile uint16_t ctsu_ssc[CTSU_CH_NUM];
extern volatile uint16_t ctsu_so0[CTSU_CH_NUM];
extern volatile uint16_t ctsu_so1[CTSU_CH_NUM];

extern volatile uint16_t ctsu_sc[CTSU_CH_NUM];
extern volatile uint16_t ctsu_rc[CTSU_CH_NUM];

extern volatile uint16_t ctsu_errs;
extern volatile uint8_t ctsu_st;



void ctsu_self_set(void);

void ctsu_self_reg_set_multi(void);

void set_ctsuwr_data(void);

void ctsu_self_reg_set(void);

void ctsu_ini(void);

void ctsu_power_on(void);
void ctsu_clock(void);
void tscap_clear(void);

void touch_port_set(void);

