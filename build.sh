load_addr=0x40200000
entry_point=0x40200000
output_name=Z-Hyper_Uimage
image_name=z-hyper.elf

NR_CPU=1
HZ=250
TRACE_ON=1

TOP_DIR=$(pwd)

if [ -d "./build" ]; then
    rm -rf ./build
fi

mkdir -p build
cd build

# usage: build.sh [-b ninja|make] [-a KEY=VALUE]... [-f CONFIG_FILE]
BUILD_SYSTEM="ninja"
declare -A CONFIG=(
    [NR_CPU]=1
    [HZ]=250
    [TRACE_ON]=1
)

# 解析 -f 指定的配置文件（KEY=VALUE 格式，每行一个，# 开头为注释）
parse_config_file() {
    local file="$1"
    if [ ! -f "$file" ]; then
        echo "error: config file '$file' not found"
        exit 1
    fi
    while IFS='=' read -r key value; do
        # 跳过空行和注释
        [[ -z "$key" || "$key" == \#* ]] && continue
        # 去掉首尾空白
        key=$(echo "$key" | xargs)
        value=$(echo "$value" | xargs)
        CONFIG["$key"]="$value"
        echo "  [config] $key=$value"
    done < "$file"
}

while getopts "b:a:f:hr:" opt; do
    case $opt in
        b)
            BUILD_SYSTEM="$OPTARG"
            ;;
        a)
            IFS='=' read -r key value <<< "$OPTARG"
            CONFIG["$key"]="$value"
            ;;
        f)
            echo "reading config from: $OPTARG"
            parse_config_file "$OPTARG"
            ;;
        h)
            echo "usage: $0 [-b ninja|make] [-a KEY=VALUE]... [-f CONFIG_FILE]"
            echo "  -b  Choose build system: ninja (default) or make"
            echo "  -a  Pass configuration items, can be used multiple times, e.g.: -a NR_CPU=4 -a HZ=1000"
            echo "  -f  Read configuration from a file in KEY=VALUE format"
            echo ""
            exit 0
            ;;
        r)
            RUN_MODE="$OPTARG"
            ;;
        *)
            echo "usage: $0 [-b ninja|make] [-a KEY=VALUE]... [-f CONFIG_FILE]"
            exit 1
            ;;
    esac
done

CMAKE_DEFS=""
for key in "${!CONFIG[@]}"; do
    CMAKE_DEFS+=" -D${key}=${CONFIG[$key]}"
done

echo "build system : $BUILD_SYSTEM"
echo "config       :${CMAKE_DEFS}"

if [ "$BUILD_SYSTEM" = "ninja" ]; then
    cmake .. -G Ninja ${CMAKE_DEFS} -DCMAKE_BUILD_TYPE=Debug
    ninja
elif [ "$BUILD_SYSTEM" = "make" ]; then
    cmake .. ${CMAKE_DEFS} -DCMAKE_BUILD_TYPE=Debug
    make
else
    echo "error: unknown build system '$BUILD_SYSTEM', use ninja or make"
    exit 1
fi

if [ "$RUN_MODE" = "normal" ]; then
    echo "running z-hyper..."
    bash $TOP_DIR/run.sh -r $TOP_DIR/build/z-hyper.elf
elif [ "$RUN_MODE" = "debug" ]; then
    echo "running z-hyper in debug mode..."
    bash $TOP_DIR/run.sh -d $TOP_DIR/build/z-hyper.elf
fi