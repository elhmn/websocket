#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <wslay/wslay.h>
#include "websocket.h"


extern char			*optarg;
extern int			optind;
extern int			opterr;
extern int			optopt;

/*
 ** Here is a little TCP websocket server
 */

int					init_ident(t_ident *ident)
{
	if (!ident)
	{
		fprintf(stderr, "Error : ident is set to NULL");
		return (1);
	}
	ident->addr = NULL;
	ident->port = NULL;
}

int					option_handler(int ac, char **av, t_ident *ident)
{
	char			*opts;

	opts = "a:p:h";
	return (0);
}

int					put_addr(struct sockaddr *addr, socklen_t len)
{
	char			hostbuf[NI_MAXHOST];
	char			servbuf[NI_MAXSERV];
	int				s;

	if ((s = getnameinfo(addr, len, hostbuf, NI_MAXHOST,
						servbuf, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV)) != 0)
	{
		fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));
		return (-1);
	}
	fprintf(stdout, "HOST = [%s] , SERV = [%s] \n",
			hostbuf, servbuf);
	return (0);
}

/*
** not sure this actually works
*/

int					put_socket_ipv6(int sock)
{
	struct sockaddr_in6		addr;
	socklen_t				len;
	char					buffer[INET6_ADDRSTRLEN];

	len = sizeof(struct sockaddr_in6);
	if ((getsockname(sock,
					(struct sockaddr*)&addr, &len)) < 0)
	{
		perror("getsockname");
		return (-1);
	}
	inet_ntop(AF_INET6, &addr.sin6_addr, buffer, INET6_ADDRSTRLEN);
	fprintf(stdout, "IP = [%s] , PORT = [%u] \n",
			buffer, htons(addr.sin6_port));
	return (0);
}

int					put_socket(int sock)
{
	struct sockaddr_in		addr;
	socklen_t				len;

	len = sizeof(struct sockaddr_in);
	if ((getsockname(sock,
					(struct sockaddr*)&addr, &len)) < 0)
	{
		perror("getsockname");
		return (-1);
	}
	fprintf(stdout, "IP = [%s] , PORT = [%u] \n",
			inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
	return (0);
}

int					create_sock_stream_ipv6(char *port, char *proto_name)
{
	int					sock;
	int					len;
	struct addrinfo		hints;
	struct addrinfo		*res;
	struct addrinfo		*rp;
	struct protoent		*protoent;
	int					s;
	int					privilege;

	if (proto_name && !(protoent = getprotobyname(proto_name)))
	{
		perror("protoname");
		return (-1);
	}
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = protoent->p_proto;
	if ((s = getaddrinfo(NULL, port, &hints, &res)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		return (-1);
	}
	for (rp = res; rp != NULL; rp = rp->ai_next)
	{
		privilege = 1;
		if ((sock = socket(AF_INET6, SOCK_STREAM, 0)) < 0)
		{
			perror("socket");
			close(sock);
			continue ;
		}
		if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
				&privilege, (socklen_t)sizeof(privilege)) == -1)
		{
			close(sock);
			continue;
		}
		if (bind(sock, rp->ai_addr, rp->ai_addrlen) == 0)
			break ;
		close(sock);
	}
	if (rp == NULL)
	{
		exit(EXIT_FAILURE);
	}
	freeaddrinfo(res);
	return (sock);
}

