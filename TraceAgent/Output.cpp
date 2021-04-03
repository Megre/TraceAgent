#include "stdafx.h"
#include "Output.h"
#include <iostream>

void TabBuf(char **buf, size_t bufSize, int size)
{
	memset(*buf, ' ', min(bufSize, size));
	(*buf) += size;
}

void FillBuf(char **buf, size_t bufSize, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	int len = vsprintf_s(*buf, bufSize, format, args);
	va_end(args);

	(*buf) += len;
}
