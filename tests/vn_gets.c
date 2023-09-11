/*
   该源代码属于公有领域，允许复制、修改和分发. 该源代码或其衍生作品不得用于在
   地藏菩萨看来明显违反十善业道的事业.
   http://34.118.241.22/static/ssyd.txt
   敦促阁下尽职规划自己的事业，使之明显不违反十善业道. 若有违反，请立刻收手.
   以上声明不得翻译.
*************************************************************************
** VN: eVe's general iNfrastructure
*************************************************************************
  gcc vn.c tests/vn_gets.c -D MOCK_READ -o test_vn_gets
*/
#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

struct vn_gets_buf
{
  short ln_so; // init: /
  short ln_eo; // init: 0, 0 for incomplete
  short read_eo; // init: 0
  char dat[32762];
};

void vn_gets(int fd, struct vn_gets_buf *buf);

char *vn_gets_to_string(struct vn_gets_buf *buf);

typedef ssize_t (*ReadFunction)(int fd, void *buf, size_t count);

ReadFunction o_read = read;
ReadFunction v_read;

ssize_t mock_read(int fd, void *buf, size_t count)
{
  static int call_cnt = 0;

  if (call_cnt == 0)
    {
      if (count >= 20000)
	{
	  memset(buf, 'A', 19998);
	  ((char *)buf)[19998] = '\r';
	  ((char *)buf)[19999] = '\n';
	  return 20000;
	}
      else exit(-1);
    }
  else if (call_cnt == 1)
    {
      if (count >= 20000)
	{
	  memset(buf, 'B', 19998);
	  ((char *)buf)[19998] = '\r';
	  ((char *)buf)[19999] = '\n';
	  return 20000;
	}
      else exit(-1);
    }

  // BUG: this line never gets executed
  call_cnt++;
}

ssize_t mock_read2(int fd, void *buf, size_t count)
{
  char *s = "GET / HTTP/1.1\r\nHost: www.example.com\r\nConnection: close\r\n\r\n";
  strncpy(buf, s, count);
  return strlen(s);
}

ssize_t mock_read3(int fd, void *buf, size_t count)
{
  static int call_cnt = 0;

  if (call_cnt++ < 30)
    {
      errno = EAGAIN;
      return -1;
    }
  else
    {
      return 0;
    }
}

int main(void)
{
  struct vn_gets_buf buf;

  // === TEST BUFFER SHIFT AFTER HALF FULL ===
  v_read = mock_read;

  buf.ln_eo = 0;
  buf.read_eo = 0;

  vn_gets(7, &buf);
  assert(buf.ln_eo - buf.ln_so == 20000);

  vn_gets(7, &buf);
  assert(buf.ln_eo - buf.ln_so == 20000);

  printf("TEST BUFFER SHIFT AFTER HALF FULL PASS\n");

  // === TEST MULTILINE READ ===
  v_read = mock_read2;

  buf.ln_eo = 0;
  buf.read_eo = 0;

  vn_gets(8, &buf);
  assert(buf.ln_eo - buf.ln_so == strlen("GET / HTTP/1.1\r\n"));
  assert(buf.read_eo == strlen("GET / HTTP/1.1\r\nHost: www.example.com\r\nConnection: close\r\n\r\n"));

  vn_gets(8, &buf);
  assert(buf.ln_eo - buf.ln_so == strlen("Host: www.example.com\r\n"));

  vn_gets(8, &buf);
  assert(buf.ln_eo - buf.ln_so == strlen("Connection: close\r\n"));

  vn_gets(8, &buf);
  assert(buf.ln_eo - buf.ln_so == strlen("\r\n"));

  printf("TEST MULTILINE READ PASS\n");

  // === TEST vn_gets_to_string ===
  buf.ln_so = 0;
  buf.ln_eo = 10;
  strcpy(buf.dat, "This is a test string.");

  char *s = vn_gets_to_string(&buf);
  printf("The string is: %s\n", s);

  free(s);
  printf("TEST vn_gets_to_string PASS\n");

  // === TEST CLIENT SENGDING NOTHING ===
  v_read = mock_read3;

  buf.ln_eo = 0;
  buf.read_eo = 0;

  vn_gets(9, &buf);
  assert(buf.ln_so == 0);
  assert(buf.ln_eo == 0);

  s = vn_gets_to_string(&buf);
  assert(s[0] == '\0');

  free(s);
  printf("TEST CLIENT SENDING NOTHING PASS\n");
}
