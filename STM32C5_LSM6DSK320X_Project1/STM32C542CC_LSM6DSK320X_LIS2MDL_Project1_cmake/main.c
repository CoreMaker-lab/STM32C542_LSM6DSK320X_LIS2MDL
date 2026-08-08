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
#define    CNT_FOR_OUTPUT       100

/* Private variables ---------------------------------------------------------*/
static int16_t data_raw_motion[3];
static int16_t data_raw_temperature;
static float_t acceleration_mg[3];
static float_t angular_rate_mdps[3];
static float_t temperature_degC;
static uint8_t whoamI;
static uint8_t tx_buffer[1000];

static lsm6dsv320x_filt_settling_mask_t filt_settling_mask;

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
static void tx_com( uint8_t *tx_buffer, uint16_t len );
static void platform_delay(uint32_t ms);
static void platform_init(void *handle);

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

	  stmdev_ctx_t dev_ctx;
	  double_t lowg_xl_sum[3], hg_xl_sum[3], gyro_sum[3], temp_sum;
	  uint16_t lowg_xl_cnt = 0, hg_xl_cnt = 0, gyro_cnt = 0, temp_cnt = 0;

	  /* Initialize mems driver interface */
	  dev_ctx.write_reg = platform_write;
	  dev_ctx.read_reg = platform_read;
	  dev_ctx.mdelay = platform_delay;
	  dev_ctx.handle = mx_i2c1_i2c_gethandle();

	  /* Init test platform */
