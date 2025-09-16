# Install script for directory: C:/ncs/v2.9.1/zephyr

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/Zephyr-Kernel")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "MinSizeRel")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "C:/ncs/toolchains/b620d30767/opt/zephyr-sdk/arm-zephyr-eabi/bin/arm-zephyr-eabi-objdump.exe")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/zephyr/arch/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/zephyr/lib/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/zephyr/soc/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/zephyr/boards/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/zephyr/subsys/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/zephyr/drivers/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/modules/nrf/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/modules/mcuboot/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/modules/mbedtls/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/modules/trusted-firmware-m/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/modules/cjson/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/modules/azure-sdk-for-c/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/modules/cirrus-logic/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/modules/openthread/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/modules/suit-processor/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/modules/memfault-firmware-sdk/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/modules/canopennode/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/modules/chre/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/modules/lz4/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/modules/nanopb/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/modules/zscilib/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/modules/cmsis/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/modules/cmsis-dsp/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/modules/cmsis-nn/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/modules/fatfs/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/modules/hal_nordic/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/modules/hal_st/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/modules/hal_wurthelektronik/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/modules/hostap/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/modules/libmetal/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/modules/liblc3/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/modules/littlefs/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/modules/loramac-node/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/modules/lvgl/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/modules/mipi-sys-t/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/modules/nrf_wifi/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/modules/open-amp/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/modules/picolibc/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/modules/segger/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/modules/tinycrypt/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/modules/uoscore-uedhoc/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/modules/zcbor/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/modules/nrfxlib/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/modules/nrf_hw_models/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/modules/connectedhomeip/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/zephyr/kernel/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/zephyr/cmake/flash/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/zephyr/cmake/usage/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Serial1_central_blejoybus/buildNaN/Serial1_central_blejoybus/zephyr/cmake/reports/cmake_install.cmake")
endif()

