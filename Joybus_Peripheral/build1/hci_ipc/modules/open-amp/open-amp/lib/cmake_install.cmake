# Install script for directory: C:/ncs/v2.9.1/modules/lib/open-amp/open-amp/lib

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

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE DIRECTORY FILES "C:/ncs/v2.9.1/modules/lib/open-amp/open-amp/lib/include/openamp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE DIRECTORY FILES "C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Joybus_Peripheral/build1/hci_ipc/modules/open-amp/open-amp/include/generated/openamp")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Joybus_Peripheral/build1/hci_ipc/modules/open-amp/open-amp/lib/virtio/cmake_install.cmake")
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Joybus_Peripheral/build1/hci_ipc/modules/open-amp/open-amp/lib/rpmsg/cmake_install.cmake")
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Joybus_Peripheral/build1/hci_ipc/modules/open-amp/open-amp/lib/remoteproc/cmake_install.cmake")
  include("C:/Users/ralls/GitHub/Bluetooth-N64-Controller/Joybus_Peripheral/build1/hci_ipc/modules/open-amp/open-amp/lib/utils/cmake_install.cmake")

endif()

