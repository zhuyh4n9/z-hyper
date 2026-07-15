load_addr=0x40200000
entry_point=0x40200000
output_name=Z-Hyper_Uimage
image_name=z-hyper.elf

NR_CPU=1
HZ=250
if [ -d "./build" ]; then
    rm -rf ./build
fi

mkdir -p build
cd build

cmake .. -DNR_CPU=${NR_CPU} -DHZ=${HZ} -DCMAKE_BUILD_TYPE=Debug
make
