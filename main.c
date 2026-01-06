/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    EEE 158 Machine Problem 1: Colorful Lighting
    PIC32CM LS00 Curiosity Nano with Curiosity Nano Explorer

  Description:
    RGB LED control with brightness and auto-cycling modes.
*******************************************************************************/

#include "definitions.h"
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>

/*******************************************************************************
 * DEFINITIONS
 ******************************************************************************/
#define PWM_PERIOD 1875  // 48MHz / 256 prescaler / 100Hz = 1875

// Operating modes
typedef enum {
    MODE_IDLE = 0,
    MODE_BRIGHTNESS,
    MODE_AUTO_CYCLE
} OperatingMode_t;

// Auto-cycle direction
typedef enum {
    DIR_FORWARD = 0,
    DIR_REVERSE
} CycleDirection_t;

// Color structure
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} RGB_Color_t;

/*******************************************************************************
 * COLOR TABLE
 ******************************************************************************/
const RGB_Color_t color_table[5] = {
    {247, 255, 132},  // Color 02: #f7ff84
    {238, 101, 255},  // Color 11: #ee65ff
    {151,  38, 255},  // Color 05: #9726ff
    { 53,  66, 255},  // Color 13: #3542ff
    {255,  33, 108}   // Color 01: #ff216c
};

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/
static volatile OperatingMode_t current_mode = MODE_IDLE;
static volatile uint8_t current_brightness = 50;  // 50% initial brightness
static volatile uint8_t current_color_index = 0;
static volatile uint16_t cycle_period_ms = 400;
static volatile CycleDirection_t cycle_direction = DIR_FORWARD;
static volatile bool cycle_frozen = false;
static volatile uint32_t last_cycle_time = 0;

// ADC filtering variables
static uint16_t adc_history[4] = {0};
static uint8_t adc_history_index = 0;

/*******************************************************************************
 * FUNCTION PROTOTYPES
 ******************************************************************************/
void RGB_SetColor(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness);
void RGB_SetBrightness(uint16_t adc_value);
void RGB_SetSpeedAndDirection(uint16_t adc_value);
uint16_t ADC_ReadFiltered(void);
void UpdateCurrentColor(void);

/*******************************************************************************
 * ADC FUNCTIONS
 ******************************************************************************/
void ADC_Initialize(void) {
    ADC_REGS->ADC_CTRLA |= (1 << 0);
    while ((ADC_REGS->ADC_SYNCBUSY & (1 << 0)) == (1 << 0));

    ADC_REGS->ADC_CTRLB |= (2 << 0);
    ADC_REGS->ADC_SAMPCTRL |= (3 << 0);
    ADC_REGS->ADC_REFCTRL |= (0x5 << 0);
    ADC_REGS->ADC_INPUTCTRL |= (0xA << 0);
    ADC_REGS->ADC_CTRLC = (uint16_t)((0x2 << 4) | (0 << 8));
    ADC_REGS->ADC_INTFLAG |= (uint8_t)0x07;
    
    while (0U != ADC_REGS->ADC_SYNCBUSY);
}

void ADC_Enable(void) {
    ADC_REGS->ADC_CTRLA |= (1 << 1);
    while (0U != ADC_REGS->ADC_SYNCBUSY);
}

void ADC_StartConversion(void) {
    ADC_REGS->ADC_SWTRIG |= (1 << 1);
    while ((ADC_REGS->ADC_SYNCBUSY & (1 << 10)) == (1 << 10));
}

uint16_t ADC_GetResult(void) {
    return (uint16_t)ADC_REGS->ADC_RESULT;
}

bool ADC_IsConversionReady(void) {
    bool status = (((ADC_REGS->ADC_INTFLAG & (1 << 0)) >> 0) != 0U);
    if (status == true) {
        ADC_REGS->ADC_INTFLAG |= (1 << 0);
    }
    return status;
}

/*******************************************************************************
 * CLOCK FUNCTIONS
 ******************************************************************************/
void GenericClock_Initialize(void) {
    GCLK_REGS->GCLK_PCHCTRL[27] = 0x00000040;
    while ((GCLK_REGS->GCLK_PCHCTRL[27] & 0x00000040) == 0);
}

