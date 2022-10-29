CC = g++
CPPFLAGS = -Wall -Wextra -Werror -fPIC -O3
# https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux
LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

NAME = libnibbler_raylib.so

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

$(NAME): $(OBJS)
	$(CC) $(CPPFLAGS) -shared $(OBJS) -o $(NAME) $(LDFLAGS)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean $(NAME)
