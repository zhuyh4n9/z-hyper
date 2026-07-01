cmake_minimum_required(VERSION 3.10)

project(z-hyper)

enable_language(C ASM)

set(CROSS_COMPILE 		aarch64-linux-gnu-)
set(CMAKE_C_COMPILER 	"${CROSS_COMPILE}gcc-14")
set(CMAKE_ASM_COMPILER 	"${CROSS_COMPILE}gcc-14")
set(CMAKE_LINKER 		"${CROSS_COMPILE}ld")
set(CMAKE_ASM_FLAGS 	"-march=armv8-a+nosimd+nofp")

include_directories(${CMAKE_SOURCE_DIR}/include
                    ${CMAKE_SOURCE_DIR}/include/libc)
set(CMAKE_ASM_FLAGS "-ffreestanding -Wextra -Wfatal-errors -Werror -O0 -D__ASSEMBLY__")
set(BUILD_FLAGS "-Wall -Wextra -Werror -O0 -g -nostdlib -ffreestanding -mstrict-align")
set(CMAKE_C_FLAGS "${BUILD_FLAGS} -DNR_CPU=${NR_CPU}")


add_executable(z-hyper.elf
    src/main/start.S
    src/main/el2_sync.c
    src/main/exception.S
    src/main/main.c
    unittest/miniheap_stress_test.c
    src/libc/string.c
    src/libc/printf.c
    src/libc/rand.c
    src/libc/miniheap.c
    src/driver/pl011/pl011.c
    src/driver/gic/gicv3.c
)

target_link_options(z-hyper.elf PRIVATE
    "-T${CMAKE_SOURCE_DIR}/lds/linker.lds"
)