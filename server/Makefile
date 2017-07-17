NAME = ws_server

SRCS = main.c http_handshake.c

OBJS = $(SRCS:.c=.o)

WSLAY_LIB_DIR = -L .libs/wslay/lib/.libs/ -L .libs/nettle-3.3/

WSLAY_INC_DIR = -I .libs/wslay/lib/includes/ -I .libs/nettle-3.3/

LIBS = -lnettle -lwslay

CC = gcc

all: $(NAME)

static: $(OBJS)
	$(CC) -static -o $(NAME) $(WSLAY_LIB_DIR) $(OBJS) $(LIBS)

$(NAME): $(OBJS)
	$(CC) -o $(NAME) $(WSLAY_LIB_DIR) $(OBJS) $(LIBS)

$(OBJS):$(SRCS)
	$(CC) -c $(WSLAY_INC_DIR) $(SRCS)

clean:
	rm -rf $(OBJS)

fclean: clean
	rm -rf $(NAME)

re: fclean all
