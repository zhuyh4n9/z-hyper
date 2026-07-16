add_library(platform_driver OBJECT
    pl011/pl011.c
    gic/common/gicv3.c
    gic/common/gicr.c
    gic/common/gicd.c
    gic/common/intr/spi.c
    gic/common/intr/ppi.c
    gic/common/intr/sgi.c
)

