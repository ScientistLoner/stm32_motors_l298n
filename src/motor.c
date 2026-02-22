#include "motor.h"
#include "gpio.h"
#include "pwm.h"
#include "system.h"
#include "utils.h"

/* Текущие скорости моторов */
static uint8_t motor_speeds[2] = {MOTOR_SPEED_DEFAULT, MOTOR_SPEED_DEFAULT};

/* Текущие направления моторов */
static Motor_Direction motor_directions[2] = {MOTOR_STOP, MOTOR_STOP};

/* Флаг инициализации */
static uint8_t motor_initialized = 0;

/* Вспомогательная функция для валидации параметров */
static Motor_Status validate_channel(Motor_Channel motor) {
    if (motor != MOTOR_LEFT && motor != MOTOR_RIGHT) {
        return MOTOR_ERROR_INVALID_CHANNEL;
    }
    return MOTOR_OK;
}

static Motor_Status validate_speed(uint8_t speed) {
    if (speed > MOTOR_SPEED_MAX) {
        return MOTOR_ERROR_INVALID_SPEED;
    }
    return MOTOR_OK;
}

static Motor_Status check_initialized(void) {
    if (!motor_initialized) {
        return MOTOR_ERROR_NOT_INITIALIZED;
    }
    return MOTOR_OK;
}

/* Инициализация драйвера моторов */
Motor_Status Motor_Init(void) {
    Motor_Status status;
    
    /* Проверяем, не инициализирован ли уже */
    if (motor_initialized) {
        return MOTOR_OK;
    }
    
    /* Инициализация PWM для управления скоростью */
    /* Частота PWM: 20kHz (выше слышимого диапазона) */
    PWM_Init(PWM_TIM3, 20000);  /* TIM3 для скорости моторов */
    PWM_Init(PWM_TIM4, 20000);  /* TIM4 для направления (если нужно PWM) */
    
    /* Настраиваем GPIO для управления направлением */
    /* IN1, IN2 (левый мотор) уже настроены как PWM в pwm_configure_pins */
    /* IN3, IN4 (правый мотор) уже настроены как PWM в pwm_configure_pins */
    
    /* Дополнительно настраиваем как обычные выходы для надежности */
    GPIO_ConfigPin(GPIOB, 0, GPIO_MODE_OUT_PP);  /* IN1 */
    GPIO_ConfigPin(GPIOB, 1, GPIO_MODE_OUT_PP);  /* IN2 */
    GPIO_ConfigPin(GPIOB, 6, GPIO_MODE_OUT_PP);  /* IN3 */
    GPIO_ConfigPin(GPIOB, 7, GPIO_MODE_OUT_PP);  /* IN4 */
    
    /* Инициализация состояний */
    GPIO_ResetPin(GPIOB, 0);  /* IN1 = 0 */
    GPIO_ResetPin(GPIOB, 1);  /* IN2 = 0 */
    GPIO_ResetPin(GPIOB, 6);  /* IN3 = 0 */
    GPIO_ResetPin(GPIOB, 7);  /* IN4 = 0 */
    
    /* Запускаем PWM таймеры */
    PWM_Start(PWM_TIM3);
    PWM_Start(PWM_TIM4);
    
    /* Устанавливаем начальную скорость 0% */
    PWM_SetDuty(PWM_TIM3, PWM_CH1, 0);  /* ENA */
    PWM_SetDuty(PWM_TIM3, PWM_CH2, 0);  /* ENB */
    
    /* Устанавливаем направление "стоп" */
    motor_directions[MOTOR_LEFT] = MOTOR_STOP;
    motor_directions[MOTOR_RIGHT] = MOTOR_STOP;
    
    motor_initialized = 1;
    return MOTOR_OK;
}

/* Деинициализация драйвера моторов */
Motor_Status Motor_Deinit(void) {
    Motor_Status status;
    
    if (!motor_initialized) {
        return MOTOR_OK;
    }
    
    /* Останавливаем все моторы */
    status = Motor_StopAll();
    if (status != MOTOR_OK) {
        return status;
    }
    
    /* Останавливаем PWM таймеры */
    PWM_Stop(PWM_TIM3);
    PWM_Stop(PWM_TIM4);
    
    /* Сбрасываем пины */
    GPIO_ResetPin(GPIOB, 0);
    GPIO_ResetPin(GPIOB, 1);
    GPIO_ResetPin(GPIOB, 6);
    GPIO_ResetPin(GPIOB, 7);
    
    motor_initialized = 0;
    return MOTOR_OK;
}

