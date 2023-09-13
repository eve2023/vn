/*
Copyright (c) 2023 Eve
Licensed under the GNU General Public License version 3 (GPLv3)
*************************************************************************
VN.C: eVe's general Infrastructure
*************************************************************************
VN is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any
later version.

VN is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License along
with VN. If not, see <https://www.gnu.org/licenses/>.
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

unsigned short vnstr_len(char *s);

void vnstra_init(char **stra, char *buf, char *sep);

int main(void)
{
	// === TEST vnstr_init ===
	char *s = strdup("%%string");
	vnstr_init(s);
	assert(vnstr_len(s) == 6);
	free(s);

	printf("TEST vnstr_init PASS\n");

	// === TEST vnstra_init ===
	char *bu0 = "%%html%%text/html%%htm%%text/html%%css%%text/css%%js"
		"%%application/javascript%%txt%%text/plain%%c%%text/plain"
		"%%png%%image/png%%svg%%image/svg+xml%%";
	char *buf = strdup(bu0);
	char *stra[20];
	char *sep = strdup("%%%%");
	vnstr_init(sep);
	assert(vnstr_len(sep) == 2);
	vnstra_init(stra, buf, sep);
	assert(vnstr_len(stra[0]) == 4);
	assert(vnstr_len(stra[1]) == 9);
	assert(vnstr_len(stra[2]) == 3);
	assert(vnstr_len(stra[3]) == 9);
	assert(vnstr_len(stra[14]) == 3);
	assert(vnstr_len(stra[15]) == 13);
	free(sep);
	free(buf);

	printf("TEST vnstra_init PASS\n");
}
