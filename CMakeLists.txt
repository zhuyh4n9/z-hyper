cmake_minimum_required(VERSION 3.10)

project(z-hyper C ASM)

enable_language(C ASM)

# ── 交叉编译工具链 ──────────────────────────────────
set(CROSS_COMPILE       aarch64-linux-gnu-)
set(CMAKE_C_COMPILER    "${CROSS_COMPILE}gcc-14")
set(CMAKE_ASM_COMPILER  "${CROSS_COMPILE}gcc-14")
set(CMAKE_LINKER        "${CROSS_COMPILE}ld")

# ── 全局 include 路径 ───────────────────────────────
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/libc
    ${CMAKE_SOURCE_DIR}/platform/aarch64/include
    ${CMAKE_SOURCE_DIR}/platform/aarch64/include/driver
    ${CMAKE_SOURCE_DIR}/platform/aarch64/driver/gic/common/
)

# ── 编译标志 ────────────────────────────────────────
set(CMAKE_ASM_FLAGS  "-march=armv8-a+nosimd+nofp")
set(CMAKE_ASM_FLAGS  "-ffreestanding -Wextra -Wfatal-errors -Werror -O0 -D__ASSEMBLY__")
set(BUILD_FLAGS      "-Wall -Wextra -Werror -O0 -g -nostdlib -ffreestanding -mstrict-align")
set(CMAKE_C_FLAGS    "${BUILD_FLAGS} -DNR_CPU=${NR_CPU} -DHZ=${HZ} -DTRACE_ON=${TRACE_ON}")

# ── 子模块 ──────────────────────────────────────────
add_subdirectory(src/libc)
add_subdirectory(platform/aarch64/main)
add_subdirectory(platform/aarch64/timer)
add_subdirectory(platform/aarch64/driver)
# add_subdirectory(platform/aarch64/vcpu)   # 开发中
add_subdirectory(unittest)

# ── 链接 ────────────────────────────────────────────
add_executable(z-hyper.elf
    $<TARGET_OBJECTS:libc>
    $<TARGET_OBJECTS:platform_main>
    $<TARGET_OBJECTS:platform_timer>
    $<TARGET_OBJECTS:platform_driver>
    $<TARGET_OBJECTS:unittest>
)

target_link_options(z-hyper.elf PRIVATE
    "-T${CMAKE_SOURCE_DIR}/lds/linker.lds"
)