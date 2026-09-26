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
/* Free-fall example for the current lsm6dsk320x project.
 * Board ID 0x75 and platform wiring are retained from the original project.
 * INT1 requires pull-up and falling-edge EXTI on the existing GPIO line 0.
 * Threshold: 312 mg. Duration: 12 samples at 480 Hz (25 ms).
 */
#include "main.h"
#include "mx_usart1.h"
#include "lsm6dsk320x_reg.h"
#include <stdio.h>

#define BOOT_TIME 10U
#define BOARD_EXPECTED_ID 0x75U
#define FREE_FALL_DURATION 12U

static stmdev_ctx_t dev_ctx;
static volatile uint8_t thread_wake = 0;
static int32_t platform_write(void *handle, uint8_t reg,
                              const uint8_t *bufp, uint16_t len);
static int32_t platform_read(void *handle, uint8_t reg,
                             uint8_t *bufp, uint16_t len);
static void platform_delay(uint32_t ms);

int _write(int file, char *ptr, int len)
{
    hal_uart_handle_t *huart1 = mx_usart1_uart_gethandle();
    (void)file;
    if (huart1 != NULL)
        HAL_UART_Transmit(huart1, ptr, len, 1000);
    return len;
}

void HAL_EXTI_TriggerCallback(hal_exti_handle_t *hexti,
                              hal_exti_trigger_t trigger)
{
    (void)trigger;
    if (HAL_EXTI_GetInstance(hexti) == HAL_EXTI_GPIO_0)
        thread_wake = 1;
}


int main(void)
{
    if (mx_system_init() != SYSTEM_OK)
    {
        return -1;
    }

    printf("HELLO\r\n");

    HAL_GPIO_WritePin(CS1_PORT, CS1_PIN, HAL_GPIO_PIN_SET);
    HAL_GPIO_WritePin(SA0_PORT, SA0_PIN, HAL_GPIO_PIN_RESET);
    HAL_GPIO_WritePin(CS2_PORT, CS2_PIN, HAL_GPIO_PIN_SET);

    uint8_t whoamI = 0;
    lsm6dsk320x_if_cfg_t if_cfg = {0};
    lsm6dsk320x_interrupt_mode_t irq = {0};
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

    /* Low-G accelerometer: 480 Hz, +/-2 g. */
    lsm6dsk320x_xl_setup(&dev_ctx,
                       LSM6DSK320X_ODR_AT_480Hz,
                       LSM6DSK320X_XL_HIGH_PERFORMANCE_MD);

    lsm6dsk320x_xl_full_scale_set(&dev_ctx, LSM6DSK320X_2g);

    /* INT1: open-drain + active low. */
    lsm6dsk320x_read_reg(&dev_ctx, LSM6DSK320X_IF_CFG,
                        (uint8_t *)&if_cfg, 1);

    if_cfg.pp_od = 1;
    if_cfg.h_lactive = 1;

    lsm6dsk320x_write_reg(&dev_ctx, LSM6DSK320X_IF_CFG,
                         (uint8_t *)&if_cfg, 1);

    /* Enable basic interrupts in latched mode. */
    irq.enable = PROPERTY_ENABLE;
    irq.lir = 1;
    lsm6dsk320x_interrupt_enable_set(&dev_ctx, irq);

    /* Avoid events during accelerometer filter settling. */
    lsm6dsk320x_filt_xl_fast_settling_set(&dev_ctx, PROPERTY_ENABLE);
    lsm6dsk320x_mask_trigger_xl_settl_set(&dev_ctx, PROPERTY_ENABLE);

    /* Free-fall threshold = 312 mg. */
    lsm6dsk320x_ff_thresholds_set(&dev_ctx, LSM6DSK320X_312_mg);

    /* Duration = 12 / 480 Hz = 25 ms. */
    lsm6dsk320x_ff_time_windows_set(&dev_ctx, FREE_FALL_DURATION);

    /* Route Free-fall event to INT1. */
    pin_int.freefall = PROPERTY_ENABLE;
    lsm6dsk320x_pin_int1_route_set(&dev_ctx, &pin_int);

    thread_wake = 1;
    printf("Free-fall detection start...\r\n");

    while (1)
    {
        if (thread_wake)
        {
            lsm6dsk320x_wake_up_src_t status = {0};

            thread_wake = 0;

            /* Read Free-fall status and acknowledge the latched event. */
            if (lsm6dsk320x_read_reg(&dev_ctx,
                    LSM6DSK320X_WAKE_UP_SRC,
                    (uint8_t *)&status, 1) != 0)
            {
                thread_wake = 1;
                platform_delay(100);
                continue;
            }

            if (status.ff_ia)
            {
                printf("Free-fall detected\r\n");
            }
        }
    }
}

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



