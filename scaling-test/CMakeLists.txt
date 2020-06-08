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

include("$ENV{DOLCESDK}/share/dolce.cmake" REQUIRED)

project(sharpscale_scaling_test LANGUAGES C)

# build font
set(FONT_FILE "unifont.bdf.gz")
set(SFN_FILE "font.sfn")

file(DOWNLOAD
  "https://ftp.gnu.org/gnu/unifont/unifont-13.0.02/unifont-13.0.02.bdf.gz"
  "${FONT_FILE}"
  EXPECTED_HASH SHA256=ee02036aec9350dc35ebaaa205c43f9f9ad14efc56d63c12b177ac4fd424e94f
)

add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${SFN_FILE}"
  COMMAND bit2sfn
    -r "Basic Latin"        "${FONT_FILE}"
    -r "Latin-1 Supplement" "${FONT_FILE}"
    -r 0x6D45 0x6D46        "${FONT_FILE}"
    -r 0x5009 0x500A        "${FONT_FILE}"
    -r 0x9E97 0x9E98        "${FONT_FILE}"
    -r 0x5B50 0x5B51        "${FONT_FILE}"
    "${CMAKE_CURRENT_BINARY_DIR}/${SFN_FILE}"
)

# build application
set(ELF "${PROJECT_NAME}.elf")
set(SELF "${PROJECT_NAME}.self")
set(VPK "${PROJECT_NAME}.vpk")

set(TITLE_NAME "Sharpscale Scaling Test")
set(TITLE_ID "AKRK00004")
set(TITLE_VER "01.05")

add_executable("${ELF}"
  main.c
)

target_compile_options("${ELF}"
  PRIVATE -O3 -std=c99
  PRIVATE -Wall -Wextra -Wdouble-promotion -Wshadow -Wundef
  PRIVATE -fsingle-precision-constant -fno-common
)

target_link_libraries("${ELF}"
  fnblit
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
  FILE icon0.png sce_sys/icon0.png
  FILE "${CMAKE_CURRENT_BINARY_DIR}/${SFN_FILE}" "${SFN_FILE}"
)
