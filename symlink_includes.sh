#!/bin/zsh
sfml=( ~/.brew/Cellar/sfml/*([1]))
ln -sf $sfml/include ./include-sfml
ln -sf $sfml/lib ./lib-sfml

sdl=( ~/.brew/Cellar/sdl2/*([1]))
ln -sf $sdl/include ./include-sdl
ln -sf $sdl/lib ./lib-sdl

sdl_ttf=( ~/.brew/Cellar/sdl2_ttf/*([1]))
ln -sf $sdl_ttf/include/SDL2/SDL_ttf.h $sdl/include/SDL2/SDL_ttf.h
ln -sf $sdl_ttf/lib ./lib-sdl_ttf

raylib=(./raylib/raylib/src/)
ln -sf $raylib ./include-raylib
ln -sf $raylib ./lib-raylib
