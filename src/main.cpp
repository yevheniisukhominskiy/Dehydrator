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
#define TIMER_MINUTE 67000      // Одна хвилина для таймеру

#define TEMP_BRIGHTMESS 7       // Яскравість дисплея від (0-7)  
#define TEMP_DELAY 300          // Затримка додавння значень 
#define TEMP_STEP 5             // Шаг для тривалому затисканню                                  
#define TEMP_MAX 70             // Максимальна температура
#define TEMP_MIN 20             // Мінімальна температура
#define TEMP_STEPON 1           // Шаг для увімкнення реле
#define TEMP_QUIZ 5000          // Час опитування датчика
/*------------------------------------------------------------*/

class Dryer
{
private:
    /*-----------------НАЛАШТУВАННЯ ОБ'ЄКТІВ КНОПОК-----------------*/  
    EncButton<EB_TICK, 2> buttonTempUp;     // Температура додавання    
    EncButton<EB_TICK, 3> buttonTempDown;   // Температура віднімання 
    EncButton<EB_TICK, 4> buttonTimerUp;    // Таймер додавання 
    EncButton<EB_TICK, 5> buttonTimerDown;  // Таймер віднімання
    EncButton<EB_TICK, 6> buttonMode;       // Вибір режиму
    EncButton<EB_TICK, 7> buttonStart;      // Старт/стоп/блокування
    /*--------------------------------------------------------------*/

    /*-----------------НАЛАШТУВАННЯ ОБ'ЄКТІВ ДИСПЛЕЇВ---------------*/
    GyverTM1637 disp_ds;            // Дисплей температури	
    GyverTM1637 disp_ta;            // Дисплей таймеру
    /*--------------------------------------------------------------*/

    /*-----------НАЛАШТУВАННЯ ОБ'ЄКТІВ ДАТЧИКА ТЕМПЕРАТУРИ----------*/
    OneWire oneWire;            // Ініціалізація об'єкта протокола
    DallasTemperature sensors;  // Ініціалізація об'єкта температури
    /*--------------------------------------------------------------*/

    /*----------------------ЗБЕРІГАННЯ ЗМІННИХ----------------------*/
    float temperature;  // Температура
    byte hours;         // Години
    byte minutes;       // Хвилини
    bool timerRunning;  // Статус таймера
    float tempC;        // Зберігння температури для виводу
    bool celsiusSign;   // Статус зміни режиму на дисплею температури
    /*--------------------------------------------------------------*/

public:
Dryer() :
        buttonTempUp(),
        buttonTempDown(),
        buttonTimerUp(),
        buttonTimerDown(),
        buttonMode(),
        buttonStart(),
        disp_ds(CLK_DS, DIO_DS),
        disp_ta(CLK_TA, DIO_TA),
        oneWire(DS18B20),
        sensors(&oneWire),
        temperature(TEMP_MIN),
        hours(0),
        minutes(0),
        timerRunning(false),
        tempC(),
        celsiusSign(false)
    {}

     void setup() 
    {
        Serial.begin(9600);                     // Монітор порта для налагодження
        sensors.begin();                        // Визов функції об'єкту 
        pinMode(CHARGE, OUTPUT);                // Ініціалізація вихідного навантаження
        disp_ds.brightness(TEMP_BRIGHTMESS);    // Яскравість дисплею температури
        disp_ta.brightness(TIMER_BRIGHTMESS);   // Яскравість дисплею таймеру
    }

    void loop() 
    {
        setTimer();                     // Функція для встановлення часу на таймері
        timerCounting();                // Функція відліку часу
        setTemperature(celsiusSign);    // Функція для встановлення температури
    }

    // Функція для встановлення часу на таймері
    void setTimer()
    {
        buttonTimerUp.tick();       // Опитуємо стан кнопки "Збільшити час"
        buttonTimerDown.tick();     // Опитуємо стан кнопки "Зменшити час"

        if (buttonTimerUp.click())
        {
            minutes += TIMER_STEP;
            if(minutes == 60)
            {
                minutes = 0;
                hours++;
            }
            else if(hours >= 24)
            {
                hours = 0;
                minutes = 0;
            }
        }
        else if(buttonTimerUp.hold())
        {
            minutes += TIMER_STEP;
            delay(TIMER_DELEY);
            if(minutes == 60)
            {
                minutes = 0;
                hours++;
            }
            else if(hours >= 24)
            {
                hours = 0;
                minutes = 0;
            }
        }

        if (buttonTimerDown.click()) 
        {
           if(minutes > 0)
           {
            minutes -= TIMER_STEP;
           }
           else
           {
                if(hours > 0)
                {
                    hours--;
                    minutes = 60 - TIMER_STEP;
                }
           }
        }
        else if(buttonTimerDown.hold())
        {
            if(minutes > 0)
           {
            minutes -= TIMER_STEP;
            delay(TIMER_DELEY);
           }
           else
           {
                if(hours > 0)
                {
                    hours--;
                    minutes = 60 - TIMER_STEP;
                }
           }
        }
        disp_ta.displayClock(hours, minutes);   // Виведення часу на дисплей
    }

