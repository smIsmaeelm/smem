# STM32 is a family of 32-bit microcontroller integrated circuits by STMicroelectronics.
This project is created to control two LEDs. Using ADC, the Micro read a photocell then it decide to light up or light down the first LED. The secound LED is turrned on manually, using UART. When a user send "M", Micro turn off the TIM2 and LED1, then turn on LED2.
