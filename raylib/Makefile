UNAME := $(shell uname)

CC = g++
CPPFLAGS = -Wall -Wextra -Werror -fPIC -O3 -std=c++11 -I../include-raylib
LDFLAGS =

ifeq ($(UNAME), Darwin)
	LDFLAGS += -framework IOKit -framework Cocoa
endif

# ifeq ($(UNAME), Linux)

# endif

NAME = libnibbler_raylib.so

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

$(NAME): $(OBJS)
	$(CC) $(CPPFLAGS) -shared $(OBJS) ../lib-raylib/libraylib.a -o $(NAME) $(LDFLAGS)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean $(NAME)
