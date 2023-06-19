#include <Arduino.h>
#include <EncButton.h>
#include <GyverTM1637.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <GyverTimers.h>

/*--------------НАЛАШТУВАННЯ ПІНІВ ІНІЦІАЛІЗАЦІЇ--------------*/
#define CLK_DS 15           // CLK датчик
#define DIO_DS 16           // DIO датчик
#define CLK_TA 17           // CLK таймер
#define DIO_TA 18           // DIO таймер
#define CHARGE 9            // Навантаження реле
#define DS18B20 10          // Датчик
/*------------------------------------------------------------*/

/*--------------------НАЛАШТУВАННЯ ЗНАЧЕНЬ--------------------*/
#define TIMER_BRIGHTMESS 3      // Яскравість дисплея від (0-7)
#define TIMER_DELEY 150         // Затримка додавння значень
#define TIMER_STEP 10           // Шаг для тривалому затисканню

#define TEMP_BRIGHTMESS 7       // Яскравість дисплея від (0-7)  
#define TEMP_DELAY 300          // Затримка додавння значень 
#define TEMP_STEP 5             // Шаг для тривалому затисканню                                  
#define TEMP_MAX 70             // Максимальна температура
#define TEMP_MIN 20             // Мінімальна температура
#define TEMP_STEPON 1           // Шаг для увімкнення реле
#define TEMP_QUIZ 5000          // Час опитування датчика
/*------------------------------------------------------------*/

void setup() 
{

}

void loop() 
{

}
