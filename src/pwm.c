#include "pwm.h"
#include "gpio.h"
#include "system.h"

/* Структура для конфигурации таймеров */
typedef struct {
    TIM_TypeDef* timer;
    uint32_t rcc_bit;
    uint8_t alt_func;
    uint32_t clock_freq;  /* Добавили частоту таймера */
} TimerConfig;

/* Конфигурация таймеров */
static const TimerConfig timer_config[] = {
    [PWM_TIM2] = {TIM2, RCC_APB1ENR_TIM2EN, 1, 72000000},  /* 72 MHz если APB1 prescaler = 1 */
    [PWM_TIM3] = {TIM3, RCC_APB1ENR_TIM3EN, 2, 72000000},  /* 72 MHz если APB1 prescaler = 1 */
    [PWM_TIM4] = {TIM4, RCC_APB1ENR_TIM4EN, 2, 72000000}   /* 72 MHz если APB1 prescaler = 1 */
};

/* Настройка выводов для PWM */
static void pwm_configure_pins(PWM_Timer timer, PWM_Channel channel) {
    switch(timer) {
        case PWM_TIM2:
            switch(channel) {
                case PWM_CH1:  /* PA0/PA15 или PB8 */
                    /* Используем PA0 (TIM2_CH1) */
                    GPIO_ConfigPin(GPIOA, 0, GPIO_MODE_AF_PP);
                    break;
                case PWM_CH2:  /* PA1 или PB9 */
                    GPIO_ConfigPin(GPIOA, 1, GPIO_MODE_AF_PP);
                    break;
                case PWM_CH3:  /* PA2 или PB10 */
                    GPIO_ConfigPin(GPIOA, 2, GPIO_MODE_AF_PP);
                    break;
                case PWM_CH4:  /* PA3 или PB11 */
                    GPIO_ConfigPin(GPIOA, 3, GPIO_MODE_AF_PP);
                    break;
            }
            break;
            
        case PWM_TIM3:
            switch(channel) {
                case PWM_CH1:  /* PA6 или PB4 или PC6 */
                    /* Используем PA6 (TIM3_CH1) для ENA */
                    GPIO_ConfigPin(GPIOA, 6, GPIO_MODE_AF_PP);
                    break;
                case PWM_CH2:  /* PA7 или PB5 или PC7 */
                    /* Используем PA7 (TIM3_CH2) для ENB */
                    GPIO_ConfigPin(GPIOA, 7, GPIO_MODE_AF_PP);
                    break;
                case PWM_CH3:  /* PB0 или PC8 */
                    /* Используем PB0 (TIM3_CH3) для IN1 */
                    GPIO_ConfigPin(GPIOB, 0, GPIO_MODE_AF_PP);
                    break;
                case PWM_CH4:  /* PB1 или PC9 */
                    /* Используем PB1 (TIM3_CH4) для IN2 */
                    GPIO_ConfigPin(GPIOB, 1, GPIO_MODE_AF_PP);
                    break;
            }
            break;
            
        case PWM_TIM4:
            switch(channel) {
                case PWM_CH1:  /* PB6 или PD12 */
                    /* Используем PB6 (TIM4_CH1) для IN3 */
                    GPIO_ConfigPin(GPIOB, 6, GPIO_MODE_AF_PP);
                    break;
                case PWM_CH2:  /* PB7 или PD13 */
                    /* Используем PB7 (TIM4_CH2) для IN4 */
                    GPIO_ConfigPin(GPIOB, 7, GPIO_MODE_AF_PP);
                    break;
                case PWM_CH3:  /* PB8 или PD14 */
                    GPIO_ConfigPin(GPIOB, 8, GPIO_MODE_AF_PP);
                    break;
                case PWM_CH4:  /* PB9 или PD15 */
                    GPIO_ConfigPin(GPIOB, 9, GPIO_MODE_AF_PP);
                    break;
            }
            break;
    }
}

