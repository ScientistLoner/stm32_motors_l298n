#include "system.h"
#include "gpio.h"
#include "motor.h"

int main(void) {
    /* Инициализация системы */
    System_Init();
    
    /* Инициализация GPIO */
    GPIO_Init();
    
    /* Инициализация моторов */
    Motor_Init();
    
    /* Тестовая последовательность */
    Motor_TestSequence();
    
    /* Основной цикл */
    while(1) {
        /* Пример 1: Движение вперед */
        Motor_ForwardAll(70);  /* 70% скорости */
        delay_ms(2000);
        
        /* Остановка */
        Motor_StopAll();
        delay_ms(500);
        
        /* Пример 2: Поворот налево */
        Motor_TurnLeft(60);
        delay_ms(1000);
        
        /* Пример 3: Поворот направо */
        Motor_TurnRight(60);
        delay_ms(1000);
        
        /* Пример 4: Движение назад */
        Motor_BackwardAll(50);
        delay_ms(2000);
        
        /* Пример 5: Разворот на месте влево */
        Motor_SpinLeft(40);
        delay_ms(800);
        
        /* Пример 6: Разворот на месте вправо */
        Motor_SpinRight(40);
        delay_ms(800);
        
        /* Полная остановка */
        Motor_StopAll();
        delay_ms(1000);
        
        /* Мигание светодиодом на PC13 (если есть) */
        GPIO_TogglePin(GPIOC, 13);
    }
    
    return 0;
}
