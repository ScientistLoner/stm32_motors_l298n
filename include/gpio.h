#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

/* GPIO registers */
typedef struct {
    volatile uint32_t CRL;
    volatile uint32_t CRH;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t BRR;
    volatile uint32_t LCKR;
} GPIO_TypeDef;

/* Base addresses */
#define GPIOA_BASE      ((uint32_t)0x40010800)
#define GPIOB_BASE      ((uint32_t)0x40010C00)
#define GPIOC_BASE      ((uint32_t)0x40011000)

/* Peripheral pointers */
#define GPIOA           ((GPIO_TypeDef*)GPIOA_BASE)
#define GPIOB           ((GPIO_TypeDef*)GPIOB_BASE)
#define GPIOC           ((GPIO_TypeDef*)GPIOC_BASE)

/* GPIO Configuration Register values */
/* Mode bits (2 bits per pin) */
#define GPIO_MODE_INPUT     0x0
#define GPIO_MODE_OUTPUT_10MHZ 0x1
#define GPIO_MODE_OUTPUT_2MHZ  0x2
#define GPIO_MODE_OUTPUT_50MHZ 0x3

/* Configuration bits (2 bits per pin) */
#define GPIO_CNF_INPUT_ANALOG  0x0
#define GPIO_CNF_INPUT_FLOATING 0x1
#define GPIO_CNF_INPUT_PUPD    0x2

#define GPIO_CNF_OUTPUT_PP     0x0  /* Push-pull */
#define GPIO_CNF_OUTPUT_OD     0x1  /* Open-drain */
#define GPIO_CNF_AF_OUTPUT_PP  0x2  /* Alternate function push-pull */
#define GPIO_CNF_AF_OUTPUT_OD  0x3  /* Alternate function open-drain */

/* Simplified pin mode macros */
#define GPIO_MODE_OUT_PP       (GPIO_MODE_OUTPUT_50MHZ | (GPIO_CNF_OUTPUT_PP << 2))
#define GPIO_MODE_OUT_OD       (GPIO_MODE_OUTPUT_50MHZ | (GPIO_CNF_OUTPUT_OD << 2))
#define GPIO_MODE_AF_PP        (GPIO_MODE_OUTPUT_50MHZ | (GPIO_CNF_AF_OUTPUT_PP << 2))
#define GPIO_MODE_AF_OD        (GPIO_MODE_OUTPUT_50MHZ | (GPIO_CNF_AF_OUTPUT_OD << 2))
#define GPIO_MODE_IN_FLOATING  (GPIO_MODE_INPUT | (GPIO_CNF_INPUT_FLOATING << 2))
#define GPIO_MODE_IN_PUPD      (GPIO_MODE_INPUT | (GPIO_CNF_INPUT_PUPD << 2))
#define GPIO_MODE_IN_ANALOG    (GPIO_MODE_INPUT | (GPIO_CNF_INPUT_ANALOG << 2))

/* Function prototypes */
void GPIO_Init(void);
void GPIO_SetPin(GPIO_TypeDef* gpio, uint8_t pin);
void GPIO_ResetPin(GPIO_TypeDef* gpio, uint8_t pin);
void GPIO_TogglePin(GPIO_TypeDef* gpio, uint8_t pin);
uint8_t GPIO_ReadPin(GPIO_TypeDef* gpio, uint8_t pin);
void GPIO_ConfigPin(GPIO_TypeDef* gpio, uint8_t pin, uint8_t mode);

#endif /* GPIO_H */