    // Функція відліку часу
    void timerCounting()
    {
        static unsigned long prevMillis = 0;
        static unsigned long interval = TIMER_MINUTE;   // Інтервал у мілісекундах

        buttonStart.tick();     // Опитування стану кнопки
        if(buttonStart.click() && hours + minutes != 0)
        {
            if (!timerRunning) 
            {    
                // Якщо таймер не запущено, то запускаємо його
                timerRunning = true;
            } 
            else 
            { 
                // Інакше зупиняємо таймер і зупиняємо навантаження
                timerRunning = false;
                digitalWrite(CHARGE, LOW);
            }
        }

        if (timerRunning) 
        {
            pollTemperatures();
            turnOffRele();
            if(hours + minutes == 0)
            {
                digitalWrite(CHARGE, LOW);
            }
            
            disp_ta.displayClock(hours, minutes);   // Виведення часу на дисплей
            
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
    }

    // Функція для встановлення температури
    void setTemperature(bool celsiusSign)
    {
        buttonTempUp.tick();    // Опитування стану кнопки температури +
        buttonTempDown.tick();  // Опитування стану кнопки температури -

        if (buttonTempUp.click())
        {
            temperature++;
        }
        else if (buttonTempUp.hold())
        {
            temperature += TEMP_STEP;
            delay(TEMP_DELAY);
        }
        if (temperature > TEMP_MAX)
        {
            temperature = TEMP_MIN;
        }
        if (buttonTempDown.click())
        {
            temperature--;
        }
        else if (buttonTempDown.hold())
        {
            temperature -= TEMP_STEP;
            delay(TEMP_DELAY);
        }
        if (temperature <= TEMP_MIN)
        {
            temperature = TEMP_MIN;
        }
        
        int temp = temperature;     // Перетворення значення температури на ціле число

        int tempone = temp / 10;    // Отримуємо десятки
        int temptwo = temp % 10;    // Отримуємо одиниці

        if(!celsiusSign)
        {
            disp_ds.display(3, temptwo);    // Виводимо одиниці
            delay(5);                       // додаємо невелику затримку
            disp_ds.display(2, tempone);    // выводимо десятки
        }
        else
        {
            disp_ds.display(1, temptwo);    // Виводимо одиниці
            delay(5);                       // додаємо невелику затримку
            disp_ds.display(0, tempone);    // выводимо десятки
            disp_ds.displayByte(2, 0x39);   // Підставляємо знак Цельсія
            delay(5);                       // додаємо невелику затримку
            disp_ds.displayByte(3, 0x63);   // Підставляємо знак градуса 
        }
    }

    // Функція для отримання температури з датчика
    void pollTemperatures()
    {
        static uint32_t timerTemp = millis();
        if(millis() - timerTemp >= TEMP_QUIZ)
        {
            sensors.requestTemperatures();          // Запитуємо дані з датчика
            tempC = sensors.getTempCByIndex(0);     // Зчитуємо температуру і зберегти її в змінній tempС

            /*---------НАЛАГОДЖЕННЯ---------*/
            Serial.print("Temperature: ");
            Serial.print(tempC);
            Serial.println("C");
            /*------------------------------*/

            timerTemp = millis();
        }
    }

    // Функція для вимкнення реле заряду при досягненні встановленої температури
    void turnOffRele() 
    {
        // Перевіряємо, чи поточна температура (tempC) перевищує або дорівнює заданій температурі (temperature)
        if(tempC >= temperature) 
        {
            digitalWrite(CHARGE, LOW);      // Якщо умова виконується, вимикаємо реле заряду (CHARGE)
        }

        // Перевіряємо, чи поточна температура (tempC) менша за задану температуру (temperature - TEMP_STEPON)
        if(tempC <= temperature - TEMP_STEPON) 
        {
            digitalWrite(CHARGE, HIGH);     // Якщо умова виконується, увімкнемо реле заряду (CHARGE)
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
