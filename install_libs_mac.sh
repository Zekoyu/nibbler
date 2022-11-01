brew install curl # Download failed: Homebrew-installed `curl` is not installed for: https://downloads.xiph.org/releases/flac/flac-1.4.2.tar.xz
brew install sfml

brew install sdl2

brew install libx11


cd raylib
git clone https://github.com/raysan5/raylib.git raylib
cd raylib/src
make PLATFORM=PLATFORM_DESKTOP RAYLIB_LIBTYPE=SHARED
sudo make install RAYLIB_LIBTYPE=SHARED
cd ../../
