#ifndef WEBSOCKET_H
# define WEBSOCKET_H

/*
** t_ident describe socket identity
*/

typedef struct		s_ident
{
	char			*addr;
	char			*port;
}					t_ident;

/*
** This struct is passed as *user_data* in callback function.  The
** *fd* member is the file descriptor of the connection to the client.
*/

struct Session
{
  int fd;
};

/*
** http_handshake.c
*/

int					http_handshake(int fd);

#endif
