CC = g++
CPPFLAGS = -Wall -Wextra -Werror -O3
LDFLAGS = -ldl -lpthread

NAME = nibbler

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

sfml:
	$(MAKE) -C ./sfml

sdl:
	$(MAKE) -C ./sdl

$(NAME): $(OBJS) sfml sdl
	$(CC) $(CPPFLAGS) $(OBJS) -o $(NAME) $(LDFLAGS)

clean:
	$(MAKE) -C ./sfml clean
	$(MAKE) -C ./sdl clean
	rm -f $(OBJS)

fclean: clean
	$(MAKE) -C ./sfml fclean
	$(MAKE) -C ./sdl fclean
	rm -f $(NAME)

re: fclean $(NAME)

.PHONY: all clean fclean re sfml sdl