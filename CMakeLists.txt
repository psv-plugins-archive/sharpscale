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

if(DEFINED ENV{DOLCESDK})
  set(CMAKE_TOOLCHAIN_FILE "$ENV{DOLCESDK}/share/dolce.toolchain.cmake")
  include("$ENV{DOLCESDK}/share/dolce.cmake" REQUIRED)
else()
  message(FATAL_ERROR "Please define DOLCESDK to point to your SDK path!")
endif()

project(sharpscale LANGUAGES C)

add_compile_options(
  -O3 -std=c99
  -Wall -Wextra -Wdouble-promotion -Wshadow -Wundef
  -fsingle-precision-constant -fno-common
)

add_link_options(
  -nostdlib
)

if(DEFINED LOG_ENABLE)
  add_compile_definitions(SCE_DBG_LOGGING_ENABLED=${LOG_ENABLE})
else()
  add_compile_definitions(SCE_DBG_LOGGING_ENABLED=0)
endif()

if(DEFINED LOG_LEVEL)
  add_compile_definitions(SCE_DBG_MINIMUM_LOG_LEVEL=${LOG_LEVEL})
else()
  add_compile_definitions(SCE_DBG_MINIMUM_LOG_LEVEL=SCE_DBG_LOG_LEVEL_INFO)
endif()

# Build self

set(ELF "sharpscale.elf")
set(SELF "sharpscale.skprx")
set(VELF_CFG "exports.yml")
set(SELF_STUB "sharpscale-stub")

add_executable("${ELF}"
  main.c
  config.c
)

target_link_libraries("${ELF}"
  m

  psp2dbgForKernel
  taihenForKernel_stub
  taihenModuleUtils_stub

  SceDebugForDriver_stub
  SceIofilemgrForDriver_stub
  SceSblACMgrForDriver_stub
  SceSysclibForDriver_stub
  SceSysmemForDriver_stub
)

target_compile_definitions("${ELF}" PRIVATE
  SCE_DBG_LOG_PREFIX="\\033[0;35m[Sharpscale]\\033[0m "
)

dolce_create_self("${SELF}"
  "${ELF}"
  CONFIG "${VELF_CFG}"
)

dolce_create_stubs("${SELF_STUB}" "${SELF}" "${VELF_CFG}"
  KERNEL
  LIB Sharpscale_stub
)

add_subdirectory(config-app)
add_subdirectory(scaling-test)
