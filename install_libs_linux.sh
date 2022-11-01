# SFML (https://www.sfml-dev.org/tutorials/2.5/start-linux.php)
sudo apt-get install libsfml-dev

# SDL2 (https://wiki.libsdl.org/Installation)
sudo apt-get install libsdl2-dev libsdl2-ttf-dev

# Raylib (https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux)
if [ ! -d ./raylib/raylib/src ]; then
	cd raylib
	git clone https://github.com/raysan5/raylib.git raylib
	cd raylib/src
	make PLATFORM=PLATFORM_DESKTOP CUSTOM_CFLAGS="-DSUPPORT_FILEFORMAT_BMP=1"
	cd ../../
fi