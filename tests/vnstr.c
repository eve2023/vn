/*
*************************************************************************
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
*************************************************************************
VN.C: eVe's general Infrastructure
*************************************************************************
  gcc vn.c tests/vnstr.c -o test_vnstr
*/
#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void vnstr_init(char *buf);

void vnstr_fromc(char *buf, char *cs);

unsigned short vnstr_len(char *s);

void vnstr_toc(char *buf, char *s);

int vnstr_cmp(char *s, char *t);

void vnstr_append(char *dest, char *src);

int vnstr_pos(char *s, char *t);

void vnstra_init(char **stra, char *buf, char *sep);

void vn_string_map(char *r, char *kvs[], char *s);

void test_vnstr_init()
{
	char *s = strdup("%%string");
	vnstr_init(s);
	assert(vnstr_len(s) == 6);

	printf("TEST vnstr_init PASS\n");
}

void test_vnstr_fromc()
{
	char s[15] = "..hello world";
	vnstr_init(s);
	char *cs = "hello world";
	char buf[50];
	vnstr_fromc(buf, cs);
	assert(vnstr_cmp(buf, s) == 0);

	printf("TEST vnstr_fromc PASS\n");
}

void test_vnstr_len()
{
	char s[15] = "..hello world";
	vnstr_init(s);
	assert(vnstr_len(s) == 11);

	printf("TEST vnstr_len PASS\n");
}

void test_vnstr_toc()
{
	char s[15] = "..hello world";
	vnstr_init(s);
	char cbuf[50];
	vnstr_toc(cbuf, s);
	assert(strcmp(cbuf, "hello world") == 0);

	printf("TEST vnstr_toc PASS\n");
}

void test_vnstr_cmp()
{
	char s[15] = "..hello world";
	char t[15] = "..hello world";
	vnstr_init(s);
	vnstr_init(t);
	assert(vnstr_cmp(s, t) == 0);
	
	printf("TEST vnstr_cmp PASS\n");
}

void test_vnstr_append()
{
	char s[15] = "..hello w";
	char ss[15] = "..orld";
	char t[15] = "..hello world";
	vnstr_init(s);
	vnstr_init(ss);
	vnstr_init(t);

	vnstr_append(s, ss);
	assert(vnstr_cmp(s, t) == 0);

	printf("TEST vnstr_append PASS\n");
}

void test_vnstr_pos()
{
	char s[15] = "..hello test";
	char t[10] = "..test";
	assert(vnstr_pos(s, t) == 6);

	printf("TEST vnstr_pos PASS\n");
}

void test_vnstra_init()
{
	char buf[200] = "%%html%%text/html%%htm%%text/html%%css%%text/css%%js"
		"%%application/javascript%%txt%%text/plain%%c%%text/plain"
		"%%png%%image/png%%svg%%image/svg+xml%%";
	char *stra[20];
	char sep[5] = "%%%%";
	vnstr_init(sep);
	assert(vnstr_len(sep) == 2);
	vnstra_init(stra, buf, sep);
	assert(vnstr_len(stra[0]) == 4);
	assert(vnstr_len(stra[1]) == 9);
	assert(vnstr_len(stra[2]) == 3);
	assert(vnstr_len(stra[3]) == 9);
	assert(vnstr_len(stra[14]) == 3);
	assert(vnstr_len(stra[15]) == 13);

	printf("TEST vnstra_init PASS\n");
}

void test_vn_string_map()
{
	char buf[200] = "..html..text/html..htm..text/html..css..text/css..js"
		"..application/javascript..txt..text/plain..c..text/plain"
		"..png..image/png..svg..image/svg+xml..";
	char sep[5] = "....";
	char *stra[17];
	vnstr_init(sep);
	vnstra_init(stra, buf, sep);
	stra[16] = NULL;

	char r[30];
	vn_string_map(r, stra, stra[0]);
	assert(vnstr_cmp(r, stra[1]) == 0);
	vn_string_map(r, stra, stra[10]);
	assert(vnstr_cmp(r, stra[11]) == 0);

	printf("TEST vn_string_map PASS\n");
}

int main(void)
{
	test_vnstr_init();
	test_vnstr_fromc();
	test_vnstr_len();
	test_vnstr_toc();
	test_vnstr_cmp();
	test_vnstra_init();
	test_vn_string_map();
}
