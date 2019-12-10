/************************************************************************************//**
* \file         Demo/ARMCM0_STM32G0_Nucleo_G071RB_TrueStudio/Boot/main.c
* \brief        Bootloader application source file.
* \ingroup      Boot_ARMCM0_STM32G0_Nucleo_G071RB_TrueStudio
* \internal
*----------------------------------------------------------------------------------------
*                          C O P Y R I G H T
*----------------------------------------------------------------------------------------
*   Copyright (c) 2019  by Feaser    http://www.feaser.com    All rights reserved
*
*----------------------------------------------------------------------------------------
*                            L I C E N S E
*----------------------------------------------------------------------------------------
* This file is part of OpenBLT. OpenBLT is free software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License as published by the Free
* Software Foundation, either version 3 of the License, or (at your option) any later
* version.
*
* OpenBLT is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
* PURPOSE. See the GNU General Public License for more details.
*
* You have received a copy of the GNU General Public License along with OpenBLT. It 
* should be located in ".\Doc\license.html". If not, contact Feaser to obtain a copy.
*
* \endinternal
****************************************************************************************/

/****************************************************************************************
* Include files
****************************************************************************************/
#include "boot.h"                                /* bootloader generic header          */
#include "stm32g0xx.h"                           /* STM32 CPU and HAL header           */
#include "stm32g0xx_ll_rcc.h"                    /* STM32 LL RCC header                */
#include "stm32g0xx_ll_bus.h"                    /* STM32 LL BUS header                */
#include "stm32g0xx_ll_system.h"                 /* STM32 LL SYSTEM header             */
#include "stm32g0xx_ll_utils.h"                  /* STM32 LL UTILS header              */
#include "stm32g0xx_ll_usart.h"                  /* STM32 LL USART header              */
#include "stm32g0xx_ll_gpio.h"                   /* STM32 LL GPIO header               */


/****************************************************************************************
* Function prototypes
****************************************************************************************/
static void Init(void);
static void SystemClock_Config(void);


/************************************************************************************//**
** \brief     This is the entry point for the bootloader application and is called
**            by the reset interrupt vector after the C-startup routines executed.
** \return    Program return code.
**
****************************************************************************************/
int main(void)
{
  /* initialize the microcontroller */
  Init();
  /* initialize the bootloader */
  BootInit();

  /* start the infinite program loop */
  while (1)
  {
    /* run the bootloader task */
    BootTask();
  }

  /* program should never get here */
  return 0;
} /*** end of main ***/


/************************************************************************************//**
** \brief     Initializes the microcontroller.
** \return    none.
**
****************************************************************************************/
static void Init(void)
{
  /* HAL library initialization */
  HAL_Init();
  /* configure system clock */
  SystemClock_Config();
} /*** end of Init ***/


/************************************************************************************//**
** \brief     System Clock Configuration. This code was created by CubeMX and configures
**            the system clock to match the configuration in the bootloader's
**            configuration (blt_conf.h), specifically the macros:
**            BOOT_CPU_SYSTEM_SPEED_KHZ and BOOT_CPU_XTAL_SPEED_KHZ.
**            Note that the Lower Layer drivers were selected in CubeMX for the RCC
**            subsystem.
** \return    none.
**
****************************************************************************************/
static void SystemClock_Config(void)
{
  /* Configure the main internal regulator output voltage */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Set flash latency. */
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_2);
  /* Verify flash latency setting. */
  if(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_2)
  {
    /* Error setting flash latency. */
    ASSERT_RT(BLT_FALSE);
  }

  /* Configure and enable HSI */
  LL_RCC_HSI_Enable();
  while(LL_RCC_HSI_IsReady() != 1)
  {
  }

  /* Configure and enable main PLL */
  /*
   * PLL configuration is based on HSI/4 (4 MHz) input clock and a VCO
   * frequency equal to four times the required output frequency (which
   * must be an exact multiple of 1 MHz in the range 16..64 MHz).
   *
   * Note: although the PLL ADC/I2S1 and RNG/TIM1 domain outputs are not
   * required by the boot loader, if the application initialises the PLL
   * dividers (P, Q) for these outputs to non-default values, they should
   * also be initialised here to the same values used by the application.
   * Otherwise, the application clock initialisation may fail.
   *
   * (The STM LL API for PLL configuration seems particularly clunky,
   * requiring three calls which must be consistent in the duplicated
   * arguments.)
   */
