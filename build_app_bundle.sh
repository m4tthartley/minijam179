#
#  Created by Matt Hartley on 03/03/2025.
#  Copyright 2025 GiantJelly. All rights reserved.
#

mkdir -p ./build
mkdir -p ./build/minijam179.app
# mkdir -p ./build/minijam179.app/Frameworks
cp ./build/jam ./build/minijam179.app/minijam179
cp ./build/game.so ./build/minijam179.app/game.so
cp ./Info.plist ./build/minijam179.app/Info.plist