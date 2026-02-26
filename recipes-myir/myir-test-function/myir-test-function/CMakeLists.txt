cmake_minimum_required(VERSION 3.10)
project(153_dual_camera)

set(CMAKE_C_STANDARD 99)#C99 # lvgl officially support C99 and above
set(CMAKE_CXX_STANDARD 17)#C17
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_BUILD_TYPE Debug)

option(CONFIG_USE_TINA_G2D "Use the G2D of ALLWINNER" off)
option(CONFIG_USE_ALLWINNER_ISP "Use the ISP of ALLWINNER" off)
option(CONFIG_USE_DEBUG "Use Printf" off)

message("CONFIG_USE_TINA_G2D : ${CONFIG_USE_TINA_G2D}")
message("CONFIG_USE_ALLWINNER_ISP : ${CONFIG_USE_ALLWINNER_ISP}")
message("CONFIG_USE_DEBUG : ${CONFIG_USE_DEBUG}")

file(GLOB_RECURSE CAMERA_SOURCES "${PROJECT_SOURCE_DIR}/camera/*.c")
file(GLOB_RECURSE G2D_SOURCES "${PROJECT_SOURCE_DIR}/G2D/*.c")

add_executable(myir_camera_test main.c ${CAMERA_SOURCES} ${G2D_SOURCES})

if (CONFIG_USE_TINA_G2D)
    set(TINA_G2D_INC ${PROJECT_SOURCE_DIR}/G2D)
    set(TINA_G2D_LIB_DIR ${TINA_G2D_INC}/libs)
    FIle(GLOB TINA_G2D_LIB "${TINA_G2D_LIB_DIR}/*.so*")
    message("${TINA_G2D_LIB}")
    target_include_directories(myir_camera_test PUBLIC ${TINA_G2D_INC})
    target_link_libraries(myir_camera_test PUBLIC ${TINA_G2D_LIB})
endif()


if (CONFIG_USE_TINA_G2D)
    target_compile_definitions(myir_camera_test PUBLIC TINA_G2D)
endif()

if (CONFIG_USE_ALLWINNER_ISP)
    target_compile_definitions(myir_camera_test PUBLIC USE_ALLWINNER_ISP)
endif()

if (CONFIG_USE_DEBUG)
    target_compile_definitions(myir_camera_test PUBLIC USE_DEBUG)
endif()

# No specific build steps required for FBDEV
target_link_libraries(myir_camera_test PUBLIC m pthread)

