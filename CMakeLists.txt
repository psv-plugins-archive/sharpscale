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

cmake_minimum_required(VERSION 3.13)

set(CMAKE_TOOLCHAIN_FILE "$ENV{DOLCESDK}/share/dolce.toolchain.cmake" CACHE PATH "DolceSDK toolchain file")
set(DOLCESDK_CMAKE_FILE "$ENV{DOLCESDK}/share/dolce.cmake" CACHE PATH "DolceSDK CMake functions file")
include("${DOLCESDK_CMAKE_FILE}" REQUIRED)

project(sharpscale LANGUAGES C)

add_compile_options(
  -Os -std=c99
  -Wall -Wextra -Wdouble-promotion -Wshadow -Wundef
  -fsingle-precision-constant -fno-common
  -ftree-vectorize -funsafe-math-optimizations -flto
)

add_link_options(
  -nostdlib
)

include_directories(
  "${CMAKE_SOURCE_DIR}/include"
)

set(LOGGING "1" CACHE STRING "Enable logging")
set(FILE_LOGGING "0" CACHE STRING "Enable file logging")
set(LOG_LEVEL "INFO" CACHE STRING "Log verbosity level")

add_compile_definitions(
  SCE_DBG_LOGGING_ENABLED=${LOGGING}
  SCE_DBG_FILE_LOGGING_ENABLED=${FILE_LOGGING}
  SCE_DBG_MINIMUM_LOG_LEVEL=SCE_DBG_LOG_LEVEL_${LOG_LEVEL}
)

if(FILE_LOGGING)
  set(LOG_PREFIX "[Sharpscale] ")
else()
  set(LOG_PREFIX "\\033[0;35m[Sharpscale]\\033[0m ")
endif()

# Build self

set(ELF "sharpscale.elf")
set(SELF "sharpscale.skprx")
set(MODULE_INFO "sharpscale.yml")
set(MODULE_STUB "sharpscale-stub")

add_executable("${ELF}"
  sharpscale.c
  config.c
)

target_link_libraries("${ELF}"
  psp2dbgForKernel

  taihenForKernel_stub
  taihenModuleUtils_stub

  SceDebugForDriver_stub
  SceIofilemgrForDriver_stub
  SceSblACMgrForDriver_stub
  SceSysclibForDriver_stub
  SceSysmemForDriver_stub
  SceThreadmgrForDriver_stub

  m
)

target_compile_definitions("${ELF}" PRIVATE
  SCE_DBG_LOG_PREFIX="${LOG_PREFIX}"
)

target_link_options("${ELF}" PRIVATE
  --entry=module_start
)

dolce_create_self("${SELF}"
  "${ELF}"
  CONFIG "${MODULE_INFO}"
)

dolce_create_stubs("${MODULE_STUB}" "${SELF}" "${MODULE_INFO}"
  KERNEL
  LIB Sharpscale_stub
)

add_subdirectory(config-app)
add_subdirectory(scaling-test)
