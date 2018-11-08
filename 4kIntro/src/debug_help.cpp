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
		buffer = (char*)malloc(length+1);
		if (buffer)
		{
			// fread has this nifty little feature where it doesn't bother with CR chars,
			// meaning that it reads less than length. Therefore, we grab how much it actually
			// read and set a \0 to terminate it.
			int actual = fread(buffer, 1, length, fragFile);
			buffer[actual] = 0;

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