void SystemClock_Configure(void) {
    PM_REGS->PM_INTFLAG = 0x01;
    PM_REGS->PM_PLCFG = 0x02;
    while ((PM_REGS->PM_INTFLAG & 0x01) == 0)
        asm("nop");
    PM_REGS->PM_INTFLAG = 0x01;

    NVMCTRL_SEC_REGS->NVMCTRL_CTRLB = (2 << 1);
    SUPC_REGS->SUPC_VREGPLL = (1 << 1);

    OSCCTRL_REGS->OSCCTRL_DFLLCTRL = 0;
    while ((OSCCTRL_REGS->OSCCTRL_STATUS & (1 << 24)) != (1 << 24));

    uint8_t calibCoarse = (uint8_t)((((uint32_t)0x00806020U) >> 25U) & 0x3fU);
    OSCCTRL_REGS->OSCCTRL_DFLLVAL = OSCCTRL_DFLLVAL_COARSE((uint32_t)calibCoarse) | 
                                     OSCCTRL_DFLLVAL_FINE((uint32_t)512U);
    while ((OSCCTRL_REGS->OSCCTRL_STATUS & (1 << 24)) != (1 << 24));

    OSCCTRL_REGS->OSCCTRL_DFLLCTRL = (1 << 1);
    while ((OSCCTRL_REGS->OSCCTRL_STATUS & (1 << 24)) != (1 << 24));

    GCLK_REGS->GCLK_GENCTRL[0] |= (1 << 16) | (7 << 0) | (1 << 8);
    while (GCLK_REGS->GCLK_SYNCBUSY & (1 << 2))
        asm("nop");

    GCLK_REGS->GCLK_PCHCTRL[28] |= (0 << 0) | (1 << 6);
    while ((GCLK_REGS->GCLK_PCHCTRL[28] & (1 << 6)) != (1 << 6));
}

/*******************************************************************************
 * TIMER FUNCTIONS
 ******************************************************************************/
void PWMTimer_Initialize(void) {
    TCC3_REGS->TCC_CTRLA = 0x01;
    while (TCC3_REGS->TCC_SYNCBUSY & ~(1 << 0));

    TCC3_REGS->TCC_CTRLA = (1 << 12) | (7 << 8);
    TCC3_REGS->TCC_WEXCTRL = TCC_WEXCTRL_OTMX(0UL);
    TCC3_REGS->TCC_WAVE = (2 << 0) | (0 << 4) | (1 << 17) | (1 << 16) | (1 << 19);
    TCC3_REGS->TCC_PER = PWM_PERIOD;

    TCC3_REGS->TCC_CTRLA |= (1 << 1);
    while (TCC3_REGS->TCC_SYNCBUSY & ~(1 << 1));
    while (TCC3_REGS->TCC_SYNCBUSY & ~(1 << 8));
    while (TCC3_REGS->TCC_SYNCBUSY & ~(1 << 9));
    while (TCC3_REGS->TCC_SYNCBUSY & ~(1 << 11));
}

void DelayTimer_Initialize(void) {
    GCLK_REGS->GCLK_PCHCTRL[23] |= (1 << 6);
    while ((GCLK_REGS->GCLK_PCHCTRL[23] & (1 << 6)) == 0);

    TC0_REGS->COUNT16.TC_CTRLA |= (1);
    while (TC0_REGS->COUNT16.TC_SYNCBUSY & (1));

    TC0_REGS->COUNT16.TC_CTRLA |= (0x0 << 2);
    TC0_REGS->COUNT16.TC_CTRLA |= (0x1 << 4);
    TC0_REGS->COUNT16.TC_CTRLA |= (0x7 << 8);
    TC0_REGS->COUNT16.TC_WAVE |= (0x1 << 0);
    TC0_REGS->COUNT16.TC_CC[0] |= (0x32C8);

    TC0_REGS->COUNT16.TC_CTRLA |= (1 << 1);
    while (TC0_REGS->COUNT16.TC_SYNCBUSY & (1 << 1));
}

void DelayTimer_Wait(void) {
    TC0_REGS->COUNT16.TC_INTFLAG = (1 << 4);
    TC0_REGS->COUNT16.TC_CTRLBSET = (1 << 0);
    while (TC0_REGS->COUNT16.TC_SYNCBUSY & (1 << 0));

    while (!(TC0_REGS->COUNT16.TC_INTFLAG & (1 << 4)));

    TC0_REGS->COUNT16.TC_INTFLAG = (1 << 4);
}

/*******************************************************************************
 * PORT INITIALIZATION
 ******************************************************************************/
