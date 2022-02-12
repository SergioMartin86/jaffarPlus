
#include "options.h"
#include <cstdio>
#include <cctype>

#define MAX_LINE 2048

// Remove whitespace around a string
std::string trim(const std::string &in)
{
	size_t first = in.find_first_not_of(" ");
	size_t last = in.find_last_not_of(" ");
	if (first == std::string::npos || last == std::string::npos)
	{
		return in;
	}
	else
	{
		return in.substr(first, last - first + 1);
	}
}

Options::Options()
{}

Options::~Options()
{}

int Options::load(const std::string &filename)
{
    FILE *fd;
    char line[MAX_LINE];
    int lineno = 1;

    fd = fopen(filename.c_str(), "r");
    if (!fd)
    {
        return -1;
    }
    while (fgets(line, MAX_LINE, fd) != nullptr)
    {
        // split line at the = sign
		std::string ln = trim(line);
		size_t eqPos = ln.find("=");
		if (eqPos == std::string::npos)
		{
			// clean up and exit
			fclose(fd);
			return lineno;
		}
		else
		{
			std::string key = trim(ln.substr(0, eqPos - 1));
			std::string value = trim(ln.substr(eqPos + 1));
			m_data[key] = value;
		}
        lineno++;
    }
    fclose(fd);
    return 0;
}

int Options::save(const std::string &filename)
{
    FILE *fd;
    fd = fopen(filename.c_str(), "w");
    if (!fd)
    {
        return -1;
    }
    for (auto it : m_data)
    {
        fprintf(fd, "%s = %s\n", it.first.c_str(), it.second.c_str());
    }
    fclose(fd);
    return 0;
}

int Options::getInt(const std::string &name, int def)
{
    int result;
    if (!has(name))
        return def;
    if (sscanf(m_data[name].c_str(), "%d", &result) != 1)
        result = def;
    return result;
}

double Options::getNum(const std::string &name, double def)
{
    double result;
    if (!has(name))
        return def;
    if (sscanf(m_data[name].c_str(), "%f", &result) != 1)
        result = def;
    return result;
}

bool Options::getBool(const std::string &name, bool def)
{
    if (!has(name))
        return def;
    // test for true/false strings
    const std::string &value = m_data[name];
    if (string_icompare(value, "true") == 0)
        return true;
    if (string_icompare(value, "false") == 0)
        return false;
    // try to convert to an int and use that as a bool
    size_t pos;
    int asInt = std::stoi(value, &pos);
    if (pos)
        return (bool)asInt;
    // fall back to default
    return def;
}

const std::string &Options::getString(const std::string &name, const std::string &def)
{
    if (!has(name))
        return def;
    return m_data[name];
}

void Options::set(const std::string &name, const std::string &value)
{
    m_data[name] = value;
}

void Options::set(const std::string &name, double value)
{
    m_data[name] = std::to_string(value);
}

void Options::set(const std::string &name, bool value)
{
    m_data[name] = value ? "true" : "false";
}

bool Options::has(const std::string &name)
{
    return m_data.find(name) != m_data.end();
}

int string_icompare(const std::string &a, const std::string &b)
{
    auto itA = a.begin();
    auto itB = b.begin();
    while (itA != a.end() && itB != b.end())
    {
        int diff = toupper(*itA) - toupper(*itB);
        if (diff)
            return diff;
        itA++;
        itB++;
    }
    return 0;
}
