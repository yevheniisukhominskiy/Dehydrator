# Dryer - Vegetable Dehydrator
Dryer is an open-source project that aims to provide a solution for efficiently dehydrating vegetables. The project focuses on building a robust and user-friendly vegetable dehydrator using C++ and Arduino Framework.

## Content
1. [Features](https://github.com/yevheniisukhominskiy/Dryer#features)
2. [Prerequisites](https://github.com/yevheniisukhominskiy/Dryer#prerequisites)
3. [Device selection](https://github.com/yevheniisukhominskiy/Dryer#device-selection)
   1. [LGT8F328P](https://github.com/yevheniisukhominskiy/Dryer#lgt8g328p)
4. [We need](https://github.com/yevheniisukhominskiy/Dryer#we-need)
5. [Connection](https://github.com/yevheniisukhominskiy/Dryer#connection)
   1. [Display for sensor](https://github.com/yevheniisukhominskiy/Dryer#display-for-sensor)
      1. [Buttons for sensor](https://github.com/yevheniisukhominskiy/Dryer#buttons-for-sensor)
   2. [Display for timer](https://github.com/yevheniisukhominskiy/Dryer#display-for-timer)
      1. [Buttons for timer](https://github.com/yevheniisukhominskiy/Dryer#buttons-for-timer)
   3. [Control buttons](https://github.com/yevheniisukhominskiy/Dryer#control-buttons)
   4. [Sensor](https://github.com/yevheniisukhominskiy/Dryer#sensor)
   5. [Charge](https://github.com/yevheniisukhominskiy/Dryer#charge)
6. [Getting Started](https://github.com/yevheniisukhominskiy/Dryer#getting-started)
7. [Versions](https://github.com/yevheniisukhominskiy/Dryer#versions)
8.  [Plans](https://github.com/yevheniisukhominskiy/Dryer#plans)
9.  [License](https://github.com/yevheniisukhominskiy/Dryer#license)
10. [Contact](https://github.com/yevheniisukhominskiy/Dryer#contact)
11. [Denial of responsibility](https://github.com/yevheniisukhominskiy/Dryer#denial-of-responsibility)

## Features
* Efficient Dehydration: Dryer employs advanced techniques to effectively dehydrate vegetables, ensuring optimal preservation of nutrients while extending their shelf life.
  
* Temperature Control: The dehydrator is equipped with a precise temperature control system, allowing users to set and maintain the desired temperature for optimal dehydration.
  
* Adjustable Drying Time: Users can adjust the drying time according to the specific requirements of different vegetables, ensuring consistent results.
  
* Easy-to-Use Interface: Dryer incorporates a user-friendly interface, making it intuitive for both novice and experienced users to operate the dehydrator effortlessly.
  
* Error Handling: The system includes comprehensive error handling mechanisms to handle unexpected situations and ensure reliable and safe operation.

## Prerequisites
Before you begin, make sure you have the following prerequisites installed on your system:

* PlatformIO IDE: Install the [PlatformIO IDE](platformio.org), which is an extension for Visual Studio Code. Visual Studio Code is a popular code editor that provides a powerful and feature-rich environment for development. Install [Visual Studio Code]([platformio.org](https://code.visualstudio.com/)) and then search for the PlatformIO extension in the Visual Studio Code Marketplace.

* Arduino Board: Obtain an Arduino board compatible with the Vegetable Dehydrators project. The project may require a specific Arduino board model or a compatible variant.

* Git: Install Git on your system to clone the project repository and manage your code changes effectively.

## Device selection
The project was developed using the LGT8F328P-LQFP48 microcontroller as its foundation. This microcontroller offers a range of features and capabilities that make it suitable for the vegetable dehydrators project. **The use of other boards based on AVR is possible with errors. Be carefull!**

To select the device for the Vegetable Dehydrators project in PlatformIO, follow these steps:
1. Open the platformio.ini file.
2. Paste the code based on your device.

### LGT8F328P
```ini
[env:lgt8f328p-LQFP48]
platform = lgt8f
board = lgt8f328p-LQFP48
framework = arduino
lib_deps = 
	gyverlibs/EncButton@^2.0
	gyverlibs/GyverTM1637@^1.4.2
	milesburton/DallasTemperature@^3.11.0
	gyverlibs/GyverTimers@^1.10
```
**Expect other microcontrollers in future releases.**

## We need
**Modules**
* 1x Arduino Board: **LGT8F328P**
* 2x Digital Display: **TM1637**
* 1x Temperature Sensor: **DS18B20**
* 6x Button Interface:	**TACT 12x12**
* 8x LED Indicator: **600-800 mCd**

**Libraries**
* EncButton
* GyverTM1637
* DallasTemperature
* GyverTimers

PlatformIO will automatically download and install the specified libraries and their dependencies when you build your project. The main thing is to save the file `platformio.ini`

## Connection

### Display for sensor
| **TM1637** | **LGT8F328P** |
|------------|---------------|
| CLK    	 | D4        	 |
| DIO    	 | D3        	 |
| GND    	 | GND       	 |
| 5V     	 | 5V        	 |

### Buttons for sensor
| **BUTTON-PLUS** | **LGT8F328P** | **BUTTON-MINUS** | **LGT8F328P** |
|-----------------|---------------|------------------|---------------|
| 5V              | D5            | 5V               | D6            |
| GND             | GND           | GND              | GND           |

### Display for timer
| **TM1637** | **LGT8F328P** |
|------------|---------------|
| CLK        | D13       	 |
| DIO    	 | D2        	 |
| GND    	 | GND      	 |
| 5V     	 | 5V       	 |

### Buttons for timer
| **BUTTON-PLUS** | **LGT8F328P** | **BUTTON-MINUS** | **LGT8F328P** |
|-----------------|---------------|------------------|---------------|
| 5V              | D7            | 5V               | D8            |
| GND             | GND           | GND              | GND           |

### Control buttons
| **BUTTON-MODE** | **LGT8F328P** | **BUTTON-START** | **LGT8F328P** |
|-----------------|---------------|------------------|---------------|
| 5V              | D9            | 5V               | D10           |
| GND             | GND           | GND              | GND           |

### Sensor
| **DS18B20** | **LGT8F328P** |
|-------------|---------------|
| VCC         | 5V            |
| DQ          | D12           |
| GND         | GND           |

**The only thing that needs to be added from the external additional strapping is a 4.7 kΩ pull-up resistor.**

### Charge
| **RELE** | **LGT8F328P** |
|----------|---------------|
| 5V       | 5V            |
| GND      | GND           |

**Also put a 220Ω resistor.**


## Getting Started
To get started with the dryer project, follow these steps:

1. Clone the repository:  
    `git clone https://github.com/yevheniisukhominskiy/dryer.git`
2. Open PlatformIO IDE: Launch Visual Studio Code and open the PlatformIO IDE. You can find the PlatformIO IDE extension in the left sidebar of Visual Studio Code.
3. Configure the Project: After opening the project, you need to configure it for the specific Arduino board you are using. Open the _platformio.ini_ file in the root of your project directory. Set the board type and other configuration options according to the requirements of the Vegetable Dehydrators project.
4. Install the necessary dependencies mentioned.
5. Build and Upload the Project: Connect your Arduino board to your computer using a USB cable. In the PlatformIO IDE, click on the "Build" button to compile the project code. Once the compilation is successful, click on the "Upload" button to flash the compiled code onto the Arduino board.

## Versions
* v. 1.0 - major release.

## Plans
- [ ] Create a graphic version of connecting modules.
- [ ] Add multi-platform AVR and STM microcontrollers
- [ ] Optimize the project by using own libraries

## License
dryer is released under the [MIT License](https://github.com/yevheniisukhominskiy/dryer/blob/main/LICENSE.txt). You are free to use, modify, and distribute the project as per the terms of the license.

## Contact
If you have any questions, suggestions, or feedback, please feel free to reach out to the project maintainer, Yevhenii Sukhominskiy, at yevhenii.sukhominskiy@gmail.com.

## Denial of responsibility
All information is for personal use only.
The team does not accept any responsibility for any damage that may be caused in any form due to the use, incompleteness or incorrectness of the information posted on this project.

**I am hope you find the dryer project useful and efficient in your vegetable dehydration endeavors!**
