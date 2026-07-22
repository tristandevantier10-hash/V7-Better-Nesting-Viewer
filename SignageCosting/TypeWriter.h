#pragma once
#include <string>

class TypeWriter
{
public:
    static void print(const std::string& text, int delayMs = -1);
    static void println(const std::string& text, int delayMs = -1);
};