/* Установка скорости мотора (0-100%) */
Motor_Status Motor_SetSpeed(Motor_Channel motor, uint8_t speed) {
    Motor_Status status;
    
    /* Проверка инициализации */
    status = check_initialized();
    if (status != MOTOR_OK) return status;
    
    /* Валидация параметров */
    status = validate_channel(motor);
    if (status != MOTOR_OK) return status;
    
    status = validate_speed(speed);
    if (status != MOTOR_OK) return status;
    
    /* Ограничиваем скорость */
    motor_speeds[motor] = clamp_u8(speed, MOTOR_SPEED_MIN, MOTOR_SPEED_MAX);
    
    /* Применяем скорость с учетом текущего направления */
    if(motor_directions[motor] != MOTOR_STOP) {
        if(motor == MOTOR_LEFT) {
            PWM_SetDuty(PWM_TIM3, PWM_CH1, motor_speeds[motor]);  /* ENA */
        } else {
            PWM_SetDuty(PWM_TIM3, PWM_CH2, motor_speeds[motor]);  /* ENB */
        }
    }
    
    return MOTOR_OK;
}

/* Получение текущей скорости мотора */
uint8_t Motor_GetSpeed(Motor_Channel motor) {
    if (validate_channel(motor) != MOTOR_OK) {
        return 0;
    }
    return motor_speeds[motor];
}

/* Установка направления движения мотора */
Motor_Status Motor_SetDirection(Motor_Channel motor, Motor_Direction dir) {
    Motor_Status status;
    
    /* Проверка инициализации */
    status = check_initialized();
    if (status != MOTOR_OK) return status;
    
    /* Валидация параметров */
    status = validate_channel(motor);
    if (status != MOTOR_OK) return status;
    
    if (dir > MOTOR_BRAKE) {
        return MOTOR_ERROR_INVALID_DIRECTION;
    }
    
    motor_directions[motor] = dir;
    
    switch(motor) {
        case MOTOR_LEFT:
            switch(dir) {
                case MOTOR_STOP:
                    GPIO_ResetPin(GPIOB, 0);  /* IN1 = 0 */
                    GPIO_ResetPin(GPIOB, 1);  /* IN2 = 0 */
                    PWM_SetDuty(PWM_TIM3, PWM_CH1, 0);  /* ENA = 0 */
                    break;
                    
                case MOTOR_FORWARD:
                    GPIO_SetPin(GPIOB, 0);    /* IN1 = 1 */
                    GPIO_ResetPin(GPIOB, 1);  /* IN2 = 0 */
                    PWM_SetDuty(PWM_TIM3, PWM_CH1, motor_speeds[MOTOR_LEFT]);
                    break;
                    
                case MOTOR_BACKWARD:
                    GPIO_ResetPin(GPIOB, 0);  /* IN1 = 0 */
                    GPIO_SetPin(GPIOB, 1);    /* IN2 = 1 */
                    PWM_SetDuty(PWM_TIM3, PWM_CH1, motor_speeds[MOTOR_LEFT]);
                    break;
                    
                case MOTOR_BRAKE:
                    GPIO_SetPin(GPIOB, 0);    /* IN1 = 1 */
                    GPIO_SetPin(GPIOB, 1);    /* IN2 = 1 */
                    PWM_SetDuty(PWM_TIM3, PWM_CH1, 100);  /* Полный тормоз */
                    break;
            }
            break;
            
        case MOTOR_RIGHT:
            switch(dir) {
                case MOTOR_STOP:
                    GPIO_ResetPin(GPIOB, 6);  /* IN3 = 0 */
                    GPIO_ResetPin(GPIOB, 7);  /* IN4 = 0 */
                    PWM_SetDuty(PWM_TIM3, PWM_CH2, 0);  /* ENB = 0 */
                    break;
                    
                case MOTOR_FORWARD:
                    GPIO_SetPin(GPIOB, 6);    /* IN3 = 1 */
                    GPIO_ResetPin(GPIOB, 7);  /* IN4 = 0 */
                    PWM_SetDuty(PWM_TIM3, PWM_CH2, motor_speeds[MOTOR_RIGHT]);
                    break;
                    
                case MOTOR_BACKWARD:
                    GPIO_ResetPin(GPIOB, 6);  /* IN3 = 0 */
                    GPIO_SetPin(GPIOB, 7);    /* IN4 = 1 */
                    PWM_SetDuty(PWM_TIM3, PWM_CH2, motor_speeds[MOTOR_RIGHT]);
                    break;
                    
                case MOTOR_BRAKE:
                    GPIO_SetPin(GPIOB, 6);    /* IN3 = 1 */
                    GPIO_SetPin(GPIOB, 7);    /* IN4 = 1 */
                    PWM_SetDuty(PWM_TIM3, PWM_CH2, 100);  /* Полный тормоз */
                    break;
            }
            break;
    }
    
    return MOTOR_OK;
}