//	  platform_init(dev_ctx.handle);

	  /* Wait sensor boot time */
	  platform_delay(BOOT_TIME);

	  /* Check device ID */
	  lsm6dsv320x_device_id_get(&dev_ctx, &whoamI);
	  printf("LSM6DSK320X_ID=0x%x,id=0x%x\n",0x75,whoamI);
	  if (whoamI != 0x75)
	    while (1);

	  /* Perform device power-on-reset */
	  lsm6dsv320x_sw_por(&dev_ctx);

	  /* Enable Block Data Update */
	  lsm6dsv320x_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);

	  /* Set Output Data Rate.
	   * Selected data rate have to be equal or greater with respect
	   * with MLC data rate.
	   */
	  lsm6dsv320x_xl_setup(&dev_ctx, LSM6DSV320X_ODR_AT_120Hz, LSM6DSV320X_XL_HIGH_PERFORMANCE_MD);
	  lsm6dsv320x_hg_xl_data_rate_set(&dev_ctx, LSM6DSV320X_HG_XL_ODR_AT_960Hz, 1);
	  lsm6dsv320x_gy_setup(&dev_ctx, LSM6DSV320X_ODR_AT_120Hz, LSM6DSV320X_GY_HIGH_PERFORMANCE_MD);

	  /* Set full scale */
	  lsm6dsv320x_xl_full_scale_set(&dev_ctx, LSM6DSV320X_2g);
	  lsm6dsv320x_hg_xl_full_scale_set(&dev_ctx, LSM6DSV320X_320g);
	  lsm6dsv320x_gy_full_scale_set(&dev_ctx, LSM6DSV320X_2000dps);



    while (1) {
        lsm6dsv320x_data_ready_t drdy;

        /* Read output only if new xl value is available */
        lsm6dsv320x_flag_data_ready_get(&dev_ctx, &drdy);

        if (drdy.drdy_xl) {
          /* Read acceleration field data */
          memset(data_raw_motion, 0x00, 3 * sizeof(int16_t));
          lsm6dsv320x_acceleration_raw_get(&dev_ctx, data_raw_motion);
          acceleration_mg[0] = lsm6dsv320x_from_fs2_to_mg(data_raw_motion[0]);
          acceleration_mg[1] = lsm6dsv320x_from_fs2_to_mg(data_raw_motion[1]);
          acceleration_mg[2] = lsm6dsv320x_from_fs2_to_mg(data_raw_motion[2]);

          lowg_xl_sum[0] += acceleration_mg[0];
          lowg_xl_sum[1] += acceleration_mg[1];
          lowg_xl_sum[2] += acceleration_mg[2];
          lowg_xl_cnt++;
        }

        if (drdy.drdy_hgxl) {
          /* Read acceleration field data */
          memset(data_raw_motion, 0x00, 3 * sizeof(int16_t));
          lsm6dsv320x_hg_acceleration_raw_get(&dev_ctx, data_raw_motion);
          acceleration_mg[0] = lsm6dsv320x_from_fs256_to_mg(data_raw_motion[0]);
          acceleration_mg[1] = lsm6dsv320x_from_fs256_to_mg(data_raw_motion[1]);
          acceleration_mg[2] = lsm6dsv320x_from_fs256_to_mg(data_raw_motion[2]);

          hg_xl_sum[0] += acceleration_mg[0];
          hg_xl_sum[1] += acceleration_mg[1];
          hg_xl_sum[2] += acceleration_mg[2];
          hg_xl_cnt++;
        }

        /* Read output only if new xl value is available */
        if (drdy.drdy_gy) {
          /* Read angular rate field data */
          memset(data_raw_motion, 0x00, 3 * sizeof(int16_t));
          lsm6dsv320x_angular_rate_raw_get(&dev_ctx, data_raw_motion);
          angular_rate_mdps[0] = lsm6dsv320x_from_fs2000_to_mdps(data_raw_motion[0]);
          angular_rate_mdps[1] = lsm6dsv320x_from_fs2000_to_mdps(data_raw_motion[1]);
          angular_rate_mdps[2] = lsm6dsv320x_from_fs2000_to_mdps(data_raw_motion[2]);

          gyro_sum[0] += angular_rate_mdps[0];
          gyro_sum[1] += angular_rate_mdps[1];
          gyro_sum[2] += angular_rate_mdps[2];
          gyro_cnt++;
        }

        if (drdy.drdy_temp) {
          /* Read temperature data */
          memset(&data_raw_temperature, 0x00, sizeof(int16_t));
          lsm6dsv320x_temperature_raw_get(&dev_ctx, &data_raw_temperature);
          temperature_degC = lsm6dsv320x_from_lsb_to_celsius(data_raw_temperature);
          temp_sum += temperature_degC;
          temp_cnt++;
        }

        if (lowg_xl_cnt >= CNT_FOR_OUTPUT) {
          /* print avg low-g xl data */
          acceleration_mg[0] = lowg_xl_sum[0] / lowg_xl_cnt;
          acceleration_mg[1] = lowg_xl_sum[1] / lowg_xl_cnt;
          acceleration_mg[2] = lowg_xl_sum[2] / lowg_xl_cnt;

          printf("lg xl (avg of %d samples) [mg]:%4.2f\t%4.2f\t%4.2f\r\n",
                  lowg_xl_cnt, acceleration_mg[0], acceleration_mg[1], acceleration_mg[2]);
          lowg_xl_sum[0] = lowg_xl_sum[1] = lowg_xl_sum[2] = 0.0;
          lowg_xl_cnt = 0;

          /* print avg high-g xl data */
          acceleration_mg[0] = hg_xl_sum[0] / hg_xl_cnt;
          acceleration_mg[1] = hg_xl_sum[1] / hg_xl_cnt;
          acceleration_mg[2] = hg_xl_sum[2] / hg_xl_cnt;

          printf("hg xl (avg of %d samples) [mg]:%4.2f\t%4.2f\t%4.2f\r\n",
                  hg_xl_cnt, acceleration_mg[0], acceleration_mg[1], acceleration_mg[2]);
          hg_xl_sum[0] = hg_xl_sum[1] = hg_xl_sum[2] = 0.0;
          hg_xl_cnt = 0;

          /* print avg gyro data */
          angular_rate_mdps[0] = gyro_sum[0] / gyro_cnt;
          angular_rate_mdps[1] = gyro_sum[1] / gyro_cnt;
          angular_rate_mdps[2] = gyro_sum[2] / gyro_cnt;

          printf("gyro (avg of %d samples) [mdps]:%4.2f\t%4.2f\t%4.2f\r\n",
                  gyro_cnt, angular_rate_mdps[0], angular_rate_mdps[1], angular_rate_mdps[2]);
          gyro_sum[0] = gyro_sum[1] = gyro_sum[2] = 0.0;
          gyro_cnt = 0;

          /* print avg temperature data */
          temperature_degC = temp_sum / temp_cnt;
          printf("Temperature (avg of %d samples) [degC]:%6.2f\r\n\r\n",
                  temp_cnt, temperature_degC);
          temp_cnt = 0;
          temp_sum = 0.0;
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