/* Инициализация PWM таймера */
void PWM_Init(PWM_Timer timer, uint32_t frequency) {
    TIM_TypeDef* TIMx = timer_config[timer].timer;
    uint32_t timer_clock = timer_config[timer].clock_freq;
    uint32_t prescaler, period;
    
    /* Включаем тактирование таймера */
    if(timer == PWM_TIM2 || timer == PWM_TIM3 || timer == PWM_TIM4) {
        RCC->APB1ENR |= timer_config[timer].rcc_bit;
    }
    
    /* Сбрасываем настройки таймера */
    TIMx->CR1 = 0;
    TIMx->CR2 = 0;
    
    /* Настраиваем предделитель и период */
    /* Для частоты 20 kHz и разрешения 1000 шагов (0.1%): */
    /* period = timer_clock / frequency */
    /* Но нам нужно разрешение 1000, так что: */
    /* period = 1000 - 1 для 0.1% разрешения */
    /* prescaler = timer_clock / (frequency * period) - 1 */
    
    /* Упрощенный расчет: фиксированное разрешение 1000 */
    period = 1000 - 1;  /* Разрешение 0.1% (0-1000) */
    
    /* Расчет предделителя с проверкой на переполнение */
    uint32_t required_clock = frequency * (period + 1);
    if (required_clock == 0 || required_clock > timer_clock) {
        /* Если не можем достичь нужной частоты, используем максимальную возможную */
        prescaler = 0;
    } else {
        prescaler = (timer_clock / required_clock) - 1;
        if (prescaler > 0xFFFF) {
            prescaler = 0xFFFF;  /* Максимальное значение предделителя */
        }
    }
    
    /* Реальная частота после настройки */
    uint32_t actual_freq = timer_clock / ((prescaler + 1) * (period + 1));
    
    /* Настройка предделителя и периода */
    TIMx->PSC = prescaler;
    TIMx->ARR = period;
    
    /* Включаем режим предзагрузки регистров */
    TIMx->CR1 |= TIM_CR1_ARPE;
    
    /* Настраиваем каналы как PWM выходы */
    switch(timer) {
        case PWM_TIM3:
            /* Каналы 1 и 2 для скорости моторов */
            TIMx->CCMR1 |= (TIM_CCMR_OC_MODE_PWM1 << 8) |  /* CH2 */
                          (TIM_CCMR_OC_MODE_PWM1 << 0);   /* CH1 */
            TIMx->CCMR1 |= (TIM_CCMR_OC_PRELOAD << 8) |
                          (TIM_CCMR_OC_PRELOAD << 0);
            TIMx->CCMR2 |= (TIM_CCMR_OC_MODE_PWM1 << 8) |  /* CH4 */
                          (TIM_CCMR_OC_MODE_PWM1 << 0);   /* CH3 */
            TIMx->CCMR2 |= (TIM_CCMR_OC_PRELOAD << 8) |
                          (TIM_CCMR_OC_PRELOAD << 0);
            
            /* Настраиваем выводы для всех каналов TIM3 */
            pwm_configure_pins(PWM_TIM3, PWM_CH1);
            pwm_configure_pins(PWM_TIM3, PWM_CH2);
            pwm_configure_pins(PWM_TIM3, PWM_CH3);
            pwm_configure_pins(PWM_TIM3, PWM_CH4);
            break;
            
        case PWM_TIM4:
            /* Каналы 1 и 2 для управления направлением */
            TIMx->CCMR1 |= (TIM_CCMR_OC_MODE_PWM1 << 8) |  /* CH2 */
                          (TIM_CCMR_OC_MODE_PWM1 << 0);   /* CH1 */
            TIMx->CCMR1 |= (TIM_CCMR_OC_PRELOAD << 8) |
                          (TIM_CCMR_OC_PRELOAD << 0);
            
            /* Настраиваем выводы для каналов TIM4 */
            pwm_configure_pins(PWM_TIM4, PWM_CH1);
            pwm_configure_pins(PWM_TIM4, PWM_CH2);
            break;
            
        case PWM_TIM2:
            /* Может использоваться для дополнительных функций */
            TIMx->CCMR1 |= (TIM_CCMR_OC_MODE_PWM1 << 0);   /* CH1 */
            TIMx->CCMR1 |= (TIM_CCMR_OC_PRELOAD << 0);
            pwm_configure_pins(PWM_TIM2, PWM_CH1);
            break;
    }
    
    /* Включаем выходы каналов */
    switch(timer) {
        case PWM_TIM3:
            TIMx->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E | 
                         TIM_CCER_CC3E | TIM_CCER_CC4E;
            break;
        case PWM_TIM4:
            TIMx->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E;
            break;
        case PWM_TIM2:
            TIMx->CCER |= TIM_CCER_CC1E;
            break;
    }
    
    /* Устанавливаем начальный коэффициент заполнения 0% */
    switch(timer) {
        case PWM_TIM3:
            TIMx->CCR1 = 0;  /* ENA */
            TIMx->CCR2 = 0;  /* ENB */
            TIMx->CCR3 = 0;  /* IN1 */
            TIMx->CCR4 = 0;  /* IN2 */
            break;
        case PWM_TIM4:
            TIMx->CCR1 = 0;  /* IN3 */
            TIMx->CCR2 = 0;  /* IN4 */
            break;
        case PWM_TIM2:
            TIMx->CCR1 = 0;
            break;
    }
}

