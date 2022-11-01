# GRAPHICS / SOUND
brew install curl # Download failed: Homebrew-installed `curl` is not installed for: https://downloads.xiph.org/releases/flac/flac-1.4.2.tar.xz
brew install sfml

brew install sdl2
brew install sdl2_ttf

if [ ! -d ./raylib/raylib/src ]; then
	cd ./raylib
	git clone https://github.com/raysan5/raylib.git raylib
	cd ./raylib/src
	make PLATFORM=PLATFORM_DESKTOP
	# sudo make install RAYLIB_LIBTYPE=SHARED
	cd ../../
fi

# SYMBOLIC LINKS
./symlink_includes.sh