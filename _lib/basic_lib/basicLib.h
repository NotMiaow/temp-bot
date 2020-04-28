#ifndef BASIC_LIB_H__
#define BASIC_LIB_H__

#include <iostream>
#include <string>
#include <sstream>


static bool ToInt(std::string s, int& n)
{
	const std::string ws(" \t\f\v\n\r");
	size_t pos = s.find_last_not_of(ws);

	if (pos != std::string::npos)
        s.erase(pos + 1);
    else s.clear();

    std::stringstream buffer(s);
    return buffer >> n && buffer.eof();
}

static int RoundToInt(float number)
{
	float temp = (float)(int)number;
	return (number - temp) > 0.5f ? (int)temp + 1 : (int)temp;
}

#endif
