#pragma once

// make sure this config is the one used
#define NRFX_CONFIG_H__

#define NRFX_POWER_ENABLED                     1
#define NRFX_CLOCK_DEFAULT_CONFIG_IRQ_PRIORITY 6
#define NRFX_POWER_DEFAULT_CONFIG_IRQ_PRIORITY 6
#define NRFX_CLOCK_ENABLED                     1
#define NRFX_UART0_ENABLED                     1
#define NRFX_PRS_ENABLED                       1
#define NRFX_CLOCK_CONFIG_LF_SRC               1
#define NRFX_NVMC_ENABLED                      1
#define IRQ_PRIORITY_USB                       2
#define IRQ_PRIORITY_UART                      3
#define IRQ_PRIORITY_I2C                       3
#define IRQ_PRIORITY_ADC                       5
#define NRFX_TWIM_ENABLED                      1
#define NRFX_TWIM0_ENABLED                     1