#define PLL_CLK_SPEED_KHZ   (HSI_VALUE / (4u * 1000u))

#define PLL_N_VALUE         (4u * (BOOT_CPU_SYSTEM_SPEED_KHZ / \
                                   PLL_CLK_SPEED_KHZ))
  
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI,
                              LL_RCC_PLLM_DIV_4,
                              PLL_N_VALUE, 
                              LL_RCC_PLLR_DIV_4);

  LL_RCC_PLL_ConfigDomain_ADC(LL_RCC_PLLSOURCE_HSI,
                              LL_RCC_PLLM_DIV_4,
                              PLL_N_VALUE, 
                              LL_RCC_PLLP_DIV_4);

  LL_RCC_PLL_ConfigDomain_TIM1(LL_RCC_PLLSOURCE_HSI,
                               LL_RCC_PLLM_DIV_4,
                               PLL_N_VALUE, 
                               LL_RCC_PLLQ_DIV_4);

  LL_RCC_PLL_Enable();
  LL_RCC_PLL_EnableDomain_SYS();
  while(LL_RCC_PLL_IsReady() != 1)
  {
  }

  /* Configure SYSCLK source from the main PLL */
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {
  }

  /* Set AHB prescaler*/
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);

  /* Set APB1 prescaler */
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);

  /* Update CMSIS system core clock speed */
  LL_SetSystemCoreClock(BOOT_CPU_SYSTEM_SPEED_KHZ * 1000u);
} /*** end of SystemClock_Config ***/


/************************************************************************************//**
** \brief     Initializes the Global MSP. This function is called from HAL_Init()
**            function to perform system level initialization (GPIOs, clock, DMA,
**            interrupt).
** \return    none.
**
****************************************************************************************/
void HAL_MspInit(void)
{
  LL_GPIO_InitTypeDef GPIO_InitStruct;

  /* SYSCFG clock enable. */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
  
  /* Flash clock enable */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_FLASH);

  /* GPIO ports clock enable. */
  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOC);

#if (BOOT_COM_UART_ENABLE > 0)
  /* UART clock enable. */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);
#endif

  /* Configure GPIO pin for the LED. */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_5;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_5);

  /* Configure GPIO pin for (optional) backdoor entry input. */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_13;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOC, &GPIO_InitStruct);

#if (BOOT_COM_UART_ENABLE > 0)
  /* UART TX and RX GPIO pin configuration. */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_2 | LL_GPIO_PIN_3;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_1;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
#endif
} /*** end of HAL_MspInit ***/


/************************************************************************************//**
** \brief     DeInitializes the Global MSP. This function is called from HAL_DeInit()
**            function to perform system level de-initialization (GPIOs, clock, DMA,
**            interrupt).
** \return    none.
**
****************************************************************************************/
void HAL_MspDeInit(void)
{
  /* Reset GPIO pin for the LED to turn it off. */
  LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_5);

  /* Deinit used GPIOs. */
  LL_GPIO_DeInit(GPIOC);
  LL_GPIO_DeInit(GPIOA);

#if (BOOT_COM_UART_ENABLE > 0)
  /* UART clock disable. */
  LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_USART2);
#endif

  /* GPIO ports clock disable. */
  LL_IOP_GRP1_DisableClock(LL_IOP_GRP1_PERIPH_GPIOC);
  LL_IOP_GRP1_DisableClock(LL_IOP_GRP1_PERIPH_GPIOA);

  /* Flash clock disable */
  LL_AHB1_GRP1_DisableClock(LL_AHB1_GRP1_PERIPH_FLASH);

  /* SYSCFG clock disable. */
  LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
} /*** end of HAL_MspDeInit ***/


/*********************************** end of main.c *************************************/