#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <list>


using namespace std;

wstring g_final_result;


// returns 1 if file can be opened
int check_path_ext(const wchar_t *exe_filename) {
	//fwprintf(stdout, L"trying %s...\n", exe_filename);
	FILE *f = _wfopen(exe_filename, L"rb");
	if (f) {
		fclose(f);
		g_final_result.assign( exe_filename );
		return 1;
	}
	return 0;
}


int check_path(const wchar_t *exe, const wstring& path_entry, const list<wstring>& PATHEXT) {
	for(list<wstring>::const_iterator iter = PATHEXT.begin(); iter != PATHEXT.end(); iter++ ) {
		wstring path_ext = *iter;
		wstring ws_exe;
		// construct exe filename
		ws_exe.assign(path_entry);
		ws_exe.append(L"\\");
		ws_exe.append(exe);
		ws_exe.append(path_ext);
		if (check_path_ext(ws_exe.c_str())) {
			return 1;
		}
	}
	return 0;
}


int wmain(int argc, wchar_t **argv) {
	if(argc < 2) {
		fprintf(stderr, "Incorrect arguments!\n" );
		fprintf(stderr, "Usage: which <program>\n" );
		return EXIT_FAILURE;
	}
	if (argv[1] == NULL) {
		return EXIT_FAILURE;
	}
	if (argv[1][0] == 0) {
		return EXIT_FAILURE;
	}

	std::list<std::wstring> PATH;
	std::list<std::wstring> PATHEXT;
	wchar_t *buffer = NULL;
	size_t bufsize = 16*1024; // 16kb
	size_t ret = 0;
	wchar_t delim[] = L";";
	wchar_t *token = NULL;
	int found = 0;

	buffer = (wchar_t *)malloc(bufsize);
	if (!buffer) {
		fprintf(stderr, "Failed to allocate memory bu buffer!\n");
		return EXIT_FAILURE;
	}

	// get PATH
	memset(buffer, 0, bufsize);
	ret = (size_t)GetEnvironmentVariableW(L"PATH", buffer, bufsize/sizeof(buffer[0]));
	if ((ret > 0) && (ret < bufsize)) {
#ifdef _DEBUG
		fwprintf(stdout, L"PATH = %s\n", buffer);
#endif
	} else {
		free(buffer);
		fprintf(stderr, "GetEnvironmentVariableW(PATH) Failed: %d!\n", (int)GetLastError());
		return EXIT_FAILURE;
	}

	// split PATH
	token = wcstok(buffer, delim);
	while( token ) {
		std::wstring path_entry(token);
		PATH.push_back(path_entry);
		token = wcstok(NULL, delim);
	}

	// get PATHEXT
	memset(buffer, 0, bufsize);
	ret = (size_t)GetEnvironmentVariableW(L"PATHEXT", buffer, bufsize/sizeof(buffer[0]));
	if ((ret > 0) && (ret < bufsize)) {
#ifdef _DEBUG
		fwprintf(stdout, L"PATHEXT = %s\n", buffer);
#endif
	} else {
		free(buffer);
		fprintf(stderr, "GetEnvironmentVariableW(PATH) Failed: %d!\n", (int)GetLastError());
		return EXIT_FAILURE;
	}

	// split PATHEXT
	token = wcstok(buffer, delim);
	while( token ) {
		std::wstring pathext_entry(token);
		PATHEXT.push_back(pathext_entry);
		token = wcstok(NULL, delim);
	}
	// cleanup, we don't need buffer any more
	free(buffer);
	buffer = NULL;

	// go
	for(std::list<std::wstring>::const_iterator iter = PATH.begin(); iter != PATH.end(); iter++ ) {
		std::wstring path_entry = *iter;
		found = check_path(argv[1], path_entry, PATHEXT);
		if (found != 0) {
			break;
		}
	}

	if (found) {
		fwprintf(stdout, L"%s\n", g_final_result.c_str());
	} else {
		fwprintf(stderr, L"No '%s' in:\n", argv[1]);
		for(std::list<std::wstring>::const_iterator iter = PATH.begin(); iter != PATH.end(); iter++ ) {
			std::wstring path_entry = *iter;
			fwprintf(stderr, L"    %s\n", path_entry.c_str());
		}
	}

	if (found)
		return EXIT_SUCCESS;
	return EXIT_FAILURE;
}
