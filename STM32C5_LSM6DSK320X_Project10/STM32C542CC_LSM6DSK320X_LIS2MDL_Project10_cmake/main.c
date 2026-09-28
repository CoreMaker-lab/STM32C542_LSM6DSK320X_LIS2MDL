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

#include "lsm6dsk320x_reg.h"

int _write(int file, char *ptr, int len)
{
    hal_uart_handle_t *huart1 = mx_usart1_uart_gethandle();

    if (huart1 != NULL)
    {
        HAL_UART_Transmit(huart1, ptr, len, 1000);
    }

    return len;
}


#define BOOT_TIME 10U
#define BOARD_EXPECTED_ID 0x75U
#define FIFO_WATERMARK 128U

static stmdev_ctx_t dev_ctx;
static volatile uint8_t thread_wake = 0;
static int32_t platform_write(void *handle, uint8_t reg,
                              const uint8_t *bufp, uint16_t len);
static int32_t platform_read(void *handle, uint8_t reg,
                             uint8_t *bufp, uint16_t len);
static void platform_delay(uint32_t ms);

/**
 * @brief  EXTI trigger callback
 */
void HAL_EXTI_TriggerCallback(hal_exti_handle_t *hexti,
                              hal_exti_trigger_t trigger)
{
    (void)trigger;
    if (HAL_EXTI_GetInstance(hexti) == HAL_EXTI_GPIO_0)
        thread_wake = 1;
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

	     printf("HELLO\r\n");

	     HAL_GPIO_WritePin(CS1_PORT, CS1_PIN, HAL_GPIO_PIN_SET);
	     HAL_GPIO_WritePin(SA0_PORT, SA0_PIN, HAL_GPIO_PIN_RESET);
	     HAL_GPIO_WritePin(CS2_PORT, CS2_PIN, HAL_GPIO_PIN_SET);

	     uint8_t whoamI = 0;
	     lsm6dsk320x_if_cfg_t if_cfg = {0};
	     lsm6dsk320x_filt_settling_mask_t filt_settling_mask = {0};
	     lsm6dsk320x_pin_int_route_t pin_int = {0};

	     /* Initialize driver interface. */
	     dev_ctx.write_reg = platform_write;
	     dev_ctx.read_reg = platform_read;
	     dev_ctx.mdelay = platform_delay;
	     dev_ctx.handle = mx_i2c1_i2c_gethandle();

	     platform_delay(BOOT_TIME);

	     /* Check device ID. */
	     lsm6dsk320x_device_id_get(&dev_ctx, &whoamI);

	     printf("LSM6DSK320X_ID=0x%x,id=0x%x\r\n",
	            (unsigned int)BOARD_EXPECTED_ID, (unsigned int)whoamI);

	     if (whoamI != BOARD_EXPECTED_ID)
	         while (1);

	     /* Reset device. */
	     lsm6dsk320x_sw_por(&dev_ctx);
	     platform_delay(BOOT_TIME);

	 /* Enable Block Data Update. */
	 lsm6dsk320x_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);

	 /* INT1: open-drain + active low. */
	 lsm6dsk320x_read_reg(&dev_ctx, LSM6DSK320X_IF_CFG,
	                     (uint8_t *)&if_cfg, 1);
	 if_cfg.pp_od = 1;
	 if_cfg.h_lactive = 1;
	 lsm6dsk320x_write_reg(&dev_ctx, LSM6DSK320X_IF_CFG,
	                      (uint8_t *)&if_cfg, 1);

	 /* Set full scales before starting the sensors. */
	 lsm6dsk320x_xl_full_scale_set(&dev_ctx, LSM6DSK320X_2g);
	 lsm6dsk320x_gy_full_scale_set(&dev_ctx, LSM6DSK320X_2000dps);

	 /* Mask data-ready during filter settling. */
	 filt_settling_mask.drdy = PROPERTY_ENABLE;
	 filt_settling_mask.irq_xl = PROPERTY_ENABLE;
	 filt_settling_mask.irq_g = PROPERTY_ENABLE;
	 lsm6dsk320x_filt_settling_mask_set(&dev_ctx, filt_settling_mask);

	 /* Configure accelerometer and gyroscope filters. */
	 lsm6dsk320x_filt_gy_lp1_set(&dev_ctx, PROPERTY_ENABLE);
	 lsm6dsk320x_filt_gy_lp1_bandwidth_set(&dev_ctx,
	                                    LSM6DSK320X_GY_ULTRA_LIGHT);
	 lsm6dsk320x_filt_xl_lp2_set(&dev_ctx, PROPERTY_ENABLE);
	 lsm6dsk320x_filt_xl_lp2_bandwidth_set(&dev_ctx,
	                                    LSM6DSK320X_XL_STRONG);

	 /* Bypass clears FIFO. Watermark counts TAG + 6-byte records,
	  * not pairs of accelerometer and gyroscope samples. */
	 lsm6dsk320x_fifo_mode_set(&dev_ctx, LSM6DSK320X_BYPASS_MODE);
	 lsm6dsk320x_fifo_watermark_set(&dev_ctx, FIFO_WATERMARK);

	 /* Batch Low-G accelerometer and gyroscope at 60 Hz each. */
	 lsm6dsk320x_fifo_xl_batch_set(&dev_ctx,
	                             LSM6DSK320X_XL_BATCHED_AT_60Hz);
	 lsm6dsk320x_fifo_gy_batch_set(&dev_ctx,
	                             LSM6DSK320X_GY_BATCHED_AT_60Hz);

	 /* Route FIFO watermark to INT1. No basic-event latch is needed. */
	 pin_int.fifo_th = PROPERTY_ENABLE;
	 lsm6dsk320x_pin_int1_route_set(&dev_ctx, &pin_int);

	 /* Continuous FIFO: oldest unread data is overwritten when full. */
	 lsm6dsk320x_fifo_mode_set(&dev_ctx, LSM6DSK320X_STREAM_MODE);

	 /* Start Low-G accelerometer and gyroscope at 60 Hz. */
	 lsm6dsk320x_xl_setup(&dev_ctx, LSM6DSK320X_ODR_AT_60Hz,
	                    LSM6DSK320X_XL_HIGH_PERFORMANCE_MD);
	 lsm6dsk320x_gy_setup(&dev_ctx, LSM6DSK320X_ODR_AT_60Hz,
	                    LSM6DSK320X_GY_HIGH_PERFORMANCE_MD);

	 thread_wake = 1;
	 printf("FIFO detection start, watermark=%u records\r\n",
	        (unsigned int)FIFO_WATERMARK);

	 while (1)
	 {
	     if (thread_wake)
	     {
	         lsm6dsk320x_fifo_status_t status = {0};
	         float xl_sum[3] = {0};
	         float gy_sum[3] = {0};
	         uint16_t xl_count = 0;
	         uint16_t gy_count = 0;
	         uint16_t num;

	         thread_wake = 0;

	         /* Reading status alone does not clear a watermark condition. */
	         if (lsm6dsk320x_fifo_status_get(&dev_ctx, &status) != 0)
	         {
	             thread_wake = 1;
	             platform_delay(100);
	             continue;
	         }

	         if (status.fifo_ovr)
	             printf("FIFO overrun: samples lost\r\n");

	         num = status.fifo_level;

	         /* Read the records already present in this FIFO snapshot. */
	         while (num > 0)
	         {
	             lsm6dsk320x_fifo_out_raw_t fifo_data = {0};
	             int16_t raw[3];

	             if (lsm6dsk320x_fifo_out_raw_get(&dev_ctx, &fifo_data) != 0)
	             {
	                 thread_wake = 1;
	                 platform_delay(100);
	                 break;
	             }
	             num--;

	             /* Decode little-endian axes without unaligned pointers. */
	             raw[0] = (int16_t)((uint16_t)fifo_data.data[0] |
	                               ((uint16_t)fifo_data.data[1] << 8));
	             raw[1] = (int16_t)((uint16_t)fifo_data.data[2] |
	                               ((uint16_t)fifo_data.data[3] << 8));
	             raw[2] = (int16_t)((uint16_t)fifo_data.data[4] |
	                               ((uint16_t)fifo_data.data[5] << 8));

	             switch (fifo_data.tag)
	             {
	                 case LSM6DSK320X_XL_NC_TAG:
	                     xl_sum[0] += lsm6dsk320x_from_fs2_to_mg(raw[0]);
	                     xl_sum[1] += lsm6dsk320x_from_fs2_to_mg(raw[1]);
	                     xl_sum[2] += lsm6dsk320x_from_fs2_to_mg(raw[2]);
	                     xl_count++;
	                     break;

	                 case LSM6DSK320X_GY_NC_TAG:
	                     gy_sum[0] += lsm6dsk320x_from_fs2000_to_mdps(raw[0]);
	                     gy_sum[1] += lsm6dsk320x_from_fs2000_to_mdps(raw[1]);
	                     gy_sum[2] += lsm6dsk320x_from_fs2000_to_mdps(raw[2]);
	                     gy_count++;
	                     break;

	                 default:
	                     break;
	             }
	         }

	         /* Print only after reading the batch to limit UART overhead.
	          * These are batch averages, not individual FIFO samples. */
	         if (xl_count > 0)
	         {
	             printf("XL avg (%u) [mg]: %.2f, %.2f, %.2f\r\n",(unsigned int)xl_count,(double)(xl_sum[0] / xl_count),(double)(xl_sum[1] / xl_count),(double)(xl_sum[2] / xl_count));
	         }

	         if (gy_count > 0)
	         {
	             printf("GY avg (%u) [mdps]: %.2f, %.2f, %.2f\r\n\r\n",(unsigned int)gy_count,(double)(gy_sum[0] / gy_count),(double)(gy_sum[1] / gy_count),(double)(gy_sum[2] / gy_count));
	         }

	         /* New samples arrived during reads/printing. If still above
	          * watermark, keep servicing: INT1 may remain active low and
	          * no additional falling edge would be generated. */
	         if (lsm6dsk320x_fifo_status_get(&dev_ctx, &status) != 0)
	         {
	             thread_wake = 1;
	             platform_delay(100);
	         }
	         else if (status.fifo_th)
	         {
	             thread_wake = 1;
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
    		LSM6DSK320X_I2C_ADD_L,
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
    		LSM6DSK320X_I2C_ADD_L,
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



