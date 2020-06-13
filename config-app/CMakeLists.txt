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
set(VPK "${PROJECT_NAME}.vpk")

set(TITLE_NAME "Sharpscale Config Menu")
set(TITLE_ID "AKRK00005")
set(TITLE_VER "01.07")

add_executable("${ELF}"
  main.c
)

target_compile_options("${ELF}"
  PRIVATE -O3 -std=c99
  PRIVATE -Wall -Wextra -Wdouble-promotion -Wshadow -Wundef
  PRIVATE -fsingle-precision-constant -fno-common
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

dolce_create_vpk("${VPK}" "${TITLE_ID}" "${SELF}"
  NAME "${TITLE_NAME}"
  VERSION "${TITLE_VER}"
  FILE icon0.png sce_sys/icon0.png
  FILE manual.png sce_sys/manual/001.png
)
