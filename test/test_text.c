
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <roint.h>

const char *cp949 =  "\xc0\xaf" "\xc0\xfa" "\xc0\xce" "\xc5\xcd" "\xc6\xe4" "\xc0\xcc" "\xbd\xba";
const char *utf8 = "\xec\x9c\xa0" "\xec\xa0\x80" "\xec\x9d\xb8" "\xed\x84\xb0" "\xed\x8e\x98" "\xec\x9d\xb4" "\xec\x8a\xa4";
const unsigned short utf16[] = {0xC720, 0xC800, 0xC778, 0xD130, 0xD398, 0xC774, 0xC2A4, 0};
const unsigned int unicode[] = {0xC720, 0xC800, 0xC778, 0xD130, 0xD398, 0xC774, 0xC2A4, 0};


static int num_tests = 0;
static int num_ok = 0;

const char* compare_cp949(char *mine) {
	num_tests++;
	if (mine == NULL)
		return("NULL");
	if (strcmp(cp949, mine) != 0) {
		free(mine);
		return("MISSMATCH");
	}
	free(mine);
	num_ok++;
	return("OK");
}


const char* compare_utf8(char *mine) {
	num_tests++;
	if (mine == NULL)
		return("NULL");
	if (strcmp(utf8, mine) != 0) {
		free(mine);
		return("MISSMATCH");
	}
	free(mine);
	num_ok++;
	return("OK");
}


const char* compare_utf16(unsigned short *mine) {
	unsigned int i;
	num_tests++;
	if (mine == NULL)
		return("NULL");
	for (i = 0; utf16[i] != 0 || mine[i] != 0; ++i) {
		if (utf16[i] != mine[i]) {
			free(mine);
			return("MISSMATCH");
		}
	}
	free(mine);
	num_ok++;
	return("OK");
}


const char* compare_unicode(unsigned int *mine) {
	unsigned int i;
	num_tests++;
	if (mine == NULL)
		return("NULL");
	for (i = 0; unicode[i] != 0 || mine[i] != 0; ++i) {
		if (unicode[i] != mine[i]) {
			free(mine);
			return("MISSMATCH");
		}
	}
	free(mine);
	num_ok++;
	return("OK");
}


int main()
{

	printf("CP949->UNICODE : %s\n", compare_unicode(roint_string_cp949_to_unicode(cp949)));
	printf("CP949->UTF16 : %s\n", compare_utf16(roint_string_cp949_to_utf16(cp949)));
	printf("CP949->UTF8 : %s\n", compare_utf8(roint_string_cp949_to_utf8(cp949)));

	printf("UTF8->UNICODE : %s\n", compare_unicode(roint_string_utf8_to_unicode(utf8)));
//	printf("UTF8->UTF16 : %s\n", compare_utf16(roint_string_utf8_to_utf16(utf8)));
	printf("UTF8->CP949 : %s\n", compare_cp949(roint_string_utf8_to_cp949(utf8)));

	printf("UTF16->UNICODE : %s\n", compare_unicode(roint_string_utf16_to_unicode(utf16)));
//	printf("UTF16->UTF8 : %s\n", compare_utf8(roint_string_utf16_to_utf8(utf16)));
	printf("UTF16->CP949 : %s\n", compare_cp949(roint_string_utf16_to_cp949(utf16)));

	printf("UNICODE->UTF16 : %s\n", compare_utf16(roint_string_unicode_to_utf16(unicode)));
	printf("UNICODE->UTF8 : %s\n", compare_utf8(roint_string_unicode_to_utf8(unicode)));
	printf("UNICODE->CP949 : %s\n", compare_cp949(roint_string_unicode_to_cp949(unicode)));

	if (num_tests != num_ok)
		return(EXIT_FAILURE);
	return(EXIT_SUCCESS);
}
