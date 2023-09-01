#include <Arduino.h>
#include "core/Button.h"
#include <GyverTM1637.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <EEPROM.h>

/*--------------НАЛАШТУВАННЯ ПІНІВ ІНІЦІАЛІЗАЦІЇ--------------*/
#define CLK 12              // CLK дисплею
#define DIO 11              // DIO дисплею
#define DS18B20 8           // Датчик
#define HEATER 10           // Нагрівач
#define COOLER 14           // Кулер
#define LEDS 9              // Підсвітлювач печі
#define TIMER_LED 16        // Світлодіод кнопки таймера
#define TEMP_LED 15         // Світлодіод кнопки температури
/*------------------------------------------------------------*/

/*--------------------НАЛАШТУВАННЯ ЗНАЧЕНЬ--------------------*/    
#define BRIGHTNESS 7            // Яскравість дисплея від (0-7)

#define TIMER_STEP 30           // Шаг зміни таймера
#define TIMER_MINUTE 70000      // Хвилина в мс

#define TEMP_STEP 5             // Шаг зміни температури
#define TEMP_MAX 70             // Максимальна температура
#define TEMP_MIN 0              // Мінімальна температура
#define TEMP_QUIZ 10000         // Час опитування датчика
#define TEMP_STEPON 1           // Шаг для увімкнення реле
/*------------------------------------------------------------*/

/*--------------------НАЛАШТУВАННЯ ЗНАЧЕНЬ--------------------*/
#define ADDR_TEMPERATURE 0  // Адресса для зберігання температури користувача
/*------------------------------------------------------------*/

class Dryer
{
private:
    /*-----------------НАЛАШТУВАННЯ ОБ'ЄКТІВ КНОПОК-----------------*/  
    Button buttonStart;
    Button buttonTemperatureMode;
    Button buttonTimerMode;
    Button buttonAdd;
    Button buttonSubstact;
    Button buttonBacklight;
    /*--------------------------------------------------------------*/

    /*-----------------НАЛАШТУВАННЯ ОБ'ЄКТІВ ДИСПЛЕЇВ---------------*/
    GyverTM1637 display;            // Ініціалізація об'єкта дисплею	
    /*--------------------------------------------------------------*/

    /*-----------НАЛАШТУВАННЯ ОБ'ЄКТІВ ДАТЧИКА ТЕМПЕРАТУРИ----------*/
    OneWire oneWire;            // Ініціалізація об'єкта протокола
    DallasTemperature sensors;  // Ініціалізація об'єкта температури
    /*--------------------------------------------------------------*/

    /*----------------------ЗБЕРІГАННЯ ЗМІННИХ----------------------*/
    byte hours;                     // Параметр годин
    byte minutes;                   // Параметр хвилин
    byte temperature;               // Параметр температури
    byte prevTemperature;           // Змінена температура (для EEPROM)
    
    float realTemperature;          // Зберігання температури для виводу
    bool timerStopped;              // Якщо відсутній датчик
    /*--------------------------------------------------------------*/

    /*---------------------------СИСТЕМНЕ---------------------------*/
    bool systemBlock;               // Системне блокування
    bool temperatureMode;           // Режим температури
    bool timerMode;                 // Режим таймера
    bool timerRunning;              // Перевірка на таймер
    bool backlightState;            // Пісвітлення
    int ledState;                   // Стан світлодіодів
    long previousMillis;            // Для зберігання часу таймеру millis()
    /*--------------------------------------------------------------*/
public:
    Dryer() :
        // Ініціалізація об'єктів кнопок
        buttonStart(7),
        buttonTemperatureMode(6),
        buttonTimerMode(5),
        buttonAdd(4),
        buttonSubstact(3),
        buttonBacklight(2),

        // Ініціалізація об'єкту дисплею
        display(CLK, DIO),

        // Ініціалізація об'єктів датчика
        oneWire(DS18B20),
        sensors(&oneWire),

        // Ініціалізація користуваціких параметрів
        hours(0),
        minutes(0),
        temperature(TEMP_MIN),
        prevTemperature(0),

        // Системне
        systemBlock(false),
        temperatureMode(false),
        timerMode(true),
        timerRunning(false),
        backlightState(false),
        ledState(LOW),
        previousMillis(0)
    {}

    void setup() 
    {
        // Ініціалізація монітор-порта
        Serial.begin(9600);
        // Визов функції ініалізації датчика
        sensors.begin();
        // Налаштування яскравості екрану дисплея
        display.brightness(BRIGHTNESS);

        // Ініціалізація кулера
        pinMode(COOLER, OUTPUT);
        // Ініціалізація порту нагрівача
        pinMode(HEATER, OUTPUT);
        // Ініціалізація підсвітлювача
        pinMode(LEDS, OUTPUT);
        // Ініціалізація світлодіода на кнопку таймера
        pinMode(TIMER_LED, OUTPUT);
        // Ініціалізація світлодіода на кнопку температури
        pinMode(TEMP_LED, OUTPUT);

        // Синхранізація колишньої користувацької температури
        temperature = EEPROM.read(ADDR_TEMPERATURE);
        // Вивід таймера на дисплей
        display.displayClock(hours, minutes);
    }

