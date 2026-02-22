#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>

/* RCC registers */
typedef struct {
    volatile uint32_t CR;
    volatile uint32_t CFGR;
    volatile uint32_t CIR;
    volatile uint32_t APB2RSTR;
    volatile uint32_t APB1RSTR;
    volatile uint32_t AHBENR;
    volatile uint32_t APB2ENR;
    volatile uint32_t APB1ENR;
    volatile uint32_t BDCR;
    volatile uint32_t CSR;
} RCC_TypeDef;

/* FLASH registers */
typedef struct {
    volatile uint32_t ACR;
    volatile uint32_t KEYR;
    volatile uint32_t OPTKEYR;
    volatile uint32_t SR;
    volatile uint32_t CR;
    volatile uint32_t AR;
    volatile uint32_t RESERVED;
    volatile uint32_t OBR;
    volatile uint32_t WRPR;
} FLASH_TypeDef;

/* SysTick registers */
typedef struct {
    volatile uint32_t CTRL;
    volatile uint32_t LOAD;
    volatile uint32_t VAL;
    volatile uint32_t CALIB;
} SysTick_TypeDef;

/* Base addresses */
#define RCC_BASE        ((uint32_t)0x40021000)
#define FLASH_BASE      ((uint32_t)0x40022000)
#define SYS_TICK_BASE   ((uint32_t)0xE000E010)

/* Peripheral pointers */
#define RCC             ((RCC_TypeDef*)RCC_BASE)
#define FLASH           ((FLASH_TypeDef*)FLASH_BASE)
#define SysTick         ((SysTick_TypeDef*)SYS_TICK_BASE)

/* RCC APB2ENR bits */
#define RCC_APB2ENR_AFIOEN  (1 << 0)
#define RCC_APB2ENR_IOPAEN  (1 << 2)
#define RCC_APB2ENR_IOPBEN  (1 << 3)
#define RCC_APB2ENR_IOPCEN  (1 << 4)
#define RCC_APB2ENR_ADC1EN  (1 << 9)
#define RCC_APB2ENR_TIM1EN  (1 << 11)
#define RCC_APB2ENR_SPI1EN  (1 << 12)
#define RCC_APB2ENR_TIM8EN  (1 << 13)
#define RCC_APB2ENR_USART1EN (1 << 14)
#define RCC_APB2ENR_TIM9EN  (1 << 19)
#define RCC_APB2ENR_TIM10EN (1 << 20)
#define RCC_APB2ENR_TIM11EN (1 << 21)

/* RCC APB1ENR bits */
#define RCC_APB1ENR_TIM2EN  (1 << 0)
#define RCC_APB1ENR_TIM3EN  (1 << 1)
#define RCC_APB1ENR_TIM4EN  (1 << 2)
#define RCC_APB1ENR_TIM5EN  (1 << 3)
#define RCC_APB1ENR_TIM6EN  (1 << 4)
#define RCC_APB1ENR_TIM7EN  (1 << 5)
#define RCC_APB1ENR_WWDGEN  (1 << 11)
#define RCC_APB1ENR_SPI2EN  (1 << 14)
#define RCC_APB1ENR_SPI3EN  (1 << 15)
#define RCC_APB1ENR_USART2EN (1 << 17)
#define RCC_APB1ENR_USART3EN (1 << 18)
#define RCC_APB1ENR_I2C1EN  (1 << 21)
#define RCC_APB1ENR_I2C2EN  (1 << 22)
#define RCC_APB1ENR_USBEN   (1 << 23)
#define RCC_APB1ENR_CAN1EN  (1 << 25)
#define RCC_APB1ENR_CAN2EN  (1 << 26)
#define RCC_APB1ENR_BKPEN   (1 << 27)
#define RCC_APB1ENR_PWREN   (1 << 28)
#define RCC_APB1ENR_DACEN   (1 << 29)

/* System clock frequency */
#define SYSTEM_CLOCK_FREQ    72000000  /* 72 MHz */

/* Function prototypes */
void System_Init(void);
void delay_ms(uint32_t ms);
void delay_us(uint32_t us);
uint32_t get_tick(void);
void SysTick_Handler(void);

extern volatile uint32_t system_tick;

#endif /* SYSTEM_H */