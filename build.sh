#
#  Created by Matt Hartley on 22/02/2025.
#  Copyright 2025 GiantJelly. All rights reserved.
#

set -e

files="system_apple.m game.c render.c system_audio_apple.m bitmap.c"
libs="-framework Cocoa -framework QuartzCore -framework Metal -framework AudioToolbox"
options="-fno-objc-arc"

mkdir -p ./build

clang -g -I../core $files $libs $options -o ./build/game.so --shared
echo "game.so built"
clang -g -I../core main.m $libs $options -o ./build/jam -DHOTRELOAD
echo "jam built"
