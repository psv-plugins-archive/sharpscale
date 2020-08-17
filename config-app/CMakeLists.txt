# This file is part of Sharpscale
# Copyright © 2020 浅倉麗子

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3 of the License.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

if(FILELOG_ENABLE)
  set(LOG_PREFIX "[SharpscaleConfiguration] ")
else()
  set(LOG_PREFIX "\\033[0;33m[SharpscaleConfiguration]\\033[0m ")
endif()

# Build self

set(ELF "config-app.elf")
set(SELF "config-app.self")
set(BOOTPARAM "${CMAKE_CURRENT_BINARY_DIR}/boot_param.bin")

add_executable("${ELF}"
  main.c
)

target_link_libraries("${ELF}"
  vita2d_sys
  psp2dbg

  m
  gcc

  Sharpscale_stub
  SceAppMgr_stub
  SceCtrl_stub
  SceDisplay_stub
  SceGxm_stub
  SceKernelDmacMgr_stub
  SceKernelThreadMgr_stub
  SceLibKernel_stub
  ScePgf_stub
  SceProcessmgr_stub
  SceSysmem_stub
  SceSysmodule_stub
)

target_compile_definitions("${ELF}" PRIVATE
  SCE_DBG_LOG_PREFIX="${LOG_PREFIX}"
)

add_custom_command(OUTPUT "${BOOTPARAM}"
  COMMAND dolce-make-bootparam app_memsize 0x2000 "${BOOTPARAM}"
)

dolce_create_self("${SELF}"
  "${ELF}"
  UNSAFE
  BOOT_PARAM "${BOOTPARAM}"
)

# Build VPK

set(VPK "sharpscale-config.vpk")
set(TITLE_NAME "Sharpscale Configuration Menu")
set(TITLE_ID "AKRK00005")
set(TITLE_VER "01.13")
set(DOLCE_MKSFOEX_FLAGS "${DOLCE_MKSFOEX_FLAGS} -s STITLE=\"Sharpscale Config Menu\"")

dolce_create_vpk("${VPK}" "${TITLE_ID}" "${SELF}"
  NAME "${TITLE_NAME}"
  VERSION "${TITLE_VER}"
  FILE icon0.png sce_sys/icon0.png
  FILE manual.png sce_sys/manual/001.png
)
