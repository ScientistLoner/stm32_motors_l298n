#include "gpio.h"
#include "system.h"

/* Инициализация GPIO */
void GPIO_Init(void) {
    /* Тактирование портов уже включено в System_Init() */
    
    /* Настраиваем светодиод на PC13 (если есть на Blue Pill) */
    GPIO_ConfigPin(GPIOC, 13, GPIO_MODE_OUT_PP);
    GPIO_ResetPin(GPIOC, 13);  /* Выключаем светодиод */
}

/* Конфигурация пина */
void GPIO_ConfigPin(GPIO_TypeDef* gpio, uint8_t pin, uint8_t mode) {
    volatile uint32_t *config_reg;
    uint32_t shift;
    
    if(pin < 8) {
        /* CRL register for pins 0-7 */
        config_reg = &gpio->CRL;
        shift = pin * 4;
    } else {
        /* CRH register for pins 8-15 */
        config_reg = &gpio->CRH;
        shift = (pin - 8) * 4;
    }
    
    /* Очищаем биты конфигурации для этого пина */
    *config_reg &= ~(0x0F << shift);
    
    /* Устанавливаем новую конфигурацию */
    *config_reg |= ((uint32_t)mode << shift);
}

/* Установка пина в 1 */
void GPIO_SetPin(GPIO_TypeDef* gpio, uint8_t pin) {
    gpio->BSRR = (1U << pin);
}

/* Сброс пина в 0 */
void GPIO_ResetPin(GPIO_TypeDef* gpio, uint8_t pin) {
    gpio->BRR = (1U << pin);
}

/* Переключение пина */
void GPIO_TogglePin(GPIO_TypeDef* gpio, uint8_t pin) {
    if(gpio->ODR & (1U << pin)) {
        GPIO_ResetPin(gpio, pin);
    } else {
        GPIO_SetPin(gpio, pin);
    }
}

/* Чтение состояния пина */
uint8_t GPIO_ReadPin(GPIO_TypeDef* gpio, uint8_t pin) {
    return (gpio->IDR & (1U << pin)) ? 1U : 0U;
}