/* Получение текущего направления мотора */
Motor_Direction Motor_GetDirection(Motor_Channel motor) {
    if (validate_channel(motor) != MOTOR_OK) {
        return MOTOR_STOP;
    }
    return motor_directions[motor];
}

/* Остановка конкретного мотора */
Motor_Status Motor_Stop(Motor_Channel motor) {
    return Motor_SetDirection(motor, MOTOR_STOP);
}

/* Остановка всех моторов */
Motor_Status Motor_StopAll(void) {
    Motor_Status status;
    
    status = Motor_Stop(MOTOR_LEFT);
    if (status != MOTOR_OK) return status;
    
    status = Motor_Stop(MOTOR_RIGHT);
    return status;
}

/* Движение всех моторов вперед */
Motor_Status Motor_ForwardAll(uint8_t speed) {
    Motor_Status status;
    
    status = Motor_SetSpeed(MOTOR_LEFT, speed);
    if (status != MOTOR_OK) return status;
    
    status = Motor_SetSpeed(MOTOR_RIGHT, speed);
    if (status != MOTOR_OK) return status;
    
    status = Motor_SetDirection(MOTOR_LEFT, MOTOR_FORWARD);
    if (status != MOTOR_OK) return status;
    
    status = Motor_SetDirection(MOTOR_RIGHT, MOTOR_FORWARD);
    return status;
}

/* Движение всех моторов назад */
Motor_Status Motor_BackwardAll(uint8_t speed) {
    Motor_Status status;
    
    status = Motor_SetSpeed(MOTOR_LEFT, speed);
    if (status != MOTOR_OK) return status;
    
    status = Motor_SetSpeed(MOTOR_RIGHT, speed);
    if (status != MOTOR_OK) return status;
    
    status = Motor_SetDirection(MOTOR_LEFT, MOTOR_BACKWARD);
    if (status != MOTOR_OK) return status;
    
    status = Motor_SetDirection(MOTOR_RIGHT, MOTOR_BACKWARD);
    return status;
}

/* Поворот налево (правый мотор вперед, левый назад) */
Motor_Status Motor_TurnLeft(uint8_t speed) {
    Motor_Status status;
    
    status = Motor_SetSpeed(MOTOR_LEFT, speed);
    if (status != MOTOR_OK) return status;
    
    status = Motor_SetSpeed(MOTOR_RIGHT, speed);
    if (status != MOTOR_OK) return status;
    
    status = Motor_SetDirection(MOTOR_LEFT, MOTOR_BACKWARD);
    if (status != MOTOR_OK) return status;
    
    status = Motor_SetDirection(MOTOR_RIGHT, MOTOR_FORWARD);
    return status;
}

/* Поворот направо (левый мотор вперед, правый назад) */
Motor_Status Motor_TurnRight(uint8_t speed) {
    Motor_Status status;
    
    status = Motor_SetSpeed(MOTOR_LEFT, speed);
    if (status != MOTOR_OK) return status;
    
    status = Motor_SetSpeed(MOTOR_RIGHT, speed);
    if (status != MOTOR_OK) return status;
    
    status = Motor_SetDirection(MOTOR_LEFT, MOTOR_FORWARD);
    if (status != MOTOR_OK) return status;
    
    status = Motor_SetDirection(MOTOR_RIGHT, MOTOR_BACKWARD);
    return status;
}