void Potentiometer_Initialize(void) {
    PORT_SEC_REGS->GROUP[1].PORT_DIRCLR |= (1 << 2);
    PORT_SEC_REGS->GROUP[1].PORT_OUTSET |= (1 << 2);
    PORT_SEC_REGS->GROUP[1].PORT_PINCFG[2] |= (0x3 << 0);
    PORT_SEC_REGS->GROUP[1].PORT_PMUX[1] |= (0x1 << 0);
}

void RedLED_Initialize(void) {
    PORT_SEC_REGS->GROUP[0].PORT_DIRSET |= (1 << 3);
    PORT_SEC_REGS->GROUP[0].PORT_OUTCLR |= (1 << 3);
    PORT_SEC_REGS->GROUP[0].PORT_PINCFG[3] |= 0x3;
    PORT_SEC_REGS->GROUP[0].PORT_PMUX[1] |= (0x9 << 4);
}

void GreenLED_Initialize(void) {
    PORT_SEC_REGS->GROUP[0].PORT_DIRSET |= (1 << 6);
    PORT_SEC_REGS->GROUP[0].PORT_OUTCLR |= (1 << 6);
    PORT_SEC_REGS->GROUP[0].PORT_PINCFG[6] |= 0x3;
    PORT_SEC_REGS->GROUP[0].PORT_PMUX[3] |= (0x9 << 0);
}

void BlueLED_Initialize(void) {
    PORT_SEC_REGS->GROUP[1].PORT_DIRSET |= (1 << 3);
    PORT_SEC_REGS->GROUP[1].PORT_OUTCLR |= (1 << 3);
    PORT_SEC_REGS->GROUP[1].PORT_PINCFG[3] |= 0x3;
    PORT_SEC_REGS->GROUP[1].PORT_PMUX[1] |= (0x9 << 4);
}

void Switch1_Initialize(void) {
    PORT_SEC_REGS->GROUP[0].PORT_DIRCLR |= (1 << 0);
    PORT_SEC_REGS->GROUP[0].PORT_PINCFG[0] |= 0x7;
    PORT_SEC_REGS->GROUP[0].PORT_OUTSET |= (1 << 0);
    PORT_SEC_REGS->GROUP[0].PORT_PMUX[0] |= (0x0 << 0);
}

void Switch2_Initialize(void) {
    PORT_SEC_REGS->GROUP[0].PORT_DIRCLR |= (1 << 1);
    PORT_SEC_REGS->GROUP[0].PORT_PINCFG[1] |= 0x7;
    PORT_SEC_REGS->GROUP[0].PORT_OUTSET |= (1 << 1);
    PORT_SEC_REGS->GROUP[0].PORT_PMUX[0] |= (0x0 << 4);
}

/*******************************************************************************
 * EXTERNAL INTERRUPT
 ******************************************************************************/
void ExternalInterrupt_Initialize(void) {
    Switches_Initialize();

    EIC_SEC_REGS->EIC_CTRLA |= 0x01;
    while ((EIC_SEC_REGS->EIC_SYNCBUSY & 0x01) == 0x01);

    EIC_SEC_REGS->EIC_CONFIG0 |= (0x2 << 0);
    EIC_SEC_REGS->EIC_CONFIG0 |= (1 << 3);
    EIC_SEC_REGS->EIC_CONFIG0 |= (0x2 << 4);
    EIC_SEC_REGS->EIC_CONFIG0 |= (1 << 7);

    EIC_SEC_REGS->EIC_DEBOUNCEN |= (1 << 0);
    EIC_SEC_REGS->EIC_DEBOUNCEN |= (1 << 1);
    EIC_SEC_REGS->EIC_DPRESCALER |= 0x000100FF;

    EIC_SEC_REGS->EIC_INTENSET |= (1 << 0);
    EIC_SEC_REGS->EIC_INTENSET |= (1 << 1);

    EIC_SEC_REGS->EIC_CTRLA |= 0x02;
    while ((EIC_SEC_REGS->EIC_SYNCBUSY & 0x02) == 0x02);

    EIC_SEC_REGS->EIC_INTFLAG |= (1 << 0);
    EIC_SEC_REGS->EIC_INTFLAG |= (1 << 1);
}

