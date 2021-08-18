#include "utils.h"
#include <dirent.h>
#include <stdarg.h>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

std::vector<std::string> split(const std::string &s, char delim)
{
  std::vector<std::string> elems;
  split(s, delim, std::back_inserter(elems));
  return elems;
}

void exitWithError [[noreturn]] (const char *fileName, const int lineNumber, const char *format, ...)
{
  char *outstr = 0;
  va_list ap;
  va_start(ap, format);
  vasprintf(&outstr, format, ap);

  std::string outString = outstr;
  free(outstr);

  char info[1024];

  snprintf(info, sizeof(info) - 1, " + From %s:%d\n", fileName, lineNumber);
  outString += info;

  throw std::runtime_error(outString.c_str());
}

bool dirExists(const std::string dirPath)
{
  DIR *dir = opendir(dirPath.c_str());
  if (dir)
  {
    closedir(dir);
    return true;
  }

  return false;
}

bool loadStringFromFile(std::string &dst, const char *fileName)
{
  std::ifstream fi(fileName);

  // If file not found or open, return false
  if (fi.good() == false) return false;

  // Reading entire file
  dst = slurp(fi);

  // Closing file
  fi.close();

  return true;
}

// Save string to a file
bool saveStringToFile(const std::string &src, const char *fileName)
{
  FILE *fid = fopen(fileName, "w");
  if (fid != NULL)
  {
    fwrite(src.c_str(), 1, src.size(), fid);
    fclose(fid);
    return true;
  }
  return false;
}

// Save Bin to a file
bool saveBinToFile(const uint8_t* src, size_t size, const char *fileName)
{
  FILE *fid = fopen(fileName, "w");
  if (fid != NULL)
  {
    fwrite(src, 1, size, fid);
    fclose(fid);
    return true;
  }
  return false;
}

// load bin from file
bool loadBinFromFile(uint8_t* dst, size_t size, const char *fileName)
{
  FILE *fid = fopen(fileName, "r");
  if (fid != NULL)
  {
    size_t readCount = fread(dst, 1, size, fid);
    fclose(fid);
    if (readCount != size) return false;
    return true;
  }
  return false;
}

// Taken from https://stackoverflow.com/questions/116038/how-do-i-read-an-entire-file-into-a-stdstring-in-c/116220#116220
std::string slurp(std::ifstream &in)
{
  std::ostringstream sstr;
  sstr << in.rdbuf();
  return sstr.str();
}

// From blastem util.c
char * path_dirname(const char *path)
{
 const char *lastslash = NULL;
 const char *cur;
 for (cur = path; *cur; cur++)
 {
  if (is_path_sep(*cur)) {
   lastslash = cur;
  }
 }
 if (!lastslash) {
  return NULL;
 }
 char *dir = (char*)malloc(lastslash-path+1);
 memcpy(dir, path, lastslash-path);
 dir[lastslash-path] = 0;

 return dir;
}

char * basename_no_extension(const char *path)
{
 const char *lastdot = NULL;
 const char *lastslash = NULL;
 const char *cur;
 for (cur = path; *cur; cur++)
 {
  if (*cur == '.') {
   lastdot = cur;
  } else if (is_path_sep(*cur)) {
   lastslash = cur + 1;
  }
 }
 if (!lastdot) {
  lastdot = cur;
 }
 if (!lastslash) {
  lastslash = path;
 }
 char *barename = (char*)malloc(lastdot-lastslash+1);
 memcpy(barename, lastslash, lastdot-lastslash);
 barename[lastdot-lastslash] = 0;

 return barename;
}

char *path_extension(char const *path)
{
 char const *lastdot = NULL;
 char const *lastslash = NULL;
 char const *cur;
 for (cur = path; *cur; cur++)
 {
  if (*cur == '.') {
   lastdot = cur;
  } else if (is_path_sep(*cur)) {
   lastslash = cur + 1;
  }
 }
 if (!lastdot || (lastslash && lastslash > lastdot)) {
  //no extension
  return NULL;
 }
 return strdup(lastdot+1);
}

char is_path_sep(char c)
{
#ifdef _WIN32
 if (c == '\\') {
  return 1;
 }
#endif
 return c == '/';
}
