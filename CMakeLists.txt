cmake_minimum_required(VERSION 3.19)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path.")
  endif()
endif()

project(vitafetch C)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(VITA_APP_NAME "vitafetch")
set(VITA_TITLEID  "VTFT00001")
set(VITA_VERSION  "01.00")

add_executable(vitafetch
    src/main.c
    src/sysinfo.c
    src/ascii.c
    src/render.c
)

target_include_directories(vitafetch PRIVATE src)

target_link_libraries(vitafetch
    vita2d
    ScePgf_stub
    ScePower_stub
    SceCtrl_stub
    SceDisplay_stub
    SceGxm_stub
    SceLibKernel_stub
    SceIofilemgr_stub
    SceKernelModulemgr_stub
    SceProcessmgr_stub
    SceCommonDialog_stub
    SceAppMgr_stub
    SceRegistryMgr_stub
    freetype
    png
    jpeg
    z
    m
    c
)

vita_create_self(vitafetch.self vitafetch)

vita_create_vpk(vitafetch.vpk ${VITA_TITLEID} vitafetch.self
    VERSION  ${VITA_VERSION}
    NAME     ${VITA_APP_NAME}
    FILE sce_sys/icon0.png                          sce_sys/icon0.png
    FILE sce_sys/livearea/contents/bg.png           sce_sys/livearea/contents/bg.png
    FILE sce_sys/livearea/contents/startup.png      sce_sys/livearea/contents/startup.png
    FILE sce_sys/livearea/contents/template.xml     sce_sys/livearea/contents/template.xml
)
