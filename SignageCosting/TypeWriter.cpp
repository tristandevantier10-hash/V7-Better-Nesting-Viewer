#include "TypeWriter.h"
#include <iostream>
#include <thread>
#include <chrono>

void TypeWriter::print(const std::string& text, int delayMs)
{
    for (char c : text)
    {
        std::cout << c << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
    }
}

void TypeWriter::println(const std::string& text, int delayMs)
{
    print(text + "\n", delayMs);
}