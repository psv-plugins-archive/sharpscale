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

project(sharpscale_config LANGUAGES C)

add_custom_command(OUTPUT boot_param.bin
  COMMAND dolce-make-bootparam app_memsize 0x2000 boot_param.bin
)

set(ELF "${PROJECT_NAME}.elf")
set(SELF "${PROJECT_NAME}.self")

add_executable("${ELF}"
  main.c
)

target_include_directories("${ELF}"
  PRIVATE "${CMAKE_SOURCE_DIR}"
)

target_link_libraries("${ELF}"
  vita2d_sys
  m
  Sharpscale_stub
  SceAppMgr_stub
  SceCtrl_stub
  SceDisplay_stub
  SceGxm_stub
  SceLibKernel_stub
  ScePgf_stub
  SceSysmodule_stub
)

dolce_create_self("${SELF}"
  "${ELF}"
  UNSAFE
  BOOT_PARAM ${CMAKE_CURRENT_BINARY_DIR}/boot_param.bin
)

dolce_create_vpk(${PROJECT_NAME}.vpk
  AKRK00005
  "${SELF}"
  NAME "Sharpscale Config Menu"
  VERSION 01.04
  FILE icon0.png sce_sys/icon0.png
)
