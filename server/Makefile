NAME = ws_server

SRCS = main.c http_handshake.c

OBJS = $(SRCS:.c=.o)

WSLAY_LIB_DIR = -L ./wslay/lib/.libs/

WSLAY_INC_DIR = -I ./wslay/lib/includes/

LIBS = -lnettle -lwslay

CC = gcc

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) -o $(NAME) $(WSLAY_LIB_DIR) $(OBJS) $(LIBS)

$(OBJS):$(SRCS)
	$(CC) -c $(WSLAY_INC_DIR) $(SRCS)

clean:
	rm -rf $(OBJS)

fclean: clean
	rm -rf $(NAME)

re: fclean all
