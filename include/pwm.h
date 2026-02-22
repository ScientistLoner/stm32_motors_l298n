#ifndef PWM_H
#define PWM_H

#include <stdint.h>

/* TIM registers structure */
typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t SMCR;
    volatile uint32_t DIER;
    volatile uint32_t SR;
    volatile uint32_t EGR;
    volatile uint32_t CCMR1;
    volatile uint32_t CCMR2;
    volatile uint32_t CCER;
    volatile uint32_t CNT;
    volatile uint32_t PSC;
    volatile uint32_t ARR;
    volatile uint32_t RESERVED1;
    volatile uint32_t CCR1;
    volatile uint32_t CCR2;
    volatile uint32_t CCR3;
    volatile uint32_t CCR4;
    volatile uint32_t RESERVED2;
    volatile uint32_t DCR;
    volatile uint32_t DMAR;
} TIM_TypeDef;

/* Base addresses */
#define TIM1_BASE       ((uint32_t)0x40012C00)
#define TIM2_BASE       ((uint32_t)0x40000000)
#define TIM3_BASE       ((uint32_t)0x40000400)
#define TIM4_BASE       ((uint32_t)0x40000800)

/* Peripheral pointers */
#define TIM1            ((TIM_TypeDef*)TIM1_BASE)
#define TIM2            ((TIM_TypeDef*)TIM2_BASE)
#define TIM3            ((TIM_TypeDef*)TIM3_BASE)
#define TIM4            ((TIM_TypeDef*)TIM4_BASE)

/* TIM CR1 bits */
#define TIM_CR1_CEN     (1 << 0)    /* Counter enable */
#define TIM_CR1_ARPE    (1 << 7)    /* Auto-reload preload enable */

/* TIM CCMR1/CCMR2 bits */
#define TIM_CCMR_OC_MODE_PWM1   (6 << 4)    /* PWM mode 1 */
#define TIM_CCMR_OC_PRELOAD    (1 << 3)    /* Output compare preload enable */

/* TIM CCER bits */
#define TIM_CCER_CC1E   (1 << 0)    /* Capture/Compare 1 output enable */
#define TIM_CCER_CC2E   (1 << 4)    /* Capture/Compare 2 output enable */
#define TIM_CCER_CC3E   (1 << 8)    /* Capture/Compare 3 output enable */
#define TIM_CCER_CC4E   (1 << 12)   /* Capture/Compare 4 output enable */

/* PWM channels */
typedef enum {
    PWM_CH1 = 0,
    PWM_CH2,
    PWM_CH3,
    PWM_CH4
} PWM_Channel;

/* PWM timers */
typedef enum {
    PWM_TIM2 = 0,
    PWM_TIM3,
    PWM_TIM4
} PWM_Timer;

/* Function prototypes */
void PWM_Init(PWM_Timer timer, uint32_t frequency);
void PWM_SetDuty(PWM_Timer timer, PWM_Channel channel, uint8_t duty);
uint8_t PWM_GetDuty(PWM_Timer timer, PWM_Channel channel);
void PWM_Start(PWM_Timer timer);
void PWM_Stop(PWM_Timer timer);
uint8_t PWM_IsRunning(PWM_Timer timer);

#endif /* PWM_H */