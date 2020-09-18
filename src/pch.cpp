#include "pch.h"

char executable_path[MAX_PATH_LENGTH] = "";
char executable_directory[MAX_PATH_LENGTH] = "";

const char* get_executable_path() {
    return executable_path;
}
void ___set_executable_path(const char* path) {
    strcpy(executable_path, path);
    Path::directory_of(executable_path, executable_directory);
}

const char* get_executable_directory() {
    return executable_directory;
}