/* Разворот на месте влево (левый назад, правый вперед) */
Motor_Status Motor_SpinLeft(uint8_t speed) {
    Motor_Status status;
    
    status = Motor_SetSpeed(MOTOR_LEFT, speed);
    if (status != MOTOR_OK) return status;
    
    status = Motor_SetSpeed(MOTOR_RIGHT, speed);
    if (status != MOTOR_OK) return status;
    
    status = Motor_SetDirection(MOTOR_LEFT, MOTOR_BACKWARD);
    if (status != MOTOR_OK) return status;
    
    status = Motor_SetDirection(MOTOR_RIGHT, MOTOR_FORWARD);
    return status;
}

/* Разворот на месте вправо (левый вперед, правый назад) */
Motor_Status Motor_SpinRight(uint8_t speed) {
    Motor_Status status;
    
    status = Motor_SetSpeed(MOTOR_LEFT, speed);
    if (status != MOTOR_OK) return status;
    
    status = Motor_SetSpeed(MOTOR_RIGHT, speed);
    if (status != MOTOR_OK) return status;
    
    status = Motor_SetDirection(MOTOR_LEFT, MOTOR_FORWARD);
    if (status != MOTOR_OK) return status;
    
    status = Motor_SetDirection(MOTOR_RIGHT, MOTOR_BACKWARD);
    return status;
}

/* Установка скоростей для обоих моторов */
Motor_Status Motor_SetSpeeds(uint8_t left_speed, uint8_t right_speed) {
    Motor_Status status;
    
    status = Motor_SetSpeed(MOTOR_LEFT, left_speed);
    if (status != MOTOR_OK) return status;
    
    status = Motor_SetSpeed(MOTOR_RIGHT, right_speed);
    return status;
}

/* Плавный старт мотора */
Motor_Status Motor_SoftStart(Motor_Channel motor, uint8_t target_speed, uint32_t duration_ms) {
    Motor_Status status;
    uint32_t start_time;
    uint8_t current_speed;
    uint32_t elapsed;
    
    status = validate_channel(motor);
    if (status != MOTOR_OK) return status;
    
    status = validate_speed(target_speed);
    if (status != MOTOR_OK) return status;
    
    if (duration_ms == 0) {
        return Motor_SetSpeed(motor, target_speed);
    }
    
    current_speed = motor_speeds[motor];
    start_time = get_tick();
    
    while (1) {
        elapsed = get_tick() - start_time;
        if (elapsed >= duration_ms) {
            break;
        }
        
        /* Линейная интерполяция скорости */
        uint8_t new_speed = current_speed + 
                           (target_speed - current_speed) * elapsed / duration_ms;
        
        status = Motor_SetSpeed(motor, new_speed);
        if (status != MOTOR_OK) return status;
        
        delay_ms(10); /* Обновляем каждые 10 мс */
    }
    
    /* Финальная установка целевой скорости */
    return Motor_SetSpeed(motor, target_speed);
}

/* Плавная остановка мотора */
Motor_Status Motor_SoftStop(Motor_Channel motor, uint32_t duration_ms) {
    Motor_Status status;
    
    status = validate_channel(motor);
    if (status != MOTOR_OK) return status;
    
    uint8_t current_speed = motor_speeds[motor];
    status = Motor_SoftStart(motor, 0, duration_ms);
    if (status != MOTOR_OK) return status;
    
    return Motor_Stop(motor);
}

