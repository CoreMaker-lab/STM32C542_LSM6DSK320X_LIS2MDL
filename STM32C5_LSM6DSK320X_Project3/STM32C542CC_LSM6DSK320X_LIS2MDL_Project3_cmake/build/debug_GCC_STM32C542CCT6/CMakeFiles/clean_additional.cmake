# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "debug_GCC_STM32C542CCT6")
  file(REMOVE_RECURSE
  "STM32C542CC_LSM6DSK320X_LIS2MDL_Project3.elf"
  "STM32C542CC_LSM6DSK320X_LIS2MDL_Project3.map"
  )
endif()
