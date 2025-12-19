#include <asf.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

/* =================== KONFIGURASI =================== */
#define ADC_SAMPLES         16
#define TIMEOUT_ADC         100000UL
#define STATUS_THRESH_PCT   50          
#define EMA_K               8
#define SHOW_DEBUG_RAW      1
#define REF_WET_RAW         3800       
#define REF_DRY_RAW         3900       

#ifndef FORCE_WET_LOWER
#define FORCE_WET_LOWER     -1
#endif

#define RELAY_PORT          PORTA
#define RELAY_PIN_bm        PIN0_bm
#define RELAY_ACTIVE_LOW    1           

#define LED_PORT            PORTR
#define LED_PIN_bm          PIN0_bm    

/* PROTOTIPE FUNGSI */
static void adc_init_pa2(void);
static uint8_t adc_read_once(uint16_t *out);
static uint16_t adc_read_avg(uint8_t n, uint8_t *ok);
static uint8_t moisture_percent(uint16_t raw);

/* RELAY CONTROL */
static inline void relay_init(void){
    RELAY_PORT.DIRSET = RELAY_PIN_bm;
#if RELAY_ACTIVE_LOW
    RELAY_PORT.OUTSET = RELAY_PIN_bm;   
#else
    RELAY_PORT.OUTCLR = RELAY_PIN_bm;  
#endif
}

static inline void relay_on(void){
#if RELAY_ACTIVE_LOW
    RELAY_PORT.OUTCLR = RELAY_PIN_bm;  
#else
    RELAY_PORT.OUTSET = RELAY_PIN_bm;   
#endif
    LED_PORT.OUTCLR = LED_PIN_bm;       
}

static inline void relay_off(void){
#if RELAY_ACTIVE_LOW
    RELAY_PORT.OUTSET = RELAY_PIN_bm;   
#else
    RELAY_PORT.OUTCLR = RELAY_PIN_bm;   
#endif
    LED_PORT.OUTSET = LED_PIN_bm;       
}

static inline void led_init(void){
    LED_PORT.DIRSET = LED_PIN_bm;
    LED_PORT.OUTSET = LED_PIN_bm;      
}

static uint8_t moisture_percent(uint16_t raw){
    int32_t wet  = (int32_t)REF_WET_RAW;
    int32_t dry  = (int32_t)REF_DRY_RAW;
    int32_t r    = (int32_t)raw;
    if (wet == dry) return 0;

    int8_t orient = FORCE_WET_LOWER;
    if (orient < 0) orient = (wet < dry) ? 0 : 1;

    if (orient == 0) {
        if (r <= wet) return 100;
        if (r >= dry) return 0;
        return (uint8_t)(((dry - r) * 100) / (dry - wet));
    } else {
        if (r >= wet) return 100;
        if (r <= dry) return 0;
        return (uint8_t)(((r - dry) * 100) / (wet - dry));
    }
}

int main(void){
    sysclk_init();
    board_init();
    delay_init(sysclk_get_cpu_hz());
    gfx_mono_init();

#ifdef LCD_BACKLIGHT_ENABLE_PIN
    gpio_set_pin_high(LCD_BACKLIGHT_ENABLE_PIN);
#endif

    relay_init();
    led_init();
    adc_init_pa2();

    gfx_mono_draw_string("Moisture Kel. 4", 0, 0, &sysfont);

    bool ema_init = false;
    int32_t ema_raw = 0;
    bool pump_on = false;
    char buf[48];

    while (1) {
        uint8_t ok = 0;
        uint16_t raw = adc_read_avg(ADC_SAMPLES, &ok);
        bool sensor_err = !ok;

        if (!ema_init) { ema_raw = raw; ema_init = true; }
        else {
            int32_t diff = (int32_t)raw - ema_raw;
            ema_raw += diff / EMA_K;
            if (ema_raw < 0) ema_raw = 0;
            if (ema_raw > 4095) ema_raw = 4095;
        }
        uint16_t r_smooth = (uint16_t)ema_raw;

        uint8_t pct = moisture_percent(r_smooth);
        bool basah = (pct > STATUS_THRESH_PCT);

        if (sensor_err) pump_on = false;
        else if (basah) pump_on = false;
        else pump_on = true;

        if (pump_on) relay_on();
        else relay_off();

        snprintf(buf, sizeof(buf), "Persentase: %3u%%", pct);
        gfx_mono_draw_string(buf, 0, 8, &sysfont);
        gfx_mono_draw_string(basah ? "Status : BASAH " : "Status : KERING", 0, 16, &sysfont);
#if SHOW_DEBUG_RAW
        snprintf(buf, sizeof(buf), "RAW:%4u PUMP:%s", r_smooth, pump_on ? "ON " : "OFF");
        gfx_mono_draw_string(buf, 0, 24, &sysfont);
#endif
        delay_ms(150);
    }
}

static void adc_hw_enable(void){
    sysclk_enable_module(SYSCLK_PORT_A, SYSCLK_ADC);
}
static void adc_configure_pa2(void){
    PORTA.DIRCLR = PIN2_bm;
    PORTA.PIN2CTRL = PORT_ISC_INPUT_DISABLE_gc;
    ADCA.PRESCALER   = ADC_PRESCALER_DIV64_gc;
    ADCA.CTRLB       = ADC_RESOLUTION_12BIT_gc;
    ADCA.REFCTRL     = ADC_REFSEL_INTVCC_gc;
    ADCA.CH0.CTRL    = ADC_CH_INPUTMODE_SINGLEENDED_gc;
    ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN2_gc;
    ADCA.CTRLA       = ADC_ENABLE_bm;
}
static uint8_t adc_read_once(uint16_t *out){
    ADCA.CH0.CTRL |= ADC_CH_START_bm;
    uint32_t t=0;
    while(!(ADCA.CH0.INTFLAGS & ADC_CH_CHIF_bm)){
        if(++t > TIMEOUT_ADC) return 0;
    }
    ADCA.CH0.INTFLAGS = ADC_CH_CHIF_bm;
    *out = ADCA.CH0.RES;
    return 1;
}
static void adc_init_pa2(void){
    adc_hw_enable();
    adc_configure_pa2();
    uint16_t d;
    for(uint8_t i=0;i<3;i++){ if(!adc_read_once(&d)) break; delay_ms(2); }
}
static uint16_t adc_read_avg(uint8_t n, uint8_t *ok){
    uint32_t acc=0; uint16_t v=0; uint8_t cnt=0;
    for(uint8_t i=0;i<n;i++){
        if(adc_read_once(&v)) { acc += v; cnt++; }
        delay_ms(2);
    }
    if (ok) *ok = cnt;
    if (cnt==0) return 0;
    return (uint16_t)(acc / cnt);
}