set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
cmake_minimum_required(VERSION 3.21)

# Specify cross compilers and tools
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER  arm-none-eabi-gcc)
set(CMAKE_AR arm-none-eabi-ar)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP arm-none-eabi-objdump)
set(SIZE arm-none-eabi-size)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

project(Derisking_SD_DAC_I2S C CXX ASM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)

add_compile_options(-mcpu=cortex-m4 -mthumb -mthumb-interwork)
add_compile_options(-ffunction-sections -fdata-sections -fno-common -fmessage-length=0)
# To force compiler color output when using Ninja.
add_compile_options(-fdiagnostics-color=always)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-parameter")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
# Disable -Wregister, which warns of the use of the register keyword in C++17 and later.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-register")

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    message(STATUS "Maximum optimization for speed")
    add_compile_options(-Ofast)
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
    message(STATUS "Maximum optimization for speed, debug info included")
    add_compile_options(-Ofast -g)
    add_definitions(-DDEBUG)
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel")
    message(STATUS "Maximum optimization for size")
    add_compile_options(-Os)
else ()
    message(STATUS "Minimal optimization, debug info included")
    add_compile_options(-O0 -g)
    add_definitions(-DDEBUG)
endif ()

set(GD32_MCU GD32F407)
add_definitions(-DUSE_STDPERIPH_DRIVER -D${GD32_MCU})

set(GD32_DIR vendor/gd32f4xx/Firmware)
include_directories(
        ${GD32_DIR}/CMSIS
        ${GD32_DIR}/CMSIS/GD/GD32F4xx/Include
        ${GD32_DIR}/GD32F4xx_standard_peripheral/Include
        src
        src/sys
)

set(OUTPUT_DIR ${PROJECT_BINARY_DIR})

add_subdirectory(vendor/gd32f4xx)

file(GLOB_RECURSE SOURCES "src/*.*")

set(FLASH_LD_DIR ${PROJECT_SOURCE_DIR}/vendor/gd32f4xx/Scripts)
set(LINKER_SCRIPT ${FLASH_LD_DIR}/GD32F407RK_FLASH.ld)
add_link_options(-T${LINKER_SCRIPT} -Wl,--cref -Wl,--gc-sections,--print-memory-usage)
add_link_options(-mcpu=cortex-m4 -mthumb -mthumb-interwork)

add_executable(
    ${CMAKE_PROJECT_NAME}.elf
    ${SOURCES}
)

target_link_libraries(
    ${CMAKE_PROJECT_NAME}.elf
    gd32_lib
)

add_link_options(-Wl,-Map=${OUTPUT_DIR}/"${CMAKE_PROJECT_NAME}".map)

set(EXECUTABLE_OUTPUT_PATH ${OUTPUT_DIR})

# Make a .bin and a .hex from the .elf
set(HEX_FILE ${OUTPUT_DIR}/${CMAKE_PROJECT_NAME}.hex)
set(BIN_FILE ${OUTPUT_DIR}/${CMAKE_PROJECT_NAME}.bin)

add_custom_command(TARGET ${CMAKE_PROJECT_NAME}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${CMAKE_PROJECT_NAME}.elf> ${HEX_FILE}
        COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${CMAKE_PROJECT_NAME}.elf> ${BIN_FILE}
        COMMENT "Building ${HEX_FILE}
Building ${BIN_FILE}")
