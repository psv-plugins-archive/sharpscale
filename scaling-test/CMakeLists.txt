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

cmake_minimum_required(VERSION 3.0)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{DOLCESDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{DOLCESDK}/share/dolce.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define DOLCESDK to point to your SDK path!")
  endif()
endif()

project(sharpscale_scaling_test LANGUAGES C)
include("$ENV{DOLCESDK}/share/dolce.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -O3 -std=c99")

set(ELF eboot.elf)
set(SELF eboot.bin)
set(VPK "${PROJECT_NAME}.vpk")

set(TITLE_NAME "Sharpscale Scaling Test")
set(TITLE_ID "AKRK00004")
set(TITLE_VER "01.03")

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
