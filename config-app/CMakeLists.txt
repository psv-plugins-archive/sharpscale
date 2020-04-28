project(sharpscale_config LANGUAGES C)

add_custom_command(OUTPUT boot_param.bin
  COMMAND dolce-make-bootparam app_memsize 0x1800 boot_param.bin
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
  NAME "Sharpscale Config"
  VERSION 01.01
  FILE icon0.png sce_sys/icon0.png
)
