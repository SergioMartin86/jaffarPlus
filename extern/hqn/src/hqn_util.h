#ifndef __HQN_UTIL_H__
#define __HQN_UTIL_H__

#include <cstdint>
#include <cstdlib>

namespace hqn
{

/*
Get the size of the given file in bytes and store it in size.
Returns false if the file could not be read.
*/
bool fileSize(const char *filename, size_t *size);

/*
Load the named file into ram. Allocates a buffer using new[].
The caller is responsible for calling delete[] on the buffer.
*/
bool load_file(const char *filename, char **data, size_t *size);

/*
Save the data into the named file.
*/
bool save_file(const char *filename, const char *data, size_t size);

/*
Return true if the file exists and false otherwise.
*/
bool file_exists(const char *filename);


/* Compare ascii strings in a case-insensitive manner. */
int stricmp(char const *a, char const *b);

}

#endif /* __HQN_UTIL_H__ */
