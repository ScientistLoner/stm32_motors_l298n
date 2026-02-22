#include "system.h"

/* Глобальная переменная для системного тика */
volatile uint32_t system_tick = 0;

/* Простая задержка в циклах */
static void delay_cycles(uint32_t cycles) {
    for(volatile uint32_t i = 0; i < cycles; i++) {
        __asm__("nop");
    }
}

/* Инициализация системы и тактирования */
void System_Init(void) {
    /* 1. Настраиваем Flash latency для 72 MHz */
    FLASH->ACR = FLASH->ACR & ~(0x0F);  /* Очищаем биты latency */
    FLASH->ACR |= 0x02;                 /* Two wait states для 48-72 MHz */
    
    /* 2. Включаем HSE и ждем его готовности */
    RCC->CR |= (1 << 16);               /* HSEON */
    while(!(RCC->CR & (1 << 17)));      /* Ждем HSERDY */
    
    /* 3. Настраиваем PLL */
    /* PLL multiplication factor = 9, источник HSE (8MHz) */
    /* 8MHz * 9 = 72MHz */
    RCC->CFGR &= ~(0xF << 18);          /* Очищаем PLLMUL */
    RCC->CFGR |= (7 << 18);             /* PLLMUL = 0111 (x9) */
    
    /* Источник PLL = HSE */
    RCC->CFGR &= ~(1 << 16);            /* PLLSRC = 0 (HSE) */
    RCC->CFGR |= (1 << 16);             /* PLLSRC = 1 (HSE) */
    
    /* 4. Включаем PLL и ждем */
    RCC->CR |= (1 << 24);               /* PLLON */
    while(!(RCC->CR & (1 << 25)));      /* Ждем PLLRDY */
    
    /* 5. Настраиваем делители шин */
    /* AHB prescaler = 1 (72MHz) */
    RCC->CFGR &= ~(0xF << 4);
    
    /* APB1 prescaler = 2 (36MHz) */
    RCC->CFGR &= ~(0x7 << 8);
    RCC->CFGR |= (0x4 << 8);            /* APB1 prescaler = /2 */
    
    /* APB2 prescaler = 1 (72MHz) */
    RCC->CFGR &= ~(0x7 << 11);
    
    /* 6. Переключаем на PLL как источник системного такта */
    RCC->CFGR &= ~(0x3);                /* Очищаем SW bits */
    RCC->CFGR |= 0x2;                   /* SW = 10 (PLL) */
    while((RCC->CFGR & 0x0C) != 0x08);  /* Ждем переключения */
    
    /* 7. Включаем тактирование портов */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN |
                    RCC_APB2ENR_IOPBEN |
                    RCC_APB2ENR_IOPCEN;
    
    /* 8. Настраиваем SysTick для задержек */
    SysTick->LOAD = (SYSTEM_CLOCK_FREQ / 1000) - 1;  /* 1ms interval */
    SysTick->VAL = 0;
    SysTick->CTRL = (1 << 2) |  /* CLKSOURCE = processor clock */
                    (1 << 1) |  /* TICKINT = enable interrupt */
                    (1 << 0);   /* ENABLE = counter enable */
}

/* Задержка в миллисекундах */
void delay_ms(uint32_t ms) {
    uint32_t start_tick = system_tick;
    while((system_tick - start_tick) < ms) {
        __asm__("wfi");  /* Wait for interrupt для экономии энергии */
    }
}

/* Задержка в микросекундах (приблизительно) */
void delay_us(uint32_t us) {
    /* Для 72MHz: 72 цикла на микросекунду */
    uint32_t cycles = us * 72;
    delay_cycles(cycles);
}

/* Получение текущего тика */
uint32_t get_tick(void) {
    return system_tick;
}

/* Обработчик SysTick - теперь только здесь! */
void SysTick_Handler(void) { 
    system_tick++; 
}