int					create_sock_stream(char *host_name,
										char *serv_name,
										int port,
										char *proto_name)
{
	int					sock;
	int					len;
	int					privilege;
	struct sockaddr_in	addr;
	struct protoent		*protoent;
	struct servent		*servent;
	struct hostent		*hostent;

	privilege = 1;
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket");
		return (-1);
	}
	if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
			&privilege, (socklen_t)sizeof(privilege)) == -1)
	{
		perror("setsockopt");
		return (-1);
	}
	if (host_name && !(hostent = gethostbyname(host_name)))
	{
		perror("gethostbyname");
		return (-1);
	}
	if (proto_name && !(protoent = getprotobyname(proto_name)))
	{
		perror("getprotobyname");
		return (-1);
	}
	if (serv_name && !(servent = getservbyname(serv_name, protoent->p_name)))
	{
		perror("getservbyname");
		return (-1);
	}
	memset(&addr, 0, sizeof(struct addr_in*));
	addr.sin_family = AF_INET;
	addr.sin_port = (serv_name)
		? servent->s_port
		: htons(port);
	addr.sin_addr.s_addr = (host_name)
		? ((struct in_addr*)hostent->h_addr)->s_addr
		: htons(INADDR_ANY);
	if (bind(sock, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) < 0)
	{
		perror("bind");
		return (-1);
	}
	return (sock);
}

int					quit_server(void)
{
	return (0);
}

/*
 * Makes file descriptor *fd* non-blocking mode.
 * This function returns 0, or returns -1.
 */
int 				make_non_block(int fd)
{
	int				flags;
	int				r;

	if((flags = fcntl(fd, F_GETFL, 0)) == -1)
	{
		perror("fcntl");
		return -1;
	}
	if((r = fcntl(fd, F_SETFL, flags | O_NONBLOCK)) == -1)
	{
		perror("fcntl");
		return -1;
	}
	return 0;
}

ssize_t send_callback(wslay_event_context_ptr ctx,
                      const uint8_t *data, size_t len, int flags,
                      void *user_data)
{
  struct Session *session = (struct Session*)user_data;
  ssize_t r;
  int sflags = 0;
#ifdef MSG_MORE
  if(flags & WSLAY_MSG_MORE) {
    sflags |= MSG_MORE;
  }
#endif // MSG_MORE
  while((r = send(session->fd, data, len, sflags)) == -1 && errno == EINTR);
  if(r == -1) {
    if(errno == EAGAIN || errno == EWOULDBLOCK) {
      wslay_event_set_error(ctx, WSLAY_ERR_WOULDBLOCK);
    } else {
      wslay_event_set_error(ctx, WSLAY_ERR_CALLBACK_FAILURE);
    }
  }
  return r;
}

ssize_t recv_callback(wslay_event_context_ptr ctx, uint8_t *buf, size_t len,
                      int flags, void *user_data)
{
  struct Session *session = (struct Session*)user_data;
  ssize_t r;
  while((r = recv(session->fd, buf, len, 0)) == -1 && errno == EINTR);
  if(r == -1)
  {
    if(errno == EAGAIN || errno == EWOULDBLOCK) {
      wslay_event_set_error(ctx, WSLAY_ERR_WOULDBLOCK);
    }
	else
	{
      wslay_event_set_error(ctx, WSLAY_ERR_CALLBACK_FAILURE);
    }
  }
  else if(r == 0)
  {
    /* Unexpected EOF is also treated as an error */
    wslay_event_set_error(ctx, WSLAY_ERR_CALLBACK_FAILURE);
    r = -1;
  }
//   else
//   {
// 	  fprintf(stdout, buf);
//   }
  return r;
}

void on_msg_recv_callback(wslay_event_context_ptr ctx,
                          const struct wslay_event_on_msg_recv_arg *arg,
                          void *user_data)
{
  /* Echo back non-control message */
  if(!wslay_is_ctrl_frame(arg->opcode))
  {
    struct wslay_event_msg msgarg = {
										arg->opcode,
										arg->msg,
										arg->msg_length
								    };
		msgarg.msg = "je suis con";
		msgarg.msg_length = strlen(msgarg.msg);
		wslay_event_queue_msg(ctx, &msgarg);
  }
}