void InterruptController_Initialize(void) {
    __DMB();
    __enable_irq();

    NVIC_SetPriority(EIC_EXTINT_0_IRQn, 2);
    NVIC_EnableIRQ(EIC_EXTINT_0_IRQn);

    NVIC_SetPriority(EIC_EXTINT_1_IRQn, 2);
    NVIC_EnableIRQ(EIC_EXTINT_1_IRQn);
}

/*******************************************************************************
 * SYSTEM INITIALIZATION
 ******************************************************************************/
void System_Initialize(void) {
    Clock_Initialize();
    Ports_Initialize();
    ExternalInterrupt_Start();
}

void Clock_Initialize(void) {
    GenericClock_Initialize();
    SystemClock_Configure();
    ADC_Initialize();
    ADC_Enable();
    DelayTimer_Initialize();
    PWMTimer_Initialize();
}

void Ports_Initialize(void) {
    RedLED_Initialize();
    GreenLED_Initialize();
    BlueLED_Initialize();
    Potentiometer_Initialize();
}

void Switches_Initialize(void) {
    Switch1_Initialize();
    Switch2_Initialize();
}

void ExternalInterrupt_Start(void) {
    MCLK_REGS->MCLK_APBAMASK |= MCLK_APBAMASK_EIC_Msk;
    GCLK_REGS->GCLK_PCHCTRL[4] = 0x00000040;
    ExternalInterrupt_Initialize();
    InterruptController_Initialize();
}

/*******************************************************************************
 * RGB LED CONTROL FUNCTIONS
 ******************************************************************************/

/**
 * @brief Set RGB LED color with brightness scaling
 * @param r Red value (0-255)
 * @param g Green value (0-255)
 * @param b Blue value (0-255)
 * @param brightness Brightness percentage (0-100)
 */
void RGB_SetColor(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness) {
    uint16_t r_scaled, g_scaled, b_scaled;
    
    // Apply brightness scaling (0-100% brightness)
    r_scaled = ((uint16_t)r * brightness) / 100;
    g_scaled = ((uint16_t)g * brightness) / 100;
    b_scaled = ((uint16_t)b * brightness) / 100;
    
    // Scale to PWM period (0-255 -> 0-PWM_PERIOD)
    r_scaled = (r_scaled * PWM_PERIOD) / 255;
    g_scaled = (g_scaled * PWM_PERIOD) / 255;
    b_scaled = (b_scaled * PWM_PERIOD) / 255;
    
    // Invert for common anode (0% duty = full brightness, 100% duty = OFF)
    r_scaled = PWM_PERIOD - r_scaled;
    g_scaled = PWM_PERIOD - g_scaled;
    b_scaled = PWM_PERIOD - b_scaled;
    
    // Set PWM duty cycles
    TCC3_REGS->TCC_CC[0] = r_scaled;  // Red (PA03)
    TCC3_REGS->TCC_CC[1] = g_scaled;  // Green (PA06)
    TCC3_REGS->TCC_CC[2] = b_scaled;  // Blue (PB03)
    
    // Wait for synchronization
    while (TCC3_REGS->TCC_SYNCBUSY & (TCC_SYNCBUSY_CC0_Msk | TCC_SYNCBUSY_CC1_Msk | TCC_SYNCBUSY_CC2_Msk));
}

/**
 * @brief Update the current color with current brightness
 */
void UpdateCurrentColor(void) {
    const RGB_Color_t *color = &color_table[current_color_index];
    RGB_SetColor(color->r, color->g, color->b, current_brightness);
}

/**
 * @brief Read ADC with filtering (moving average)
 * @return Filtered ADC value (0-1023)
 */
uint16_t ADC_ReadFiltered(void) {
    uint32_t sum = 0;
    uint8_t i;
    
    // Start ADC conversion
    ADC_StartConversion();
    
    // Wait for conversion to complete
    while (!ADC_IsConversionReady());
    
    // Get result and add to history
    adc_history[adc_history_index] = ADC_GetResult();
    adc_history_index = (adc_history_index + 1) % 4;
    
    // Calculate moving average
    for (i = 0; i < 4; i++) {
        sum += adc_history[i];
    }
    
    return (uint16_t)(sum / 4);
}

/**
 * @brief Set brightness based on potentiometer reading (SW1 mode)
 * @param adc_value ADC reading (0-1023)
 */
