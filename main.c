#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_gpio.h"
#include "stm32f0xx_ll_rtc.h"
#include "stm32f0xx_ll_pwr.h"
#include "stm32f0xx_ll_tim.h"
#include "stm32f0xx_ll_exti.h"
#include "xprintf.h"
#include "oled_driver.h"
#include "time.h"
#include "donov.h"
#define LL_TIM_OCMODE_PWM1  (TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1)
#define point 10

/**
  * System Clock Configuration
  * The system Clock is configured as follow :
  *    System Clock source            = PLL (HSI/2)
  *    SYSCLK(Hz)                     = 48000000
  *    HCLK(Hz)                       = 48000000
  *    AHB Prescaler                  = 1
  *    APB1 Prescaler                 = 1
  *    HSI Frequency(Hz)              = 8000000
  *    PLLMUL                         = 12
  *    Flash Latency(WS)              = 1
  */


//put tactirovanie
static void rcc_config()
{
    /* Set FLASH latency */
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);

    /* Enable HSI and wait for activation*/
    LL_RCC_HSI_Enable();
    while (LL_RCC_HSI_IsReady() != 1);

    /* Main PLL configuration and activation */
    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI_DIV_2,
                                LL_RCC_PLL_MUL_12);

    LL_RCC_PLL_Enable();
    while (LL_RCC_PLL_IsReady() != 1);

    /* Sysclk activation on the main PLL */
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL);

    /* Set APB1 prescaler */
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);

    /* Update CMSIS variable (which can be updated also
     * through SystemCoreClockUpdate function) */
    SystemCoreClock = 48000000;
}
//configure ports
static void gpio_config(void)
{
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_8, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_9, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_8, LL_GPIO_MODE_OUTPUT);
    //buttons
    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_14, LL_GPIO_MODE_INPUT);
    LL_GPIO_SetPinPull(GPIOC, LL_GPIO_PIN_14, LL_GPIO_PULL_UP);
    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_7, LL_GPIO_MODE_INPUT);
    LL_GPIO_SetPinPull(GPIOC, LL_GPIO_PIN_7, LL_GPIO_PULL_UP);
    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_2, LL_GPIO_MODE_INPUT);
    LL_GPIO_SetPinPull(GPIOC, LL_GPIO_PIN_2, LL_GPIO_PULL_UP);
    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_3, LL_GPIO_MODE_INPUT);
    LL_GPIO_SetPinPull(GPIOC, LL_GPIO_PIN_3, LL_GPIO_PULL_UP);

    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_0, LL_GPIO_MODE_INPUT);
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_5, LL_GPIO_MODE_ALTERNATE);
  	LL_GPIO_SetAFPin_0_7(GPIOA, LL_GPIO_PIN_5, LL_GPIO_AF_2);
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_10, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_11, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_12, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_13, LL_GPIO_MODE_OUTPUT);
    return;
}
//display printf config
static void printf_config(void)
{
   xdev_out(oled_putc);
   return;
}
//timers configuration
static void timers_config(void)
{
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
  LL_TIM_SetPrescaler(TIM2, 47999);
  LL_TIM_SetAutoReload(TIM2, 999);
  LL_TIM_OC_SetCompareCH1(TIM2,499);
  LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH1);
  LL_TIM_OC_SetPolarity(TIM2, LL_TIM_CHANNEL_CH1, LL_TIM_OCPOLARITY_HIGH);
  LL_TIM_OC_SetMode(TIM2, LL_TIM_CHANNEL_CH1, LL_TIM_OCMODE_PWM1);
  LL_TIM_SetCounterMode(TIM2, LL_TIM_COUNTERMODE_UP);
  LL_TIM_EnableIT_CC1(TIM2);
  LL_TIM_EnableCounter(TIM2);
  NVIC_EnableIRQ(TIM2_IRQn);
  NVIC_SetPriority(TIM2_IRQn, 1);
  return;
}
//handlers
void TIM2_IRQHandler(void)
{
  LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_5);
  LL_TIM_ClearFlag_CC1(TIM2);
}
//delay
__attribute__((naked)) static void delay(void)
{
asm ("push {r7, lr}");
asm ("ldr r6, [pc, #8]");
asm ("sub r6, #1");
asm ("cmp r6, #0");
asm ("bne delay+0x4");
asm ("pop {r7, pc}");
asm (".word 0x1F4");
//delay
}

void ENDGAME(int* ctr, int* counter)
{
    if(ctr == point){
    oled_update();
    oled_set_cursor(0, 0);
    xprintf("\n    Congratulations,\n\n    you've stayed\n\n       alive!\n\n");
    oled_update();
    LL_TIM_DisableCounter(TIM2);
    rcc_config();
    oled_config();
    oled_pic(donov, 100);
    oled_update();
}
    if(counter == 1)
    {
      rcc_config();
      oled_config();
      printf_config();
      oled_set_cursor(53, 32);
      xprintf(" ");
      oled_update();
      rcc_config();
      oled_config();
      printf_config();
      oled_set_cursor(0, 0);
      xprintf("     Ti opozdal!\n\n      Otchislen!\n\n     Pishi PSJ!");
      oled_update();
      while(1){
        LL_TIM_SetPrescaler(TIM2, 239);
        LL_TIM_OC_SetCompareCH1(TIM2,15);
      }
    }

    LL_GPIO_TogglePin(GPIOA, LL_GPIO_PIN_10);
    LL_GPIO_TogglePin(GPIOA, LL_GPIO_PIN_11);
    LL_GPIO_TogglePin(GPIOA, LL_GPIO_PIN_12);
    LL_GPIO_TogglePin(GPIOA, LL_GPIO_PIN_13);
  }

