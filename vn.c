/*
 * Copyright (c) 2023 Eve
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include <arpa/inet.h>
#include <errno.h> // EAGAIN EWOULDBLOCK
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h> // nanosleep

#ifdef MOCK_READ
typedef ssize_t (*ReadFunction)(int fd, void *buf, size_t count);
extern ReadFunction v_read;
#define read v_read
#endif

void vnstr_setlen(char *buf, unsigned short len)
{
	buf[0] = (len >> 8) & 0xff;
	buf[1] = len & 0xff;
}

void vnstr_init(char *buf)
{
	size_t len = strlen(buf + 2);
	vnstr_setlen(buf, len);
}

void vnstr_fromc(char *buf, char *cs)
{
	size_t len = strlen(cs);
	memcpy(buf + 2, cs, len);
	vnstr_setlen(buf, len);
}

unsigned short vnstr_len(char *s)
{
	return ntohs(*(unsigned short *)s);
}

void vnstr_toc(char *buf, char *s)
{
	unsigned short len = vnstr_len(s);
	memcpy(buf, s + 2, len);
	buf[len] = '\0';
}

int vnstr_cmp(char *s, char *t)
{
	unsigned short l = vnstr_len(s);
	unsigned short m = vnstr_len(t);
	int min_len = (l < m) ? l : m;
	int cmp_result = memcmp(s + 2, t + 2, min_len);

	if (cmp_result == 0)
	{
		return l - m;
	}

	return cmp_result;
}

void vnstr_append(char *dest, char *src)
{
	unsigned short dest_len = vnstr_len(dest);
	unsigned short src_len = vnstr_len(src);
	unsigned short new_len = dest_len + src_len;

	vnstr_setlen(dest, new_len);
	memcpy(dest + 2 + dest_len, src + 2, src_len);
}

int vnstr_pos(char *s, char *t)
{
	unsigned short l = vnstr_len(s);
	unsigned short m = vnstr_len(t);

	if (m > l)
		return -1;

	for (int i = 0; i <= l - m; i++)
	{
		if (memcmp(s + 2 + i, t + 2, m) == 0)
		{
			return i;
		}
	}

	return -1;
}

// TODO refactor this with vnstr_str
void vnstra_init(char **stra, char *buf, char *sep)
{
	unsigned short lsep = vnstr_len(sep);

	int i = 0, j = 0;
	while (buf[i])
	{
		if (memcmp(&buf[i], sep + 2, lsep) == 0)
		{
			if (j > 0)
			{
				int len = buf + i - stra[j-1] - lsep;
				stra[j-1][0] = (len >> 8) & 0xff;
				stra[j-1][1] = len & 0xff;
			}

			if (buf[i+lsep])
			{
				stra[j++] = buf + i;
			}

			i+=lsep;
		}
		else
		{
			i++;
		}
	}
}

struct vn_gets_buf
{
  short ln_so; // init: /
  short ln_eo; // init: 0, 0 for incomplete
  short read_eo; // init: 0
  char dat[32762];
};

// TODO nework line ending not handled correctly
void vn_gets(int fd, struct vn_gets_buf *buf)
{
  if (buf->ln_eo > 16384)
    {
      memcpy(buf->dat, &buf->dat[buf->ln_eo], buf->read_eo - buf->ln_eo);
      buf->read_eo -= buf->ln_eo;
      buf->ln_eo = 0;
    }

  buf->ln_so = buf->ln_eo;

  if (buf->ln_eo < buf->read_eo)
    {
#define VN_GETS_SCAN_LINE() \
      for (int i = 0; buf->ln_so+i < buf->read_eo - 1; i++) \
	if (buf->dat[buf->ln_so+i] == '\r' && buf->dat[buf->ln_so+i+1] == '\n') \
	  { \
	    buf->ln_eo += i+2; \
	    return; \
	  } \
\
      buf->ln_eo = buf->read_eo; \
      return;

      VN_GETS_SCAN_LINE();
    }

  size_t bytes_read = 0;
  while (buf->read_eo < sizeof(buf->dat))
    {
      int ret = read(fd, buf->dat + buf->read_eo, sizeof(buf->dat) - buf->read_eo);
      if (ret > 0)
	{
	  buf->read_eo += ret;
	  VN_GETS_SCAN_LINE();
	}
      // TODO sleep and retry; for now it simply quits
      else if (ret == 0)
	{
	  // connection closed by client
	  break;
	}
      else
	{
	  if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
	      struct timespec sleep_time;
	      sleep_time.tv_sec = 0;
	      sleep_time.tv_nsec = 10000000; // 10 ms
	      nanosleep(&sleep_time, NULL);
            }
	  else
            {
	      // error occurred while reading
	      perror("read");
	      break;
           } 
	}
    }
}

char *vn_gets_to_string(struct vn_gets_buf *buf)
{
  char *str = malloc(buf->ln_eo - buf->ln_so + 1);
  if (str == NULL)
    {
      return NULL;
    }

  // TODO this breaks if string contains null byte
  strncpy(str, buf->dat + buf->ln_so, buf->ln_eo - buf->ln_so);
  str[buf->ln_eo - buf->ln_so] = '\0';

  return str;
}

ssize_t vn_read(int fd, char *buf, ssize_t buffer_size)
{
  ssize_t bytes_read, n;
  bytes_read = 0;
  while ((n = read(fd, buf + bytes_read, buffer_size - bytes_read)) > 0)
    {
      if (n > 0)
	{
	  bytes_read += n;
	  if (bytes_read == buffer_size)
	    {
	      printf("File is larger than 1MB.\n");
	    }
	  // TODO sleep and retry; for now it simply quits
	}
      else
	{
	  perror("read");
	  break;
	}
    }

  return bytes_read;
}

void vn_puts(int fd, const void *buf, size_t len)
{
  ssize_t bytes_written = 0;
  while (bytes_written < len)
    {
      ssize_t n = write(fd, buf + bytes_written, len - bytes_written);
      if (n == -1)
        {
	  if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
	      struct timespec sleep_time;
	      sleep_time.tv_sec = 0;
	      sleep_time.tv_nsec = 10000000; // 10 ms
	      nanosleep(&sleep_time, NULL);
            }
	  else
            {
	      perror("write");
	      break;
            }
        }
      else
        {
	  bytes_written += n;
        }
    }
}

void vn_string_map(char *r, char *kvs[], char *s)
{
  size_t len = vnstr_len(s);

  int i, j, k;
  for (i = 0, j = 0; i < len;)
    {
      for (k = 0; kvs[k]; k += 2)
	{
	  if (vnstr_cmp(kvs[k], s+i) == 0)
	    {
	      strcpy(r+2+j, kvs[k+1]+2);
	      i += vnstr_len(kvs[k]);
	      j += vnstr_len(kvs[k+1]);
	      break;
	    }
	}

      if (!kvs[k])
      {
        r[2+j++] = s[2+i++];
      }
    }

  vnstr_setlen(r, j);
}
