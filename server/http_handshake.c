/*
** Wslay - The WebSocket Library
**
** Copyright (c) 2011, 2012 Tatsuhiro Tsujikawa
**
** Permission is hereby granted, free of charge, to any person obtaining
** a copy of this software and associated documentation files (the
** "Software"), to deal in the Software without restriction, including
** without limitation the rights to use, copy, modify, merge, publish,
** distribute, sublicense, and/or sell copies of the Software, and to
** permit persons to whom the Software is furnished to do so, subject to
** the following conditions:
**
** The above copyright notice and this permission notice shall be
** included in all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
** NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
** LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
** OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
** WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*
** http_handshake
**
** Dependency: nettle-dev
**
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <sys/signal.h>
#include "base64.h"
#include "sha.h"


/*
** Calculates SHA-1 hash of *src*. The size of *src* is *src_length* bytes.
** *dst* must be at least SHA1_DIGEST_SIZE.
*/

void sha1(uint8_t *dst, const uint8_t *src, size_t src_length)
{
	struct sha1_ctx ctx;
	sha1_init(&ctx);
	sha1_update(&ctx, src_length, src);
	sha1_digest(&ctx, SHA1_DIGEST_SIZE, dst);
}

/*
** Base64-encode *src* and stores it in *dst*.
** The size of *src* is *src_length*.
** *dst* must be at least BASE64_ENCODE_RAW_LENGTH(src_length).
*/

void base64(uint8_t *dst, const uint8_t *src, size_t src_length)
{
	struct base64_encode_ctx ctx;
	base64_encode_init(&ctx);
	base64_encode_raw(dst, src_length, src);
}

#define WS_GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

/*
** Create Server's accept key in *dst*.
** *client_key* is the value of |Sec-WebSocket-Key| header field in
** client's handshake and it must be length of 24.
** *dst* must be at least BASE64_ENCODE_RAW_LENGTH(20)+1.
*/

void create_accept_key(char *dst, const char *client_key)
{
	uint8_t sha1buf[20], key_src[60];
	memcpy(key_src, client_key, 24);
	memcpy(key_src+24, WS_GUID, 36);
	sha1(sha1buf, key_src, sizeof(key_src));
	base64((uint8_t*)dst, sha1buf, 20);
	dst[BASE64_ENCODE_RAW_LENGTH(20)] = '\0';
}

/* We parse HTTP header lines of the format
**   \r\nfield_name: value1, value2, ... \r\n
**
** If the caller is looking for a specific value, we return a pointer to the
** start of that value, else we simply return the start of values list.
*/

static char*	http_header_find_field_value(char *header,
												char *field_name,
												char *value)
{
	char *header_end,
		 *field_start,
		 *field_end,
		 *next_crlf,
		 *value_start;
	int field_name_len;

	/* Pointer to the last character in the header */
	header_end = header + strlen(header) - 1;

	field_name_len = strlen(field_name);

	field_start = header;

	do{
		field_start = strstr(field_start+1, field_name);

		field_end = field_start + field_name_len - 1;

		if(field_start != NULL
				&& field_start - header >= 2
				&& field_start[-2] == '\r'
				&& field_start[-1] == '\n'
				&& header_end - field_end >= 1
				&& field_end[1] == ':')
		{
			break; /* Found the field */
		}
		else
		{
			continue; /* This is not the one; keep looking. */
		}
	} while(field_start != NULL);

	if(field_start == NULL)
		return NULL;

	/* Find the field terminator */
	next_crlf = strstr(field_start, "\r\n");

	/* A field is expected to end with \r\n */
	if(next_crlf == NULL)
		return NULL; /* Malformed HTTP header! */

	/* If not looking for a value, then return a pointer to the start of values string */
	if(value == NULL)
		return field_end+2;

	value_start = strstr(field_start, value);

	/* Value not found */
	if(value_start == NULL)
		return NULL;

	/* Found the value we're looking for */
	if(value_start > next_crlf)
		return NULL; /* ... but after the CRLF terminator of the field. */

	/* The value we found should be properly delineated from the other tokens */
	if(isalnum(value_start[-1]) || isalnum(value_start[strlen(value)]))
		return NULL;

	return value_start;
}

/*
** Performs HTTP handshake. *fd* is the file descriptor of the
** connection to the client. This function returns 0 if it succeeds,
** or returns -1.
*/

int http_handshake(int fd)
{

/*
** Note: The implementation of HTTP handshake in this function is
** written for just a example of how to use of wslay library and is
** not meant to be used in production code.  In practice, you need
** to do more strict verification of the client's handshake.
*/

	char header[16384], accept_key[29], *keyhdstart, *keyhdend, res_header[256];
	size_t header_length = 0, res_header_sent = 0, res_header_length;
	ssize_t r;
	while(42)
	{
		while((r = read(fd, header+header_length,
						sizeof(header)-header_length)) == -1 && errno == EINTR);
		if(r == -1)
		{
			perror("read");
			return -1;
		}
		else if(r == 0)
		{
			fprintf(stderr, "HTTP Handshake: Got EOF");
			return -1;
		}
		else
		{
			header_length += r;
			if(header_length >= 4 &&
					memcmp(header+header_length-4, "\r\n\r\n", 4) == 0) {
				break;
			} else if(header_length == sizeof(header)) {
				fprintf(stderr, "HTTP Handshake: Too large HTTP headers");
				return -1;
			}
		}
	}

	if(http_header_find_field_value(header, "Upgrade", "websocket") == NULL
		|| http_header_find_field_value(header, "Connection", "Upgrade") == NULL
		|| (keyhdstart = http_header_find_field_value(header, "Sec-WebSocket-Key", NULL)) == NULL)
	{
		fprintf(stderr, "HTTP Handshake: Missing required header fields");
		return -1;
	}
	for(; *keyhdstart == ' '; ++keyhdstart);
	keyhdend = keyhdstart;
	for(; *keyhdend != '\r' && *keyhdend != ' '; ++keyhdend);
	if(keyhdend-keyhdstart != 24)
	{
		printf("%s\n", keyhdstart);
		fprintf(stderr, "HTTP Handshake: Invalid value in Sec-WebSocket-Key");
		return -1;
	}
	create_accept_key(accept_key, keyhdstart);
	snprintf(res_header, sizeof(res_header),
			"HTTP/1.1 101 Switching Protocols\r\n"
			"Upgrade: websocket\r\n"
			"Connection: Upgrade\r\n"
			"Sec-WebSocket-Accept: %s\r\n"
			"\r\n", accept_key);
	res_header_length = strlen(res_header);
	while(res_header_sent < res_header_length)
	{
		while((r = write(fd, res_header+res_header_sent,
						res_header_length-res_header_sent)) == -1
						&& errno == EINTR);
		if(r == -1)
		{
			perror("write");
			return -1;
		}
		else
		{
			res_header_sent += r;
		}
	}
	return 0;
}