/* Установка коэффициента заполнения PWM (0-100%) */
void PWM_SetDuty(PWM_Timer timer, PWM_Channel channel, uint8_t duty) {
    TIM_TypeDef* TIMx = timer_config[timer].timer;
    uint32_t duty_value;
    
    /* Преобразуем 0-100% в 0-1000 (разрешение 0.1%) */
    if (duty > 100) duty = 100;
    duty_value = (duty * 10);  /* 100% = 1000 */
    
    switch(channel) {
        case PWM_CH1:
            TIMx->CCR1 = duty_value;
            break;
        case PWM_CH2:
            TIMx->CCR2 = duty_value;
            break;
        case PWM_CH3:
            TIMx->CCR3 = duty_value;
            break;
        case PWM_CH4:
            TIMx->CCR4 = duty_value;
            break;
    }
}

/* Получение текущего коэффициента заполнения (0-100%) */
uint8_t PWM_GetDuty(PWM_Timer timer, PWM_Channel channel) {
    TIM_TypeDef* TIMx = timer_config[timer].timer;
    uint32_t duty_value;
    
    switch(channel) {
        case PWM_CH1:
            duty_value = TIMx->CCR1;
            break;
        case PWM_CH2:
            duty_value = TIMx->CCR2;
            break;
        case PWM_CH3:
            duty_value = TIMx->CCR3;
            break;
        case PWM_CH4:
            duty_value = TIMx->CCR4;
            break;
        default:
            return 0;
    }
    
    /* Преобразуем 0-1000 в 0-100% */
    return (uint8_t)(duty_value / 10);
}

/* Запуск PWM */
void PWM_Start(PWM_Timer timer) {
    TIM_TypeDef* TIMx = timer_config[timer].timer;
    TIMx->CR1 |= TIM_CR1_CEN;
}

/* Остановка PWM */
void PWM_Stop(PWM_Timer timer) {
    TIM_TypeDef* TIMx = timer_config[timer].timer;
    TIMx->CR1 &= ~TIM_CR1_CEN;
}

/* Проверка, работает ли PWM */
uint8_t PWM_IsRunning(PWM_Timer timer) {
    TIM_TypeDef* TIMx = timer_config[timer].timer;
    return (TIMx->CR1 & TIM_CR1_CEN) ? 1 : 0;
}