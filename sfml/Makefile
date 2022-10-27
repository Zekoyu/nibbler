CC = g++
CPPFLAGS = -Wall -Wextra -Werror -O3 -fPIC
LDFLAGS = -lsfml-graphics -lsfml-window -lsfml-system

NAME = libsfml.so

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

$(NAME): $(OBJS)
	$(CC) $(CPPFLAGS) -shared $(OBJS) -o $(NAME) $(LDFLAGS)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean $(NAME)