void RGB_SetBrightness(uint16_t adc_value) {
    // Map 10-bit ADC (0-1023) to brightness (0-100%)
    // Apply hysteresis: ignore changes < 10 counts
    uint8_t new_brightness = (uint8_t)((adc_value * 100UL) / 1023);
    
    // Only update if change is significant (>= 1%)
    if (abs((int)new_brightness - (int)current_brightness) >= 1) {
        current_brightness = new_brightness;
        UpdateCurrentColor();
    }
}

/**
 * @brief Set speed and direction based on potentiometer reading (SW2 mode)
 * @param adc_value ADC reading (0-1023)
 */
void RGB_SetSpeedAndDirection(uint16_t adc_value) {
    // Calculate potentiometer percentage (0-100)
    uint8_t pot_percent = (uint8_t)((adc_value * 100UL) / 1023);
    
    // Determine period and direction based on potentiometer zones
    if (pot_percent <= 20) {
        // [0,20]: 400ms Reverse (Frozen)
        cycle_period_ms = 400;
        cycle_direction = DIR_REVERSE;
        cycle_frozen = true;
    } else if (pot_percent <= 40) {
        // (20,40]: 800ms Reverse
        cycle_period_ms = 800;
        cycle_direction = DIR_REVERSE;
        cycle_frozen = false;
    } else if (pot_percent <= 60) {
        // (40,60]: 800ms Forward
        cycle_period_ms = 800;
        cycle_direction = DIR_FORWARD;
        cycle_frozen = false;
    } else if (pot_percent < 80) {
        // [60,80): 400ms Forward
        cycle_period_ms = 400;
        cycle_direction = DIR_FORWARD;
        cycle_frozen = false;
    } else {
        // [80,100]: 400ms Forward
        cycle_period_ms = 400;
        cycle_direction = DIR_FORWARD;
        cycle_frozen = false;
    }
}

/*******************************************************************************
 * INTERRUPT HANDLERS
 ******************************************************************************/

/**
 * @brief SW1 interrupt handler - Brightness control mode
 */
void EIC_EXTINT_0_Handler(void) {
    // Clear interrupt flag
    EIC_SEC_REGS->EIC_INTFLAG |= (1 << 0);
    
    // Switch to brightness control mode
    current_mode = MODE_BRIGHTNESS;
}

/**
 * @brief SW2 interrupt handler - Auto-cycle mode
 */
void EIC_EXTINT_1_Handler(void) {
    // Clear interrupt flag
    EIC_SEC_REGS->EIC_INTFLAG |= (1 << 1);
    
    // Switch to auto-cycle mode
    current_mode = MODE_AUTO_CYCLE;
    last_cycle_time = 0;  // Reset cycle timer
}

/*******************************************************************************
 * MAIN FUNCTION
 ******************************************************************************/
int main(void) {
    uint32_t cycle_counter = 0;
    uint16_t adc_value;
    
    // Initialize system
    System_Initialize();
    
    // Initialize ADC history buffer
    for (uint8_t i = 0; i < 4; i++) {
        adc_history[i] = 512;  // Initialize to mid-range
    }
    
    // Display first color at 50% brightness on reset
    current_color_index = 0;
    current_brightness = 50;
    UpdateCurrentColor();
    
    // Main loop
    while (1) {
        // Read potentiometer with filtering
        adc_value = ADC_ReadFiltered();
        
        switch (current_mode) {
            case MODE_IDLE:
                // Do nothing, wait for button press
                DelayTimer_Wait();  // 100ms delay
                break;
                
            case MODE_BRIGHTNESS:
                // Update brightness based on potentiometer
                RGB_SetBrightness(adc_value);
                DelayTimer_Wait();  // 100ms delay for responsiveness
                break;
                
            case MODE_AUTO_CYCLE:
                // Update speed and direction based on potentiometer
                RGB_SetSpeedAndDirection(adc_value);
                
                // Auto-cycle colors based on period
                if (!cycle_frozen) {
                    if (cycle_counter >= cycle_period_ms / 100) {
                        // Time to change color
                        if (cycle_direction == DIR_FORWARD) {
                            current_color_index = (current_color_index + 1) % 5;
                        } else {
                            current_color_index = (current_color_index == 0) ? 4 : current_color_index - 1;
                        }
                        UpdateCurrentColor();
                        cycle_counter = 0;
                    } else {
                        cycle_counter++;
                    }
                }
                
                DelayTimer_Wait();  // 100ms delay
                break;
        }
    }

    return 0;
}
