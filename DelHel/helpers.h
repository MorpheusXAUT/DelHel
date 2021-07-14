#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <WinInet.h>

#include "constants.h"

extern "C" IMAGE_DOS_HEADER __ImageBase;

std::string FetchLatestVersion();

class delhelexception : public std::exception
{
public:
	explicit delhelexception(std::string& what) : std::exception{ what.c_str() } {}
	virtual inline const long icon() const = 0;
	inline void whatMessageBox()
	{
		MessageBox(NULL, this->what(), PLUGIN_NAME, MB_OK | icon());
	}
};

class error : public delhelexception
{
public:
	explicit error(std::string& what) : delhelexception{ what } {}
	inline const long icon() const
	{
		return MB_ICONERROR;
	}
};

class warning : public delhelexception
{
public:
	explicit warning(std::string& what) : delhelexception{ what } {}
	inline const long icon() const
	{
		return MB_ICONWARNING;
	}
};

class information : public delhelexception
{
public:
	explicit information(std::string& what) : delhelexception{ what } {}
	inline const long icon() const
	{
		return MB_ICONINFORMATION;
	}
};

inline std::string GetPluginDirectory()
{
	char buf[MAX_PATH] = { 0 };
	GetModuleFileName(HINSTANCE(&__ImageBase), buf, MAX_PATH);

	std::string::size_type pos = std::string(buf).find_last_of("\\/");

	return std::string(buf).substr(0, pos);
}

inline std::vector<std::string> split(const std::string& s, char delim = ' ')
{
	std::istringstream ss(s);
	std::string item;
	std::vector<std::string> res;

	while (std::getline(ss, item, delim)) {
		res.push_back(item);
	}

	return res;
}

inline std::string join(const std::vector<std::string>& s, const char delim = ' ')
{
	std::ostringstream ss;
	std::copy(s.begin(), s.end(), std::ostream_iterator<std::string>(ss, &delim));
	return ss.str();
}

inline bool starts_with(const std::string& str, const std::string& pre)
{
	return str.rfind(pre, 0) == 0;
}

inline void to_upper(std::string& str)
{
	std::transform(str.begin(), str.end(), str.begin(),
		[](unsigned char c) -> unsigned char { return std::toupper(c); });
}

inline int round_to_closest(int num, int closest)
{
	return ((num + closest / 2) / closest) * closest;
}

inline std::string trim(const std::string& str, const std::string& charset = " \t")
{
	size_t begin = str.find_first_not_of(charset);
	if (begin == std::string::npos) {
		// Empty string or only characters from charset provided
		return "";
	}

	size_t end = str.find_last_not_of(charset);

	return str.substr(begin, end - begin + 1);
}