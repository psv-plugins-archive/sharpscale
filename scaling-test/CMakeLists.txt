cmake_minimum_required(VERSION 3.0)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{DOLCESDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{DOLCESDK}/share/dolce.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define DOLCESDK to point to your SDK path!")
  endif()
endif()

project(scalingtest LANGUAGES C)
include("$ENV{DOLCESDK}/share/dolce.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -O3 -std=c99")

set(ELF eboot.elf)
set(SELF eboot.bin)
set(VPK "${PROJECT_NAME}.vpk")

set(TITLE_NAME "Scaling test")
set(TITLE_ID "AKRK00004")
set(TITLE_VER "01.01")

add_executable("${ELF}"
  main.c
)

target_link_libraries("${ELF}"
  SceCtrl_stub
  SceDisplay_stub
  SceLibKernel_stub
)

dolce_create_self("${SELF}"
  "${ELF}"
)

dolce_create_vpk("${VPK}" "${TITLE_ID}" "${SELF}"
  VERSION "${TITLE_VER}"
  NAME "${TITLE_NAME}"
)