int counter = 100;

main(void)
{
  gpio_config();
  timers_config();


      /////////////////////////
     //////////////////////////

  int but_USER = 0;
  int but_0 = 0;
  int but_1 = 0;
  int but_2 = 0;
  int but_3 = 0;
  int butcount_0 = 0;
  int butcount_1 = 0;
  int butcount_2 = 0;
  int butcount_3 = 0;
  int butUSERcount = 0;
  int press_num = 1;
  int count = 0;
  int ctr = 0;
  int reset_flag = 0;

  while(1){

       if ((LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_14) == 1)||
           (LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_7 ) == 1)||
           (LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_2 ) == 1)||
           (LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_3 ) == 1))
       {
         count++;
         if(count >= 5)
         {
            butcount_0 = 0;
            butcount_1 = 0;
            butcount_2 = 0;
            butcount_3 = 0;
            count = 0;
         }
         if (!LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_3))
         {
           butcount_3++;
           ctr = ctr + 4;
           if(butcount_3 == 2)
           {
            but_3 = 1;
            LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_8);
           }
           delay();
           if (LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_3))
           {
             butcount_3 = 0;
             but_3 = 0;
           }
         }
         if (!LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_2))
         {
            butcount_2++;
            if(butcount_2 == 2)
            {
              but_2 = 1;
              ctr = ctr + 3;
              LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_8);
            }
            delay();
            if (LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_2))
            {
              butcount_2 = 0;
              but_2 = 0;
            }
          }
          if (!LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_7))
          {
            butcount_1++;
            if(butcount_1 == 2)
            {
              but_1 = 1;
              ctr = ctr + 2;
              LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_8);
            }
            delay();
            if (LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_7))
            {
              butcount_1 = 0;
              but_1 = 0;
            }
          }
          if (!LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_14))
          {
            butcount_0++;
            if(butcount_0 == 2)
            {
              but_0 = 1;
              ctr = ctr + 1;
              LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_8);
            }
            delay();
            if (LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_14))
            {
              butcount_0 = 0;
              but_0 = 0;
            }
          }
          if (ctr == point)
          {
            rcc_config();
            oled_config();
            printf_config();
            oled_set_cursor(53, 32);
            xprintf(" ");
            oled_update();
            break;
          }
          if (ctr > point)
          {
            rcc_config();
            oled_config();
            printf_config();
            oled_set_cursor(53, 32);
            xprintf(" ");
            oled_set_cursor(0, 0);
            xprintf("Too big number.\n\n\nHold USER button");
            reset_flag = 1;
            oled_update();
            butcount_0 = 0;
            butcount_1 = 0;
            butcount_2 = 0;
            butcount_3 = 0;
            count = 0;
          }
          if(butUSERcount >= 2)
          {
          butUSERcount--;
          }
          if (LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_0))
          {
            butUSERcount++;
            if(butUSERcount == 2)
            {
              but_USER = 1;
            }
            delay();
            if (!LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_0))
            {
              butUSERcount = 0;
              but_USER = 0;
            }
          }
          else
          {
            but_USER = 0;
          }
          if (!LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_0))
          {
            butcount_1++;
            if(butcount_1 == 2)
            {
              but_1 = 1;
            }
            delay();
            if (LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_0))
            {
              butcount_1 = 0;
              but_1 = 0;
            }
          }
          if (but_USER)
          {
            LL_TIM_SetPrescaler(TIM2, 239);
            LL_TIM_OC_SetCompareCH1(TIM2,15);
            ctr = 0;
            reset_flag = 0;
            but_USER = 0;
          }
          else
          {
            if(reset_flag == 0)
            {
               LL_TIM_SetPrescaler(TIM2, 47999);
               LL_TIM_OC_SetCompareCH1(TIM2,499);
               rcc_config();
               oled_config();
               printf_config();
               oled_set_cursor(53, 32);
               xprintf("%d", counter);
               oled_update();
               counter--;
               oled_update();
            }
            else
            {
               LL_TIM_SetPrescaler(TIM2, 47999);
               LL_TIM_OC_SetCompareCH1(TIM2,499);
               counter--;
            }
            if(counter == 1)
            {
               oled_set_cursor(53, 32);
               rcc_config();
               oled_config();
               printf_config();
               oled_set_cursor(53, 32);
               xprintf(" ");
               oled_update();
               break;
             }
           }
         }
       }
  ENDGAME(ctr, counter);


  return 0;
}
