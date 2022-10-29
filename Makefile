CC = g++
CPPFLAGS = -Wall -Wextra -Werror -O3
LDFLAGS = -ldl -lpthread

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

$(NAME): $(OBJS) sfml sdl raylib
	$(CC) $(CPPFLAGS) $(OBJS) -o $(NAME) $(LDFLAGS)

clean:
	$(MAKE) -C ./sfml clean
	$(MAKE) -C ./sdl clean
	$(MAKE) -C ./raylib clean
	rm -f $(OBJS)

fclean: clean
	$(MAKE) -C ./sfml fclean
	$(MAKE) -C ./sdl fclean
	$(MAKE) -C ./raylib fclean
	rm -f $(NAME)

re: fclean $(NAME)

.PHONY: all clean fclean re sfml sdl raylib