CC = g++
CPPFLAGS = -Wall -Wextra -Werror -O3
LDFLAGS = -ldl -lpthread

NAME = nibbler

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

sfml:
	$(MAKE) -C ./sfml

$(NAME): $(OBJS) sfml
	$(CC) $(CPPFLAGS) $(OBJS) -o $(NAME) $(LDFLAGS)

clean:
	$(MAKE) -C ./sfml clean
	rm -f $(OBJS)

fclean: clean
	$(MAKE) -C ./sfml fclean
	rm -f $(NAME)

re: fclean $(NAME)

.PHONY: all clean fclean re sfml