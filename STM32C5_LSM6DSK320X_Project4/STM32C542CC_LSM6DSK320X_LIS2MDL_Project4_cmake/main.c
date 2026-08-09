/**
  ******************************************************************************
  * file           : main.c
  * brief          : Main program body
  *                  Calls target system initialization then loop in main.
  ******************************************************************************
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private functions prototype -----------------------------------------------*/

#include "mx_usart1.h"
#include <stdio.h>
#include <string.h>

#include "lsm6dsv320x_reg.h"

int _write(int file, char *ptr, int len)
{
    hal_uart_handle_t *huart1 = mx_usart1_uart_gethandle();

    if (huart1 != NULL)
    {
        HAL_UART_Transmit(huart1, ptr, len, 1000);
    }

    return len;
}


/* Private macro -------------------------------------------------------------*/
#define    BOOT_TIME            10 //ms


/* Private variables ---------------------------------------------------------*/
static uint8_t whoamI;


/* Extern variables ----------------------------------------------------------*/


/* Private functions ---------------------------------------------------------*/

/*
 *   WARNING:
 *   Functions declare in this section are defined at the end of this file
 *   and are strictly related to the hardware platform used.
 *
 */
static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp,
                              uint16_t len);
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len);
static void platform_delay(uint32_t ms);


static stmdev_ctx_t dev_ctx;
static volatile uint8_t thread_wake = 0;


/**
 * @brief  EXTI trigger callback
 */
void HAL_EXTI_TriggerCallback(hal_exti_handle_t *hexti,
                              hal_exti_trigger_t trigger)
{
    if (HAL_EXTI_GetInstance(hexti) == HAL_EXTI_GPIO_0)
    {
        thread_wake = 1;
    }
}



/**
  * brief:  The application entry point.
  * retval: none but we specify int to comply with C99 standard
  */
int main(void)
{
  /** System Init: this code placed in targets folder initializes your system.
    * It calls the initialization (and sets the initial configuration) of the peripherals.
    * You can use STM32CubeMX to generate and call this code or not in this project.
    * It also contains the HAL initialization and the initial clock configuration.
    */
  if (mx_system_init() != SYSTEM_OK)
  {
    return (-1);
  }
  else
  {
    /*
      * You can start your application code here
      */

	  printf("HELLO\n");
	  HAL_GPIO_WritePin(CS1_PORT, CS1_PIN, HAL_GPIO_PIN_SET);
	  HAL_GPIO_WritePin(SA0_PORT, SA0_PIN, HAL_GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(CS2_PORT, CS2_PIN, HAL_GPIO_PIN_SET);

      lsm6dsv320x_stpcnt_mode_t pedometer = {0};
      lsm6dsv320x_pin_int_route_t pin_int = {0};
      lsm6dsv320x_if_cfg_t if_cfg = {0};

      uint16_t step_count = 0;


      /* Initialize mems driver interface */
      dev_ctx.write_reg = platform_write;
      dev_ctx.read_reg = platform_read;
      dev_ctx.mdelay = platform_delay;
      dev_ctx.handle = mx_i2c1_i2c_gethandle();


      /* Wait sensor boot time */
      platform_delay(BOOT_TIME);


      /* Check device ID */
      lsm6dsv320x_device_id_get(&dev_ctx, &whoamI);

      printf("LSM6DSK320X_ID=0x%x,id=0x%x\n",
             0x75, whoamI);

      if (whoamI != 0x75)
        while (1);


      /* Perform device power-on-reset */
      lsm6dsv320x_sw_por(&dev_ctx);


      /* Enable Block Data Update */
      lsm6dsv320x_block_data_update_set(&dev_ctx,PROPERTY_ENABLE);


      /*
       * Pedometer works internally at 30 Hz.
       * Low-G accelerometer ODR must be >= 30 Hz.
       */
      lsm6dsv320x_xl_setup(&dev_ctx,LSM6DSV320X_ODR_AT_30Hz,LSM6DSV320X_XL_HIGH_PERFORMANCE_MD);

      /* Set Low-G accelerometer full scale */
      lsm6dsv320x_xl_full_scale_set(&dev_ctx,LSM6DSV320X_2g);


      /* INT1/INT2: open-drain + active low */
      lsm6dsv320x_read_reg(&dev_ctx,LSM6DSV320X_IF_CFG,(uint8_t *)&if_cfg,1);

      if_cfg.pp_od = 1;
      if_cfg.h_lactive = 1;

      lsm6dsv320x_write_reg(&dev_ctx,LSM6DSV320X_IF_CFG,(uint8_t *)&if_cfg,1);


      /* Enable pedometer and step counter */
      pedometer.step_counter_enable = PROPERTY_ENABLE;

      /*
       * False-positive rejection disabled.
       * Basic pedometer algorithm is used in this demo.
       */
      pedometer.false_step_rej = PROPERTY_DISABLE;

      lsm6dsv320x_stpcnt_mode_set(&dev_ctx,pedometer);


      /*
       * Set pedometer debounce.
       *
       * Default value = 10 steps.
       * Set to 3 steps for easier demonstration.
       */
      lsm6dsv320x_stpcnt_debounce_set(&dev_ctx,3);

      /* Reset step counter */
      lsm6dsv320x_stpcnt_rst_step_set(&dev_ctx,PROPERTY_ENABLE);


      /* Route Step Detector event to INT1 */
      pin_int.step_detector = PROPERTY_ENABLE;
      lsm6dsv320x_pin_int1_route_embedded_set(&dev_ctx,&pin_int);


      printf("Pedometer start...\r\n");

    while (1) {

        if (thread_wake)
        {
          lsm6dsv320x_all_sources_t status = {0};

          thread_wake = 0;

          /* Read interrupt source */
          lsm6dsv320x_all_sources_get(&dev_ctx,
                                      &status);

          /* Check Step Detector event */
          if (status.step_detector)
          {
            /* Read Step Counter */
            lsm6dsv320x_stpcnt_steps_get(&dev_ctx,
                                         &step_count);

            printf("Step detected, Steps = %d\r\n",
                   step_count);
          }
        }
    }
  }
} /* end main */





/*
 * @brief  Write generic device register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to write
 * @param  bufp      pointer to data to write in register reg
 * @param  len       number of consecutive register to write
 *
 */
static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp,
                              uint16_t len)
{
    hal_i2c_handle_t *hi2c = (hal_i2c_handle_t *)handle;

    if (HAL_I2C_MASTER_MemWrite(hi2c,
    		LSM6DSV320X_I2C_ADD_L,
                                reg,
                                HAL_I2C_MEM_ADDR_8BIT,
                                bufp,
                                len,
                                1000) != HAL_OK)
    {
        return -1;
    }

    return 0;
}

/*
 * @brief  Read generic device register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to read
 * @param  bufp      pointer to buffer that store the data read
 * @param  len       number of consecutive register to read
 *
 */
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len)
{

    hal_i2c_handle_t *hi2c = (hal_i2c_handle_t *)handle;

    if (HAL_I2C_MASTER_MemRead(hi2c,
    		LSM6DSV320X_I2C_ADD_L,
                               reg,
                               HAL_I2C_MEM_ADDR_8BIT,
                               bufp,
                               len,
                               1000) != HAL_OK)
    {
        return -1;
    }

    return 0;
}



/*
 * @brief  platform specific delay (platform dependent)
 *
 * @param  ms        delay in ms
 *
 */
static void platform_delay(uint32_t ms)
{

  HAL_Delay(ms);
}