    void loop()
    {
        ledBlinking(timerMode, temperatureMode);    // Функція блимання світлодіодів кнопок підсвітлення
        switchMode(systemBlock);                    // Функція вибору режиму
        backlight();                                // Функція підсвітлювача печі
        timerCounting();                            // Функція відліку часу
    }

    // Функція вибору режиму
    void switchMode(bool block)
    {
        // Перевірка на системне блокування
        if (block == false)
        {
            // Опитуємо кнопку температури
            buttonTemperatureMode.tick();
            // Опитуємо кнопку таймера
            buttonTimerMode.tick();

            // Якщо натистута кнопка температури
            if (buttonTemperatureMode.click())
            {
                // Дозвіл на зміну температури
                temperatureMode = true;
                // Заборона на зміну часу таймеру
                timerMode = false;
            }
            // Якщо натистута кнопка таймеру
            if (buttonTimerMode.click())
            {
                // Дозвіл на зміну часу таймера
                timerMode = true;
                // Заборона на зміну температури
                temperatureMode = false;
            }

            // Дозвіл на зміну температури
            if (temperatureMode == true) setTemperature(systemBlock);
            // Дозвіл на зміну часу таймера
            else if (timerMode == true) setTimer(systemBlock);
        }
    }

    // Функція блимання світлодіодів кнопок підсвітлення
    void ledBlinking(bool timerState, bool tempState)
    {
        // Якщо таймер не працює
        if (timerRunning == false)
        {
            unsigned long currentMillis = millis();
            if(currentMillis - previousMillis > 1000) 
            {
                // Зберігаємо час останнього перемикання
                previousMillis = currentMillis; 
            
                // Якщо світлодіод не горить, то запалюємо
                if (ledState == LOW) ledState = HIGH;
                // І навпаки
                else ledState = LOW; 
                // Встановлюємо стани виходу, щоб увімкнути або вимкнути світлодіод
                if (timerState == true ) digitalWrite(TIMER_LED, ledState);
                // Інакше працює підсвітлення без блимання
                else digitalWrite(TIMER_LED, HIGH);
                // Встановлюємо стани виходу, щоб увімкнути або вимкнути світлодіод
                if (tempState == true ) digitalWrite(TEMP_LED, ledState);
                // Інакше працює підсвітлення без блимання
                else digitalWrite(TEMP_LED, HIGH);
            }
        }
        // Інакше працює підсвітлення без блимання
        else
        {
            digitalWrite(TIMER_LED, HIGH);
            digitalWrite(TEMP_LED, HIGH);
        }
    }

    // Функція режиму користувацького таймера
    void setTimer(bool block)
    {
        // Перевіркуа на блокуванння системи
        if (block == false)
        {
            // Опитуємо кнопку +
            buttonAdd.tick();
            // Опитуємо кнопку -
            buttonSubstact.tick();

            // Якщо натиснуто +
            if (buttonAdd.click())
            {
                minutes += TIMER_STEP;
                if (minutes >= 60)
                {
                    hours += minutes / 60;
                    minutes = 0;
                    if (hours >= 24)
                    {
                        hours = 0;
                    }
                }
            }

            // Якщо натиснуто -
            if (buttonSubstact.click())
            {
                if (minutes >= TIMER_STEP)
                {
                    minutes -= TIMER_STEP;
                }
                else
                {
                    if (hours > 0)
                    {
                        hours--;
                        minutes = 60 - TIMER_STEP;
                    }
                }
            }
            // Вивід годин на дисплей
            display.displayClock(hours, minutes);
        }
    }

    // Функція для встановлення температури
    void setTemperature(bool block)
    {
        if (block == false)
        {   // Опитування стану кнопки
            buttonAdd.tick();
            // Опитування стану кнопки
            buttonSubstact.tick();

            // Вимикаємо двокрапку
            display.point(POINT_OFF);

            // Якщо натиснуто +
            if (buttonAdd.click())
            {
                temperature += TEMP_STEP;
            }
            // Обмежання в діапозон
            if (temperature > TEMP_MAX)
            {
                temperature = TEMP_MIN;
            }

            // Якщо натиснуто -
            if (buttonSubstact.click())
            {
                temperature -= TEMP_STEP;
            }
            // Обмежання в діапозон
            if (temperature <= TEMP_MIN)
            {
                temperature = TEMP_MIN;
            }

            // Запис в EEPROM
            if (temperature != prevTemperature)
            {
                EEPROM.write(ADDR_TEMPERATURE, temperature);
                prevTemperature = temperature;
            }

            // Переведення под дисплей
            byte temp = temperature;     // Перетворення значення температури на ціле число
            byte tempone = temp / 10;    // Отримуємо десятки
            byte temptwo = temp % 10;    // Отримуємо одиниці

            // Вивід на дисплей
            display.displayByte(0, 0x00);   // Звільнити місце
            display.display(2, temptwo);    // Виводимо одиниці
            display.display(1, tempone);    // выводимо десятки
            display.displayByte(3, 0x61);   // Підставляємо знак Цельсія
        }
    }

