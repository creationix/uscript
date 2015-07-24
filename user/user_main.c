#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_config.h"

#include "../lib/uscript.c"

extern volatile uint32_t PIN_OUT;
extern volatile uint32_t PIN_OUT_SET;
extern volatile uint32_t PIN_OUT_CLEAR;

extern volatile uint32_t PIN_DIR;
extern volatile uint32_t PIN_DIR_OUTPUT;
extern volatile uint32_t PIN_DIR_INPUT;

extern volatile uint32_t PIN_IN;

static void pinMode(uint8_t pin, uint8_t mode) {
  if (pin < 16) {
    if (mode) PIN_DIR_OUTPUT = 1 << pin;
    else PIN_DIR_INPUT = 1 << pin;
  }
  else if (pin == 16) {
    if (mode) {
      WRITE_PERI_REG(PAD_XPD_DCDC_CONF,
       (READ_PERI_REG(PAD_XPD_DCDC_CONF) & 0xffffffbc) | (uint32)0x1); 	// mux configuration for XPD_DCDC to output rtc_gpio0
      WRITE_PERI_REG(RTC_GPIO_CONF,
       (READ_PERI_REG(RTC_GPIO_CONF) & (uint32)0xfffffffe) | (uint32)0x0);	//mux configuration for out enable
      WRITE_PERI_REG(RTC_GPIO_ENABLE,
       (READ_PERI_REG(RTC_GPIO_ENABLE) & (uint32)0xfffffffe) | (uint32)0x1);	//out enable
    }
    else {
      WRITE_PERI_REG(PAD_XPD_DCDC_CONF,
       (READ_PERI_REG(PAD_XPD_DCDC_CONF) & 0xffffffbc) | (uint32)0x1); 	// mux configuration for XPD_DCDC and rtc_gpio0 connection
      WRITE_PERI_REG(RTC_GPIO_CONF,
       (READ_PERI_REG(RTC_GPIO_CONF) & (uint32)0xfffffffe) | (uint32)0x0);	//mux configuration for out enable
      WRITE_PERI_REG(RTC_GPIO_ENABLE,
        READ_PERI_REG(RTC_GPIO_ENABLE) & (uint32)0xfffffffe);	//out disable
    }
  }
}

static void pinWrite(uint8_t pin, uint8_t value) {
  if (pin < 16) {
    if (value) PIN_OUT_SET = 1 << pin;
    else PIN_OUT_CLEAR = 1 << pin;
  }
  else if (pin == 16) {
    WRITE_PERI_REG(RTC_GPIO_OUT,
     (READ_PERI_REG(RTC_GPIO_OUT) & (uint32)0xfffffffe) | (uint32)(value & 1));
  }
}

static uint8_t pinRead(uint8_t pin) {
  if (pin < 16) {
    return (PIN_IN >> pin) & 1;
  }
  else if (pin == 16) {
    return (uint8)(READ_PERI_REG(RTC_GPIO_IN_DATA) & 1);
  }
  return 0;
}

static unsigned char* ICACHE_FLASH_ATTR PinMode(struct uState* S, unsigned char* pc, number* res) {
  if (!res) return skip(S, skip(S, pc));
  number pin, mode;
  pc = eval(S, eval(S, pc, &pin), &mode);
  pinMode(pin, mode);
  *res = mode;
  return pc;
}

static unsigned char* ICACHE_FLASH_ATTR DigitalWrite(struct uState* S, unsigned char* pc, number* res) {
  if (!res) return skip(S, skip(S, pc));
  number pin, val;
  pc = eval(S, eval(S, pc, &pin), &val);
  pinWrite(pin, val);
  *res = val;
  return pc;
}

static unsigned char* ICACHE_FLASH_ATTR DigitalRead(struct uState* S, unsigned char* pc, number* res) {
  if (!res) return skip(S, pc);
  number pin;
  pc = eval(S, pc, &pin);
  *res = pinRead(pin);
  return pc;
}

static struct uState S;

static struct user_func funcs[] = {
  {"PM", PinMode},
  {"DW", DigitalWrite},
  {"DR", DigitalRead},
  {NULL},
};

#define user_procTaskPrio        0
#define user_procTaskQueueLen    1
os_event_t    user_procTaskQueue[user_procTaskQueueLen];
static void user_procTask(os_event_t *events);

static volatile os_timer_t some_timer;

uint8_t* prog = (uint8_t*)"DO 2 IF NOT GET i DO 2 SET i 1 PM 2 1 DW 2 NOT DR 2";

void some_timerfunc(void *arg)
{
    number res;
    eval(&S, prog, &res);
}

//Do nothing function
static void ICACHE_FLASH_ATTR
user_procTask(os_event_t *events)
{
    os_delay_us(10);
}

//Init function
void ICACHE_FLASH_ATTR
user_init()
{
    S.funcs = funcs;
    S.num_funcs = 0;
    while (funcs[S.num_funcs++].name);
    compile(&S, prog);

    // Initialize the GPIO subsystem.
    gpio_init();

    //Set GPIO2 to output mode
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);

    //Set GPIO2 low
    gpio_output_set(0, BIT2, BIT2, 0);

    //Disarm timer
    os_timer_disarm(&some_timer);

    //Setup timer
    os_timer_setfn(&some_timer, (os_timer_func_t *)some_timerfunc, NULL);

    //Arm the timer
    //&some_timer is the pointer
    //1000 is the fire time in ms
    //0 for once and 1 for repeating
    os_timer_arm(&some_timer, 1000, 1);

    //Start os task
    system_os_task(user_procTask, user_procTaskPrio,user_procTaskQueue, user_procTaskQueueLen);
}
