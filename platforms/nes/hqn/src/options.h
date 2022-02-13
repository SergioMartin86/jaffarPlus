#ifndef __OPTIONS_H__
#define __OPTIONS_H__

#include <string>
#include <map>

/**
 * Compare two strings case-insensitively.
 */
int string_icompare(const std::string& a, const std::string &b);

/**
 * Holds options provides an interface to load them
 * and save them.
 */
class Options
{
public:
    Options();
    ~Options();

    /**
     * Tries to load options from the filename.
     * If the file cannot be opened returns -1,
     * if there is an error returns the line the error was on (starting at 1).
     * Otherwise returns 0.
     */
    int load(const std::string &filename);

    /**
     * Save the options to the named file.
     * If all goes well returns 0 otherwise returns an error code.
     * 
     * Errors:
     *   -1 = failed to open file
     */
    int save(const std::string &filename);

    /**
     * Get a named integer or return the default value.
     */
    int getInt(const std::string &name, int def=0);

    /**
     * Get the option as a double.
     */
    double getNum(const std::string &name, double def=0.0);

    /**
     * Get the named option as a boolean or use the default value.
     */
    bool getBool(const std::string &name, bool def=false);

    /**
     * Get the given option as a string, or return the default value.
     * @param name the name of the option to get
     * @param def default value if name is not found
     */
    const std::string &getString(const std::string &name, const std::string &def="");

    /**
     * Check if the option set has the given option.
     */
    bool has(const std::string &name);

    /**
     * Set a value in the options.
     */
    void set(const std::string &name, const std::string &value);
    void set(const std::string &name, double value);
    void set(const std::string &name, bool value);

private:
    std::map<std::string, std::string> m_data;
};

#endif // __OPTIONS_H__