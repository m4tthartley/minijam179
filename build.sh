#
#  Created by Matt Hartley on 22/02/2025.
#  Copyright 2025 GiantJelly. All rights reserved.
#

set -e

# Sub process version
# files="main_sub_process.m game.c render.c bitmap.c system_resource.c system.m system_apple.m"
# hostfiles="main_host_process.m system_apple.m"

files="game.c"
hostfiles="main.m system_apple.m"

libs="-framework Cocoa -framework QuartzCore -framework Metal -framework AudioToolbox"
options="-fno-objc-arc"

mkdir -p ./build

clang -g -I../core $files $libs $options -o ./build/game.so --shared
echo "game.so built"
clang -g -I../core $hostfiles $libs $options -o ./build/jam -DHOTRELOAD
echo "jam built"
