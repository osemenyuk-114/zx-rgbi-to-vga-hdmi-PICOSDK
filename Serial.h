#pragma once

#include <string>

using String = std::string;

#define DEC 10
#define HEX 16
#define OCT 8
#define BIN 2

class SerialIo
{
private:
    int lookahead = -1; // -1 means no buffered char
    size_t printNumber(unsigned long, uint8_t);
    size_t printULLNumber(unsigned long long, uint8_t);
    size_t printFloat(double, int);

public:
    void begin(unsigned long);
    bool available();
    int read();

    size_t write(const uint8_t *, size_t);
    size_t write(const char *, size_t);
    size_t write(const char *);
    size_t write(char);

    size_t print(const String &);
    size_t print(const char[]);
    size_t print(char);
    size_t print(unsigned char, int = DEC);
    size_t print(int, int = DEC);
    size_t print(unsigned int, int = DEC);
    size_t print(long, int = DEC);
    size_t print(unsigned long, int = DEC);
    size_t print(long long, int = DEC);
    size_t print(unsigned long long, int = DEC);
    size_t print(double, int = 2);

    size_t println(const String &s);
    size_t println(const char[]);
    size_t println(char);
    size_t println(unsigned char, int = DEC);
    size_t println(int, int = DEC);
    size_t println(unsigned int, int = DEC);
    size_t println(long, int = DEC);
    size_t println(unsigned long, int = DEC);
    size_t println(long long, int = DEC);
    size_t println(unsigned long long, int = DEC);
    size_t println(double, int = 2);
    size_t println(void);
};