int					communication_handler(int sock)
{
	wslay_event_context_ptr 		ctx;
	struct wslay_event_callbacks 	callbacks = {
													recv_callback,
													send_callback,
													NULL,
													NULL,
													NULL,
													NULL,
													on_msg_recv_callback
												};
	struct Session 								session = {sock};
	struct pollfd								event;

#ifdef IPV6
	struct sockaddr_in6							addr;
#else
	struct sockaddr_in							addr;
#endif

	socklen_t									len;
	char										buffer[256];
	char										msg[256];

#ifdef IPV6
	len = sizeof(struct sockaddr_in6);
#else
	len = sizeof(struct sockaddr_in);
#endif

	if (getpeername(sock, (struct sockaddr*)&addr, &len) < 0)
	{
		perror("getpeername");
		return (-1);
	}
	fprintf(stdout, "len = [%u]\n", len);
	put_addr((struct sockaddr*)&addr, len);
	if (http_handshake(sock) < 0)
	{
		perror("http_handshake");
		return (-1);
	}
	if (make_non_block(sock) < 0)
	{
		perror("make_non_block");
		return (-1);
	}
	memset(&event, 0, sizeof(struct pollfd));
	event.fd = sock;
	event.events = POLLIN;
	wslay_event_context_server_init(&ctx, &callbacks, &session);
	while(wslay_event_want_read(ctx) || wslay_event_want_write(ctx))
	{
		if(poll(&event, 1, -1) == -1)
		{
			perror("poll");
			return (-1);
		}
		if(((event.revents & POLLIN) && wslay_event_recv(ctx) != 0)
			|| ((event.revents & POLLOUT) && wslay_event_send(ctx) != 0)
			|| (event.revents & (POLLERR | POLLHUP | POLLNVAL)))
		{
			/*
			 * If either wslay_event_recv() or wslay_event_send() return
			 * non-zero value, it means serious error which prevents wslay
			 * library from processing further data, so WebSocket connection
			 * must be closed.
			 */
			return (-1);
		}
		event.events = 0;
		if(wslay_event_want_read(ctx))
		{
			event.events |= POLLIN;
		}
		if(wslay_event_want_write(ctx))
		{
			event.events |= POLLOUT;
		}
	}
	close(sock);
	return (0);
}

int					connection_handler(int sock)
{
	int					sock_com;
	socklen_t			len;
	struct sockaddr_in6	addr;

	len = sizeof(struct sockaddr_in6);
	if (listen(sock, 5) < 0)
	{
		perror("listen");
		return (-1);
	}
	while (!quit_server())
	{
		memset(&addr, 0, len);
		if ((sock_com = accept(sock, (struct sockaddr*)&addr, &len)) < 0)
		{
			perror("accept");
			return (-1);
		}
		switch (fork())
		{
			/*
			** Child
			*/
			case 0:
				close(sock);
				/*
#ifdef IPV6
				put_socket_ipv6(sock_com);
#else
				put_socket(sock_com);
#endif
				*/
				communication_handler(sock_com);
				exit(EXIT_SUCCESS);
				break;
			case -1:
				perror("fork");
				return (-1);
			/*
			** Father
			*/
			default:
				close(sock_com);
				signal(SIGCHLD, SIG_IGN);/* That's dirty */
		}
	}
	return (0);
}

int					main(int ac, char **av)
{
	int				sock_con;
	char			*serv;
	char			*port;

	serv = NULL;

#ifdef IPV6
	port = NULL;
#else
	port = "0";
#endif

	if (ac > 1)
	{
		port = av[1];
		if (ac == 3)
			serv = av[2];
	}

#ifdef IPV6
	fprintf(stdout, "ipv6\n"); //Debug
	if ((sock_con = create_sock_stream_ipv6(port, "tcp")) < 0)
	{
		fprintf(stderr, "ERROR : sock_con < 0\n");
		exit(EXIT_FAILURE);
	}
	put_socket_ipv6(sock_con);
#else
	fprintf(stdout, "ipv4\n"); //Debug
	if ((sock_con = create_sock_stream(NULL, NULL, atoi(port), "tcp")) < 0)
	{
		fprintf(stderr, "ERROR : sock_con < 0\n");
		exit(EXIT_FAILURE);
	}
	put_socket(sock_con);
#endif

	fprintf(stdout, "websocket server adress : ");
	connection_handler(sock_con);
	close(sock_con);
	return (EXIT_SUCCESS);
}
