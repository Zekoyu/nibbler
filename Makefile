CC = g++
CPPFLAGS = -Wall -Wextra -Werror -O3 -std=c++11 -I./include-sfml
LDFLAGS = -ldl -lpthread -lsfml-window -L./lib-sfml

NAME = nibbler

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

sfml:
	@echo Compiling SFML lib
	$(MAKE) -C ./sfml

sdl:
	@echo Compiling SDL lib
	$(MAKE) -C ./sdl

raylib:
	@echo Compiling Raylib lib
	$(MAKE) -C ./raylib

sfml-sound:
	@echo Compiling SFML Sound lib
	$(MAKE) -C ./sfml-sound

$(NAME): $(OBJS) sfml sdl raylib sfml-sound
	$(CC) $(CPPFLAGS) $(OBJS) -o $(NAME) $(LDFLAGS)

clean:
	$(MAKE) -C ./sfml clean
	$(MAKE) -C ./sdl clean
	$(MAKE) -C ./raylib clean
	$(MAKE) -C ./sfml-sound clean
	rm -f $(OBJS)

fclean: clean
	$(MAKE) -C ./sfml fclean
	$(MAKE) -C ./sdl fclean
	$(MAKE) -C ./raylib fclean
	$(MAKE) -C ./sfml-sound fclean
	rm -f $(NAME)

re: fclean $(NAME)

.PHONY: all clean fclean re sfml sdl raylib sfml-sound