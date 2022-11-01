CC = g++
CPPFLAGS = -Wall -Wextra -Werror -fPIC -O3 -std=c++11 -I../include-sdl
LDFLAGS = -lSDL2 -lSDL2_ttf -L../lib-sdl -L../lib-sdl_ttf

NAME = libnibbler_sdl.so

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

$(NAME): $(OBJS)
	$(CC) $(CPPFLAGS) -shared $(OBJS) -o $(NAME) $(LDFLAGS)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean $(NAME)
