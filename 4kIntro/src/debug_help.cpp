#include "debug_help.h"
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>


/* Yes, this leaks memory. It's also a demo, so it doesn't matter. */
char*  loadShader(const char* path) {
	FILE *fragFile;
	fragFile = fopen(path, "r");
	long length;
	char * buffer = 0;

	if (fragFile)
	{
		fseek(fragFile, 0, SEEK_END);
		length = ftell(fragFile);
		fseek(fragFile, 0, SEEK_SET);
		buffer = (char*)malloc(length);
		if (buffer)
		{
			fread(buffer, 1, length, fragFile);
		}
		fclose(fragFile);
	}

	if (buffer)
	{
		return buffer;
	}
	else
	{
		MessageBox(0, path, "Error: couldn't load shader", MB_OK | MB_ICONERROR);
		return nullptr;
	}
}