/* Самотестирование системы */
Motor_Status Motor_SelfTest(void) {
    Motor_Status status;
    
    status = check_initialized();
    if (status != MOTOR_OK) return status;
    
    /* Тест левого мотора */
    status = Motor_SetDirection(MOTOR_LEFT, MOTOR_FORWARD);
    if (status != MOTOR_OK) return status;
    delay_ms(100);
    status = Motor_Stop(MOTOR_LEFT);
    if (status != MOTOR_OK) return status;
    delay_ms(50);
    
    /* Тест правого мотора */
    status = Motor_SetDirection(MOTOR_RIGHT, MOTOR_FORWARD);
    if (status != MOTOR_OK) return status;
    delay_ms(100);
    status = Motor_Stop(MOTOR_RIGHT);
    if (status != MOTOR_OK) return status;
    delay_ms(50);
    
    /* Тест торможения */
    status = Motor_SetDirection(MOTOR_LEFT, MOTOR_BRAKE);
    if (status != MOTOR_OK) return status;
    status = Motor_SetDirection(MOTOR_RIGHT, MOTOR_BRAKE);
    if (status != MOTOR_OK) return status;
    delay_ms(100);
    
    status = Motor_StopAll();
    return status;
}

/* Отладочная печать статуса (если включен MOTOR_DEBUG) */
#ifdef MOTOR_DEBUG
void Motor_PrintStatus(void) {
    /* Здесь можно добавить вывод через UART */
    /* Пример: */
    /* printf("Motor L: speed=%d%%, dir=%d\n", 
              motor_speeds[MOTOR_LEFT], 
              motor_directions[MOTOR_LEFT]);
       printf("Motor R: speed=%d%%, dir=%d\n", 
              motor_speeds[MOTOR_RIGHT], 
              motor_directions[MOTOR_RIGHT]); */
}
#endif

/* Тестовая последовательность для проверки моторов */
void Motor_TestSequence(void) {
    Motor_Status status;
    
    /* Проверяем инициализацию */
    status = Motor_Init();
    if (status != MOTOR_OK) {
        /* Индикация ошибки - быстрое мигание светодиода */
        for(int i = 0; i < 10; i++) {
            GPIO_TogglePin(GPIOC, 13);
            delay_ms(50);
        }
        return;
    }
    
    /* Короткая пауза после инициализации */
    delay_ms(100);
    
    /* Мигаем светодиодом - начало теста */
    for(int i = 0; i < 3; i++) {
        GPIO_TogglePin(GPIOC, 13);
        delay_ms(200);
    }
    
    /* Тест 1: Плавный разгон вперед */
    for(uint8_t speed = 0; speed <= 80; speed += 10) {
        status = Motor_ForwardAll(speed);
        if (status != MOTOR_OK) break;
        delay_ms(200);
    }
    Motor_StopAll();
    delay_ms(500);
    
    /* Тест 2: Плавный разгон назад */
    for(uint8_t speed = 0; speed <= 80; speed += 10) {
        status = Motor_BackwardAll(speed);
        if (status != MOTOR_OK) break;
        delay_ms(200);
    }
    Motor_StopAll();
    delay_ms(500);
    
    /* Тест 3: Повороты */
    status = Motor_TurnLeft(60);
    if (status == MOTOR_OK) {
        delay_ms(1000);
        Motor_StopAll();
        delay_ms(300);
    }
    
    status = Motor_TurnRight(60);
    if (status == MOTOR_OK) {
        delay_ms(1000);
        Motor_StopAll();
        delay_ms(500);
    }
    
    /* Тест 4: Развороты на месте */
    status = Motor_SpinLeft(50);
    if (status == MOTOR_OK) {
        delay_ms(800);
        Motor_StopAll();
        delay_ms(300);
    }
    
    status = Motor_SpinRight(50);
    if (status == MOTOR_OK) {
        delay_ms(800);
        Motor_StopAll();
        delay_ms(500);
    }
    
    /* Тест 5: Торможение */
    status = Motor_ForwardAll(70);
    if (status == MOTOR_OK) {
        delay_ms(500);
        Motor_SetDirection(MOTOR_LEFT, MOTOR_BRAKE);
        Motor_SetDirection(MOTOR_RIGHT, MOTOR_BRAKE);
        delay_ms(300);
    }
    
    /* Финиш - все остановлены */
    Motor_StopAll();
    
    /* Мигаем светодиодом - успешное завершение */
    for(int i = 0; i < 5; i++) {
        GPIO_TogglePin(GPIOC, 13);
        delay_ms(100);
    }
}