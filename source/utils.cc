#include "utils.h"
#include <dirent.h>
#include <stdarg.h>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

std::vector<std::string> split(const std::string &s, char delim)
{
 std::string newString = s;
 std::replace(newString.begin(), newString.end(), '\n', ' ');
 std::vector<std::string> elems;
 split(newString, delim, std::back_inserter(elems));
 return elems;
}

void exitWithError [[noreturn]] (const char *fileName, const int lineNumber, const char *format, ...)
{
  char *outstr = 0;
  va_list ap;
  va_start(ap, format);
  int ret = vasprintf(&outstr, format, ap);
  if (ret < 0) exit(-1);

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

// Taken from https://stackoverflow.com/questions/116038/how-do-i-read-an-entire-file-into-a-stdstring-in-c/116220#116220
std::string slurp(std::ifstream &in)
{
  std::ostringstream sstr;
  sstr << in.rdbuf();
  return sstr.str();
}

