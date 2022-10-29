# SFML (https://www.sfml-dev.org/tutorials/2.5/start-linux.php)
sudo apt-get install libsfml-dev

# SDL2 (https://wiki.libsdl.org/Installation)
sudo apt-get install libsdl2-dev libsdl2-ttf-dev

# Raylib (https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux)
cd raylib
git clone https://github.com/raysan5/raylib.git raylib
cd raylib/src
make PLATFORM=PLATFORM_DESKTOP RAYLIB_LIBTYPE=SHARED
sudo make install RAYLIB_LIBTYPE=SHARED
cd ../../