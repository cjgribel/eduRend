/**
 * @file parseutil.h
 * @author Carl Johan Gribel 2016, cjgribel@gmail.com
 * @brief Auxiliary parsing & string handling stuff
*/

#pragma once
#ifndef parseutil_h
#define parseutil_h

#include <string>
#include <vector>

/**
 * @brief Trims whitespace from the start of a string.
 * @param[in, out] str String to trim.
 * @return A reference to the input string.
*/
inline std::string& rtrim(std::string& str)
{
    str.erase(str.find_last_not_of(" \n\r\t")+1);
    return str;
}

/**
 * @brief Trims whitespace from the end of a string.
 * @param[in, out] str String to trim.
 * @return A reference to the input string.
*/
inline std::string& ltrim(std::string& str)
{
    str.erase(0, str.find_first_not_of(" \n\r\t"));
    return str;
}

/**
 * @brief Trims whitespace from both the start and end of a string.
 * @details Calls #ltrim(std::string&) and #rtrim(std::string&) internally.
 * @param[in, out] str String to trim.
 * @return A reference to the input string.
*/
inline std::string& lrtrim(std::string& str)
{
    return ltrim(rtrim(str));
}

/**
 * @brief Find the parent folder of the path.
 * @details Looks for the last "/" in the given path and returns everything up to that point.
 * @param[in] path String represnting a file path.
 * @return A new string containing just the path, or an empty string if no valid parent folder was found.
*/
static std::string get_parentdir(const std::string& path)
{
    std::string parent;
    size_t pos = path.rfind("/");
    
    if (pos != std::string::npos)
        parent = path.substr(0, pos+1);
    
    return parent;
}

/**
 * @brief Find and extract first occurance of *.[suffix] in a string.
 * @param[in] str The string to search.
 * @param[in] suffix Suffix to search for.
 * @param[out] res Result of the search.
 * @return True if a result was found, False if no occuernce was found.
*/
static bool find_filename_from_suffix(const std::string& str, const std::string& suffix, std::string& res)
{
    std::string dotsuffix = std::string(".")+suffix;
    size_t end = str.find(dotsuffix);

    if (end == std::string::npos)
        return false;
    
    size_t start = str.rfind(" ", end);
    
    if (start == end)
        return false;
    if (start == std::string::npos)
        start = ~(size_t)0;

    res = str.substr(start+1, end-start-1 + dotsuffix.length());
    return true;
}

/**
 * @brief Find and extract first occurance of *.[{suffix-list}] in a string.
 * @details Calls find_filename_from_suffix(const std::string&, const std::string&, std::string&) for each suffix in the list untill one of them returns true.
 * @param[in] str The string to search.
 * @param[in] suffixes List of suffixes to search for.
 * @param[out] res Result of the search.
 * @return True if a result was found, False if no occuernce was found.
*/
static bool find_filename_from_suffixes(const std::string& str, const std::vector<std::string>& suffixes, std::string& res)
{
	for (auto& suffix : suffixes)
		if (find_filename_from_suffix(str, suffix, res))
			return true;
	return false;
}

#endif /* parseutil_h */