    // Функція відліку часу
    void timerCounting()
    {
        static unsigned long prevMillis = 0;
        static unsigned long interval = TIMER_MINUTE;

        // Опитування стану кнопки
        buttonStart.tick();   
        //Якщо натиснута, і години + хвилини == 0, то
        if(buttonStart.click() && hours + minutes != 0)
        {
            // Перевірка на роботу таймера
            if (!timerRunning) 
            {    
                // Якщо таймер не запущено, то запускаємо його
                timerRunning = true;
                // Блокуємо
                systemBlock = true;
                // Повертаємо яскравість
                display.brightness(BRIGHTNESS);
            } 
            // Інакше
            else 
            { 
                // Інакше зупиняємо таймер і зупиняємо навантаження
                timerRunning = false;
                // Розблокуємо системне блокування
                systemBlock = false;
            }
        }

        if (timerRunning) 
        {
            pollTemperatures();
            manageRele();

            if (hours + minutes == 0)
            {
                digitalWrite(HEATER, LOW);
                digitalWrite(COOLER, LOW);
            }
                
            display.displayClock(hours, minutes);   // Виведення часу на дисплей
                    
            unsigned long currentMillis = millis();
            if (currentMillis - prevMillis >= interval) 
            {
                prevMillis = currentMillis;
                if (minutes == 0) 
                {   
                    // Якщо хвилини закінчилися, то зменшуємо годинник
                    if (hours > 0) 
                    {
                        hours--;
                        minutes = 59;
                    } 
                    else 
                    { 
                        // Якщо і години закінчилися, то зупиняємо таймер
                        timerRunning = false;
                    }
                } 
                else 
                {  
                    // інакше зменшуємо хвилини
                    minutes--;
                }
            }
        }
        else
        {
            digitalWrite(HEATER, LOW);
            digitalWrite(COOLER, LOW);
        }
    }


    // Функція отримання температури датчика в печі
    void pollTemperatures()
    {
        static uint32_t timerTemp = millis();
        if(millis() - timerTemp >= TEMP_QUIZ)
        {
            // Запрос данних з датчика температури
            sensors.requestTemperatures();
            // Зчитування данних
            realTemperature = sensors.getTempCByIndex(0);
            // Відладка помилки відсутності/несправності датчика (Помилка 1)
            if (realTemperature == -127)
            {
                // Зупиняємо таймер
                timerRunning = false;
                // Вимикаємо двокрапку
                display.point(0);
                // Виведення в монітор порту (Помилка 1)
                Serial.println("Temperature retrieval error.");
                // Виведення на дисплей (Помилка 1)
                display.displayByte(_E, _r, _r, _1);
                // Якщо пройшло 5 секунд, перехід на старт-меню
                delay(5000);
                // Обнуляємо значення годин, 
                hours = 0;
                minutes = 0;
                temperature = TEMP_MIN;

            }
            // Якщо все добре, вивід в монітор порта поточної температури
            else
            {
                Serial.print("Temperature: ");
                Serial.print(realTemperature);
                Serial.println("C");
            }
            timerTemp = millis();
        }
    }

    // Функція для вимкнення реле заряду при досягненні встановленої температури
    void manageRele() 
    {
        // Перевіряємо, чи поточна температура перевищує або дорівнює заданій температурі
        if(realTemperature >= temperature) 
        {
            // Якщо умова виконується, вимикаємо реле
            digitalWrite(HEATER, LOW);
            // Якщо умова виконується, увімкнемо кулер
            digitalWrite(COOLER, HIGH);
        }

        // Перевіряємо, чи поточна температура менша за задану температуру
        if(realTemperature <= temperature - TEMP_STEPON) 
        {
            // Якщо умова виконується, увімкнемо нагрівач
            digitalWrite(HEATER, HIGH);
            // Якщо умова виконується, увімкнемо кулер
            digitalWrite(COOLER, HIGH);
        }
    }

    // Функція підсвітлювача печі
    void backlight()
    {
        // Опитуємо кнопку підсвітлювача
        buttonBacklight.tick();

        // Перевірка, чи натиснуто кнопку
        if (buttonBacklight.click())
        {
            // Перемкнути стан підсвічування
            backlightState = !backlightState;

            // Оновлення світлодіодів на основі стану підсвічування
            digitalWrite(LEDS, backlightState ? HIGH : LOW);
        }
    }
};

Dryer dryer;

void setup() 
{
    dryer.setup();
}

void loop() 
{
    dryer.loop();
}