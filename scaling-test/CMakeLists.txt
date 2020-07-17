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

# Build font

set(FONT_BASE "unifont-13.0.02")
set(FONT_FILE "${CMAKE_CURRENT_BINARY_DIR}/unifont.bdf.gz")
set(SFN_FILE "${CMAKE_CURRENT_BINARY_DIR}/unifont.sfn")

file(DOWNLOAD
  "https://ftp.gnu.org/gnu/unifont/${FONT_BASE}/${FONT_BASE}.bdf.gz"
  "${FONT_FILE}"
  EXPECTED_HASH SHA256=ee02036aec9350dc35ebaaa205c43f9f9ad14efc56d63c12b177ac4fd424e94f
)

add_custom_command(OUTPUT "${SFN_FILE}"
  DEPENDS "${FONT_FILE}"
  COMMAND bit2sfn
    -r "Basic Latin"        "${FONT_FILE}"
    -r "Latin-1 Supplement" "${FONT_FILE}"
    -r 0x6D45 0x6D46        "${FONT_FILE}"
    -r 0x5009 0x500A        "${FONT_FILE}"
    -r 0x9E97 0x9E98        "${FONT_FILE}"
    -r 0x5B50 0x5B51        "${FONT_FILE}"
    "${SFN_FILE}"
)

# Build self

set(ELF "scaling-test.elf")
set(SELF "scaling-test.self")

add_executable("${ELF}"
  main.c
)

target_link_libraries("${ELF}"
  fnblit
  psp2dbg

  SceCtrl_stub
  SceDisplay_stub
  SceIofilemgr_stub
  SceLibKernel_stub
  SceSysmem_stub
)

target_compile_definitions("${ELF}" PRIVATE
  SCE_DBG_LOG_PREFIX="\\033[0;36m[SharpscaleScalingTest]\\033[0m "
)

dolce_create_self("${SELF}"
  "${ELF}"
)

# Build VPK

set(VPK "sharpscale-scaling-test.vpk")
set(TITLE_NAME "Sharpscale Scaling Test")
set(TITLE_ID "AKRK00004")
set(TITLE_VER "01.07")

dolce_create_vpk("${VPK}" "${TITLE_ID}" "${SELF}"
  NAME "${TITLE_NAME}"
  VERSION "${TITLE_VER}"
  FILE icon0.png sce_sys/icon0.png
  FILE "${SFN_FILE}" font.sfn
)
