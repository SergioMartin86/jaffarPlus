
#include "hqn_util.h"
#include <cctype>
#include <cstdio>
#include <sys/stat.h>

namespace hqn
{


bool fileSize(const char *filename, size_t *size)
{
    struct stat s;
    if (stat(filename, &s) == 0)
    {
        *size = s.st_size;
        return true;
    }
    else
    {
        *size = 0;
        return false;
    }
}
    
bool load_file(const char *filename, char **data, size_t *size)
{
    char *dataRef = nullptr;
    char *dataInsert;
    size_t dataSize;
    size_t readAmount;
    FILE *fd;

    if (!fileSize(filename, &dataSize))
    { goto read_failed; }

    dataRef = new char[dataSize];
    dataInsert = dataRef;
    readAmount = 0; // how many bytes we read

    fd = fopen(filename, "rb");
    if (!fd)
    { goto read_failed; }

    do
    {
        readAmount = fread(dataInsert, 1, dataSize - (dataInsert - dataRef), fd);
        dataInsert += readAmount;
    } while (readAmount != 0);
    fclose(fd);
    
    *data = dataRef;
    *size = dataSize;
    return true;

// Jump here if we failed to open the file.
read_failed:
    if (dataRef)
    { delete[] dataRef; }
    *data = nullptr;
    *size = 0;
    return false;
}

bool save_file(const char *filename, const char *data, size_t size)
{
    FILE *fd = fopen(filename, "wb");
    if (!fd)
    { return false; }
    if (fwrite(data, 1, size, fd) != size)
    {
        fclose(fd);
        return false;
    }
    fclose(fd);
    return true;
}

bool file_exists(const char *filename)
{
    struct stat s;
    return stat(filename, &s) == 0;
}


int stricmp(char const *a, char const *b)
{
    for (;; a++, b++) {
        int d = tolower(*a) - tolower(*b);
        if (d != 0 || !*a)
            return d;
    }
}

}
