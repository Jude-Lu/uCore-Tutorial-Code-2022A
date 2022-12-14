#include "modules.h"

int procid()
{
	return 0;
}

int threadid()
{
	return 0;
}

void clean_bss()
{
	char *p;
	for (p = s_bss; p < e_bss; ++p)
		*p = 0;
}

void main()
{
	clean_bss();
	console_init();
	printf("\n");
	printf("hello wrold!\n");
	errorf("stext: %p", s_text);
	warnf("etext: %p", e_text);
	infof("sroda: %p", s_rodata);
	debugf("eroda: %p", e_rodata);
	debugf("sdata: %p", s_data);
	infof("edata: %p", e_data);
	warnf("sbss : %p", s_bss);
	errorf("ebss : %p", e_bss);
	panic("ALL DONE");
}
