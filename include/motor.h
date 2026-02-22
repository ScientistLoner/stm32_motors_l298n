#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>

/* Motor direction */
typedef enum {
    MOTOR_STOP = 0,
    MOTOR_FORWARD,
    MOTOR_BACKWARD,
    MOTOR_BRAKE
} Motor_Direction;

/* Motor channels */
typedef enum {
    MOTOR_LEFT = 0,
    MOTOR_RIGHT
} Motor_Channel;

/* Motor status codes */
typedef enum {
    MOTOR_OK = 0,
    MOTOR_ERROR_INVALID_CHANNEL,
    MOTOR_ERROR_INVALID_SPEED,
    MOTOR_ERROR_INVALID_DIRECTION,
    MOTOR_ERROR_NOT_INITIALIZED
} Motor_Status;

/* Motor speeds */
#define MOTOR_SPEED_MIN     0
#define MOTOR_SPEED_MAX     100
#define MOTOR_SPEED_DEFAULT 50

/* L298N control pins configuration */
/* Blue Pill pinout for L298N:
 * 
 * LEFT MOTOR:
 *   IN1: PB0 (Timer 3, Channel 3)
 *   IN2: PB1 (Timer 3, Channel 4)
 *   ENA: PA6 (Timer 3, Channel 1) - PWM speed control
 * 
 * RIGHT MOTOR:
 *   IN3: PB6 (Timer 4, Channel 1)
 *   IN4: PB7 (Timer 4, Channel 2)
 *   ENB: PA7 (Timer 3, Channel 2) - PWM speed control
 */

/* Инициализация и базовые функции */
Motor_Status Motor_Init(void);
Motor_Status Motor_Deinit(void);
Motor_Status Motor_SetSpeed(Motor_Channel motor, uint8_t speed);
Motor_Status Motor_SetDirection(Motor_Channel motor, Motor_Direction dir);
Motor_Status Motor_Stop(Motor_Channel motor);
Motor_Status Motor_StopAll(void);
uint8_t Motor_GetSpeed(Motor_Channel motor);
Motor_Direction Motor_GetDirection(Motor_Channel motor);

/* Комплексные движения */
Motor_Status Motor_ForwardAll(uint8_t speed);
Motor_Status Motor_BackwardAll(uint8_t speed);
Motor_Status Motor_TurnLeft(uint8_t speed);
Motor_Status Motor_TurnRight(uint8_t speed);
Motor_Status Motor_SpinLeft(uint8_t speed);
Motor_Status Motor_SpinRight(uint8_t speed);

/* Дополнительные функции */
Motor_Status Motor_SetSpeeds(uint8_t left_speed, uint8_t right_speed);
Motor_Status Motor_SoftStart(Motor_Channel motor, uint8_t target_speed, uint32_t duration_ms);
Motor_Status Motor_SoftStop(Motor_Channel motor, uint32_t duration_ms);

/* Тестовые функции */
void Motor_TestSequence(void);
Motor_Status Motor_SelfTest(void);

/* Отладочные функции */
#ifdef MOTOR_DEBUG
void Motor_PrintStatus(void);
#endif

#endif /* MOTOR_H */