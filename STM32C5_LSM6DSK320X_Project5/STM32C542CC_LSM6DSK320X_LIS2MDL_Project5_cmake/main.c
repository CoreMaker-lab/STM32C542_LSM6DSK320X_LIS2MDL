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

/*
 * Wake-up threshold:
 *
 * WU_INACT_THS_W = 0
 * 1 LSB = 7.8125 mg
 *
 * 2 = 15.625 mg
 * 4 = 31.25  mg
 * 8 = 62.5   mg
 */
#define    WAKEUP_THRESHOLD     4


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
static int32_t platform_write(void *handle,
                              uint8_t reg,
                              const uint8_t *bufp,
                              uint16_t len);

static int32_t platform_read(void *handle,
                             uint8_t reg,
                             uint8_t *bufp,
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

      lsm6dsv320x_pin_int_route_t pin_int = {0};
      lsm6dsv320x_if_cfg_t if_cfg = {0};
      lsm6dsv320x_interrupt_mode_t irq = {0};
      lsm6dsv320x_act_thresholds_t wu = {0};


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

      platform_delay(BOOT_TIME);


      /* Enable Block Data Update */
      lsm6dsv320x_block_data_update_set(&dev_ctx,
                                        PROPERTY_ENABLE);



      /*
       * Set Low-G accelerometer.
       *
       * ODR = 480 Hz
       * FS  = +/-2 g
       */
      lsm6dsv320x_xl_setup(&dev_ctx,LSM6DSV320X_ODR_AT_480Hz,LSM6DSV320X_XL_HIGH_PERFORMANCE_MD);
      lsm6dsv320x_xl_full_scale_set(&dev_ctx,LSM6DSV320X_2g);

      /*
       * INT1/INT2:
       * open-drain + active low
       */
      lsm6dsv320x_read_reg(&dev_ctx, LSM6DSV320X_IF_CFG,(uint8_t *)&if_cfg,1);

      if_cfg.pp_od = 1;
      if_cfg.h_lactive = 1;

      lsm6dsv320x_write_reg(&dev_ctx, LSM6DSV320X_IF_CFG,(uint8_t *)&if_cfg,1);


      /*
       * Enable interrupt function.
       *
       * Latched mode is used so that
       * the event is not missed by MCU.
       */
      irq.enable = PROPERTY_ENABLE;
      irq.lir = 1;

      lsm6dsv320x_interrupt_enable_set(&dev_ctx,irq);


      /*
       * Enable XL filter fast settling.
       */
      lsm6dsv320x_filt_xl_fast_settling_set(&dev_ctx,PROPERTY_ENABLE);


      /*
       * Mask interrupt trigger during
       * accelerometer filter settling.
       */
      lsm6dsv320x_mask_trigger_xl_settl_set(&dev_ctx,PROPERTY_ENABLE);


      /*
       * Use High-Pass filter for
       * Wake-up detection.
       */
      lsm6dsv320x_filt_wkup_act_feed_set(&dev_ctx,LSM6DSV320X_WK_FEED_HIGH_PASS);


      /*
       * Wake-up threshold configuration.
       *
       * wu_inact_ths_w = 0:
       * 1 LSB = 7.8125 mg
       *
       * threshold = 4:
       * 4 x 7.8125 mg = 31.25 mg
       */
      wu.inactivity_cfg.inact_dur = 0;

      wu.inactivity_cfg.xl_inact_odr = 1;

      wu.inactivity_cfg.wu_inact_ths_w = 0;

      wu.inactivity_ths = 0;

      wu.threshold = WAKEUP_THRESHOLD;

      wu.duration = 0;

      lsm6dsv320x_act_thresholds_set(&dev_ctx,&wu);


      /*
       * Route Wake-up event to INT1
       */
      pin_int.wakeup = PROPERTY_ENABLE;

      lsm6dsv320x_pin_int1_route_set(&dev_ctx,&pin_int);


      printf("Wake-up Motion Detection start...\r\n");

      printf("Threshold = %d x 7.8125 mg\r\n",WAKEUP_THRESHOLD);


    while (1) {

        if (thread_wake)
        {
          lsm6dsv320x_all_sources_t status = {0};

          thread_wake = 0;


          /* Read interrupt source */
          lsm6dsv320x_all_sources_get(&dev_ctx, &status);


          /* Check Wake-up event */
          if (status.wake_up)
          {
            printf("Motion detected\r\n");
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



