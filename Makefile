CC = g++
CPPFLAGS = -Wall -Wextra -Werror -O3
LDFLAGS = -ldl

NAME = nibbler

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

$(NAME): $(OBJS)
	$(CC) $(CPPFLAGS) $(OBJS) -o $(NAME) $(LDFLAGS)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean $(NAME)