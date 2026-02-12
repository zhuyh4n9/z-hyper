load_addr=0x40200000
entry_point=0x40200000
output_name=Z-Hyper_Uimage
image_name=z-hyper.elf

if [ -d "./build" ]; then
    rm -rf ./build
fi

mkdir -p build
cd build

cmake ..
make
