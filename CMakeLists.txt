# This file is part of Sharpscale
# Copyright 2020 浅倉麗子

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3 of the License.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.13)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{DOLCESDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{DOLCESDK}/share/dolce.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define DOLCESDK to point to your SDK path!")
  endif()
endif()

include("$ENV{DOLCESDK}/share/dolce.cmake" REQUIRED)

project(sharpscale LANGUAGES C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -O3 -std=c99 -Wdouble-promotion -fsingle-precision-constant")

if(LOG_PRINTF)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DLOG_PRINTF")
endif()

if(LOG_FILE)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DLOG_FILE")
endif()

set(ELF "${PROJECT_NAME}.elf")
set(SELF "${PROJECT_NAME}.skprx")

add_executable("${ELF}"
  main.c
  config.c
)

target_link_options("${ELF}"
  PRIVATE -nostdlib
)

target_link_libraries("${ELF}"
  gcc
  m
  SceDebugForDriver_stub
  SceIofilemgrForDriver_stub
  SceSblACMgrForDriver_stub
  SceSysclibForDriver_stub
  SceSysmemForDriver_stub
  taihenForKernel_stub
  taihenModuleUtils_stub
)

dolce_create_self("${SELF}"
  "${ELF}"
  CONFIG exports.yml
)

dolce_create_stubs("${PROJECT_NAME}-stubs"
  "${ELF}"
  exports.yml
  KERNEL
  LIB Sharpscale_stub
)

add_subdirectory(config-app)
