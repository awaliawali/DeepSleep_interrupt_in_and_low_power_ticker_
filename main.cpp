





/*
 * Mbed Application program
 *  Check Deep Sleep Mode
 *
 * Copyright (c) 2020,'21 Kenji Arai / JH1PJL
 *  http://www7b.biglobe.ne.jp/~kenjia/
 *  https://os.mbed.com/users/kenjiArai/
 *      Revised:    March     12th, 2020
 *      Revised:    January   15th, 2021
 */

/*
    Reference information:
        https://os.mbed.com/docs/mbed-os/v6.6/apis/power-management-sleep.html
        https://forums.mbed.com/t/how-to-deep-sleep/7551

    Tested on with mbed-os5.15.1
        https://os.mbed.com/users/kenjiArai/code/Check_DeepSleep_os5/
        Nucleo-L152RE       -> 4.23uA (Normal run = 5mA to 8mA)
        Nucleo-L476RG       -> 2.13uA (Normal run = 7mA to 10mA)
        Nucleo-F411RE       -> 1.91mA (not uA)(Normal run = 7mA to 10mA)
        Nucleo-F446RE       -> 1.67mA (not uA)(Normal run = 14mA to 17mA)
    Tested on with mbed-os6.6.0
        Nucleo-L152RE       -> 4.22uA (Normal run = 4mA to 7mA)
        Nucleo-L476RG       -> 2.23uA (Normal run = 7mA to 10mA)
        Nucleo-F411RE       -> 1.65mA (not uA)(Normal run = 7mA to 10mA)
        Nucleo-F446RE       -> 1.76mA (not uA)(Normal run = 14mA to 16mA)

        Current Measurement:
         Nucleo board has IDD Jumper (JP6).
         I measured CPU current using Digital Multi-meter DCI mode.
 */

//  Include --------------------------------------------------------------------
#include "mbed.h"

//  Constructor ----------------------------------------------------------------
DigitalIn   my_sw(BUTTON1);//PC_13
LowPowerTicker lpTicker;

DigitalOut  myled(D13);
static BufferedSerial pc(USBTX, USBRX, 9600);
AnalogIn    a_in(A0);
float ain;
int flag_warning_datchik;

//  Function prototypes --------------------------------------------------------
void sw_irq(void);
void LowPowerConfiguration(void);

void tickerIRQ (void)
{
         system_reset();
}

void print_datchik(void)
{
     ain = a_in.read();
    printf("analog = %4.3f, \r\n", ain );  
  myled = 1; 
   ThisThread::sleep_for(3s);
   myled = 0;  
}

void sw_irq(void)
{
      system_reset();
}

int main()
{
  //  my_sw.mode(PullUp);
    printf("\r\n Reset \r\n");
   myled = 0;
   lpTicker.attach(tickerIRQ, 10); // every 10 second


 //если от lpTicker  либо от кнопки пришло прерывание
// то распечатать в консоль показания АЦП С А0 (это покажет что он вышел из глубокого сна)
//зажечь светодиод на 3сек
      print_datchik();

// уходит в глубокий сон
     printf("\r\n Deep-sleep mode...to exit deep sleep wait 10s or push User button\r\n");
            LowPowerConfiguration();
            InterruptIn my_irq(BUTTON1);//PC_13);
     while (my_irq.read() == 0) {;} //ждет пока отпустится кнопка чтобы начать сначала
            ThisThread::sleep_for(100ms);
            my_irq.fall(sw_irq); // назначаем обработчик - работает по спаду импульса

            //------------ IMPORTANT-------------------
            // 1) removes the receive interrupt handler
            pc.enable_input(false);
            // 2) and releases the deep sleep lock
            sleep_manager_can_deep_sleep();
            // 3) then enter Deep Sleep mode

//            ThisThread::sleep_for(10s);
while(1)
    {
        myled = 0;
      ThisThread::sleep_for(10s);
    }

}





void LowPowerConfiguration(void)
{
#if defined(TARGET_NUCLEO_L152RE)
    RCC->AHBENR |=
        (RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN |
         RCC_AHBENR_GPIODEN | RCC_AHBENR_GPIOHEN);
#elif defined(TARGET_NUCLEO_L476RG)

#elif defined(TARGET_NUCLEO_F446RE)
    RCC->AHB1ENR |=
        (RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN |
         RCC_AHB1ENR_GPIODEN | RCC_AHB1ENR_GPIOHEN);
#endif
    GPIO_InitTypeDef GPIO_InitStruct;
    // All other ports are analog input mode
    GPIO_InitStruct.Pin = GPIO_PIN_All;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
    HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);
#if defined(TARGET_NUCLEO_L152RE)
    RCC->AHBENR &=
        ~(RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN |RCC_AHBENR_GPIOCEN |
          RCC_AHBENR_GPIODEN | RCC_AHBENR_GPIOHEN);
#elif defined(TARGET_NUCLEO_L476RG)
    RCC->AHB1ENR = 0;
    RCC->AHB2ENR = 0;
    RCC->AHB3ENR = 0;
    RCC->APB1ENR2 = 0;
    RCC->APB2ENR = 0;
#elif defined(TARGET_NUCLEO_F446RE)
    RCC->AHB1ENR = 0;
    RCC->AHB2ENR = 0;
    RCC->AHB3ENR = 0;
    RCC->APB1ENR = 0x8;     // alive TIM5
    RCC->APB2ENR = 0;
    RCC->AHB1LPENR = 0;
    RCC->AHB2LPENR = 0;
    RCC->APB1LPENR = 0x8;   // alive TIM5
    RCC->APB2LPENR = 0;
#endif
}