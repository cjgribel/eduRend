//
//  parseutil.h
//	Auxiliary parsing & string handling stuff
//
//  Carl Johan Gribel 2016, cjgribel@gmail.com
//

#pragma once
#ifndef parseutil_h
#define parseutil_h

#include <string>
#include <vector>

inline std::string& rtrim(std::string& str)
{
    str.erase(str.find_last_not_of(" \n\r\t")+1);
    return str;
}

inline std::string& ltrim(std::string& str)
{
    str.erase(0, str.find_first_not_of(" \n\r\t"));
    return str;
}

inline std::string& lrtrim(std::string& str)
{
    return ltrim(rtrim(str));
}

static std::string get_parentdir(std::string path)
{
    std::string parent;
    size_t pos = path.rfind("/");
    
    if (pos != std::string::npos)
        parent = path.substr(0, pos+1);
    
    return parent;
}

//
// find and extract first occurance of *.[suffix] in a string
//
static bool find_filename_from_suffix(const std::string &str, const std::string &suffix, std::string& res)
{
    std::string dotsuffix = std::string(".")+suffix;
    size_t end = str.find(dotsuffix);

    if (end == std::string::npos)
        return false;
    
    size_t start = str.rfind(" ", end);
    
    if (start == end)
        return false;
    if (start == std::string::npos)
        start = -1;
    
    res = str.substr(start+1, end-start-1 + dotsuffix.length());
    return true;
}

//
// find and extract first occurance of *.[{suffix-list}] in a string
//
static bool find_filename_from_suffixes(const std::string &str, const std::vector<std::string>& suffixes, std::string& res)
{
	for (auto& suffix : suffixes)
		if (find_filename_from_suffix(str, suffix, res))
			return true;
	return false;
}

#endif /* parseutil_h */
