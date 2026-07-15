cmake_minimum_required(VERSION 3.10)

project(z-hyper)

enable_language(C ASM)

set(CROSS_COMPILE 		aarch64-linux-gnu-)
set(CMAKE_C_COMPILER 	"${CROSS_COMPILE}gcc-14")
set(CMAKE_ASM_COMPILER 	"${CROSS_COMPILE}gcc-14")
set(CMAKE_LINKER 		"${CROSS_COMPILE}ld")
set(CMAKE_ASM_FLAGS 	"-march=armv8-a+nosimd+nofp")

include_directories(${CMAKE_SOURCE_DIR}/include
                    ${CMAKE_SOURCE_DIR}/include/libc
                    ${CMAKE_SOURCE_DIR}/platform/aarch64/include
                    ${CMAKE_SOURCE_DIR}/platform/aarch64/include/driver
                    ${CMAKE_SOURCE_DIR}/platform/aarch64/driver/gic/common/)
set(CMAKE_ASM_FLAGS "-ffreestanding -Wextra -Wfatal-errors -Werror -O0 -D__ASSEMBLY__")
set(BUILD_FLAGS "-Wall -Wextra -Werror -O0 -g -nostdlib -ffreestanding -mstrict-align")
set(CMAKE_C_FLAGS "${BUILD_FLAGS} -DNR_CPU=${NR_CPU} -DHZ=${HZ} -DTRACE_ON=${TRACE_ON}")

add_executable(z-hyper.elf
    src/libc/string.c
    src/libc/printf.c
    src/libc/rand.c
    src/libc/miniheap.c
    src/libc/console.c
    platform/aarch64/main/start.S
    platform/aarch64/main/el2_sync.c
    platform/aarch64/main/exception.S
    platform/aarch64/main/main.c
    unittest/miniheap_stress_test.c
    platform/aarch64/timer/timer.c
    platform/aarch64/driver/pl011/pl011.c
    platform/aarch64/driver/gic/common/gicv3.c
    platform/aarch64/driver/gic/common/gicr.c
    platform/aarch64/driver/gic/common/gicd.c
    platform/aarch64/driver/gic/common/intr/spi.c
    platform/aarch64/driver/gic/common/intr/ppi.c
    platform/aarch64/driver/gic/common/intr/sgi.c
)

target_link_options(z-hyper.elf PRIVATE
    "-T${CMAKE_SOURCE_DIR}/lds/linker.lds"
)