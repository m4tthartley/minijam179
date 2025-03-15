#
#  Created by Matt Hartley on 22/02/2025.
#  Copyright 2025 GiantJelly. All rights reserved.
#

set -e

files="game.c render.c bitmap.c system_resource.c system.m"
libs="-framework Cocoa -framework QuartzCore -framework Metal -framework AudioToolbox"
options="-fno-objc-arc"

mkdir -p ./build

clang -g -I../core $files $libs $options -o ./build/game.so --shared
echo "game.so built"
clang -g -I../core main.m game.c render.c bitmap.c system_apple.m system.m $libs $options -o ./build/jam -DHOTRELOAD
echo "jam built"
