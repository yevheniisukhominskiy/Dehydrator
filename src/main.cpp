#include <Arduino.h>
#include <EncButton.h>
#include <GyverTM1637.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <GyverTimers.h>

/*--------------НАЛАШТУВАННЯ ПІНІВ ІНІЦІАЛІЗАЦІЇ--------------*/
#define CLK_DS 4           // CLK датчик
#define DIO_DS 3           // DIO датчик
#define CLK_TA 13           // CLK таймер
#define DIO_TA 2           // DIO таймер
#define CHARGE 11            // Навантаження реле
#define DS18B20 12       // Датчик

const int ledPins[] = {14, 15, 16, 17, 18, 19, 20, 21};
const int numLeds = sizeof(ledPins) / sizeof(ledPins[0]);
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

/*---------------------АВТОМАТИЧНІ РЕЖИМИ---------------------*/
enum modesTemperature
{
    APPLES,     // Яблука
    PEARS,      // Груші
    CHERRY,     // Вишні
    BERRIES,    // Ягоди
    MUSHROOMS,  // Гриби
    MEAT,       // М'ясо
    HERBS,      // Трави
    ROSEHIP     // Шипшина
};
/*------------------------------------------------------------*/

class Dryer
{
private:
    /*-----------------НАЛАШТУВАННЯ ОБ'ЄКТІВ КНОПОК-----------------*/  
    EncButton<EB_TICK, 5> buttonTempUp;     // Температура додавання    
    EncButton<EB_TICK, 6> buttonTempDown;   // Температура віднімання 
    EncButton<EB_TICK, 7> buttonTimerUp;    // Таймер додавання 
    EncButton<EB_TICK, 8> buttonTimerDown;  // Таймер віднімання
    EncButton<EB_TICK, 9> buttonMode;       // Вибір режиму
    EncButton<EB_TICK, 10> buttonStart;      // Старт/стоп/блокування
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
    bool setBlock;      // Блокування для зміни значень
    bool modeSection;   // Вибір режиму авто-руч
    int activeLed;      // Активний світлодіод
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
        celsiusSign(false),
        setBlock(false),
        modeSection(true),                     // true - AUTOMATIC, false - MANUAL
        activeLed(0)
    {}

     void setup() 
    {
        Serial.begin(9600);                     // Монітор порта для налагодження
        sensors.begin();                        // Визов функції об'єкту 
        pinMode(CHARGE, OUTPUT);                // Ініціалізація вихідного навантаження
        disp_ds.brightness(TEMP_BRIGHTMESS);    // Яскравість дисплею температури
        disp_ta.brightness(TIMER_BRIGHTMESS);   // Яскравість дисплею таймеру
        for (int i = 0; i < numLeds; i++)       // Ініціалізації ствітлодіодів
        {
            pinMode(ledPins[i], OUTPUT);
            digitalWrite(ledPins[i], LOW);
        }
    }

    void loop() 
    {
        // Блокування по кнопці старту (при тривалому натисканні)
        if(setBlock == false)
        {
            setTimer();                     // Функція для встановлення часу на таймері
            modeSelection();                // Функція для встановлення температури автоматичний (режими)
        }
        timerCounting();    // Функція відліку часу
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

        // Флаг для блокування зміни значень
        if(buttonStart.held() && hours + minutes != 0)
        {
            setBlock = !setBlock;
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

    // Функція вибору рижиму авто/руч
    void modeSelection()
    {
        buttonMode.tick();

        if(buttonMode.held())
        {
            modeSection = !modeSection;
        }

        if(modeSection == true)
        {
            setTemperatureAuto(celsiusSign);           // Функція для встановлення температури автоматичний
        }
        else
        {
            setTemperatureManual(celsiusSign);    // Функція для встановлення температури ручний
        }
    }

    void setTemperatureAuto(bool celsiusSign)
    {
        if (buttonMode.click()) 
        {
            digitalWrite(ledPins[activeLed], LOW);      // Вимикаємо попередній світлодіод
            activeLed = (activeLed + 1) % numLeds;      // Збільшуємо індекс активного світлодіода
            digitalWrite(ledPins[activeLed], HIGH);     // Збільшуємо індекс активного світлодіода

            switch (activeLed)
            {
            case modesTemperature::APPLES:
                Serial.println("Режим: Яблука");
                temperature = 70;
                break;
            case modesTemperature::PEARS:
                Serial.println("Режим: Груши");
                temperature = 65;
                break;
            case modesTemperature::CHERRY:
                Serial.println("Режим: Вишні");
                temperature = 70;
                break;
            case modesTemperature::BERRIES:
                Serial.println("Режим: Ягоди");
                temperature = 50;
                break;
            case modesTemperature::MUSHROOMS:
                Serial.println("Режим: Гриби");
                temperature = 50;
                break;
            case modesTemperature::MEAT:
                Serial.println("Режим: М'ясо");
                temperature = 45;
                break;
            case modesTemperature::HERBS:
                Serial.println("Режим: Трави");
                temperature = 35;
                break;
            case modesTemperature::ROSEHIP:
                Serial.println("Режим: Шипшина");
                temperature = 55;
                break;
            default:
                Serial.println("Помилка вибору режиму!");
                temperature = TEMP_MIN;
                break;
            }
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

    // Функція для встановлення температури
    void setTemperatureManual(bool celsiusSign)
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
