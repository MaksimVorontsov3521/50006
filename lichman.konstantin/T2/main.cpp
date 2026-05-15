#include <vector>
#include <string>
#include <algorithm>
#include <iterator>
#include <iomanip>
#include <iostream>
#include <cctype>

struct DataStruct {
    double key1;
    unsigned long long key2;
    std::string key3;
};

void skipSpaces(std::istream& in)
{
    while (in && std::isspace(in.peek()))
        in.get();
}

bool readDblit(std::istream& in, double& val)
{
    std::string s;
    skipSpaces(in);

    if (in.peek() == '-')
        s += static_cast<char>(in.get());

    if (!std::isdigit(in.peek()))
        return false;

    while (std::isdigit(in.peek()))
        s += static_cast<char>(in.get());

    if (in.peek() != '.')
        return false;
    s += static_cast<char>(in.get());

    if (!std::isdigit(in.peek()))
        return false;

    while (std::isdigit(in.peek()))
        s += static_cast<char>(in.get());

    char suffix = static_cast<char>(in.get());
    if (suffix != 'd' && suffix != 'D')
        return false;

    val = std::stod(s);
    return true;
}

bool readUllHex(std::istream& in, unsigned long long& val)
{
    std::string s;
    skipSpaces(in);

    char c0 = in.get();
    char c1 = in.get();

    if (c0 != '0' || (c1 != 'x' && c1 != 'X'))
        return false;

    while (std::isxdigit(in.peek()))
        s += static_cast<char>(in.get());

    if (s.empty()) return false;
    val = std::stoull(s, nullptr, 16);

    return true;
}

bool readString(std::istream& in, std::string& val)
{
    skipSpaces(in);

    char c0 = in.get();
    if (c0 != '"')
        return false;

    while (in && in.peek() != '"')
        val += static_cast<char>(in.get());

    in.get();
    return true;
}

std::istream& operator>>(std::istream& in, DataStruct& ds)
{
    skipSpaces(in);
    if (in.peek() == EOF) return in;

    DataStruct tmp{};
    bool gotKey1 = false, gotKey2 = false, gotKey3 = false;

    char c0 = in.get();
    char c1 = in.get();
    if (c0 != '(' || c1 != ':') {
        in.setstate(std::ios::failbit);
        return in;
    }

    for (int i = 0; i < 3; i++) {
        std::string keyName;
        while (in && in.peek() != ' ' && in.peek() != ':')
            keyName += static_cast<char>(in.get());

        skipSpaces(in);

        if (keyName == "key1") {
            if (!readDblit(in, tmp.key1)) {
                in.setstate(std::ios::failbit);
                return in;
            }
            gotKey1 = true;
        }
        else if (keyName == "key2") {
            if (!readUllHex(in, tmp.key2)) {
                in.setstate(std::ios::failbit);
                return in;
            }
            gotKey2 = true;
        }
        else if (keyName == "key3") {
            if (!readString(in, tmp.key3)) {
                in.setstate(std::ios::failbit);
                return in;
            }
            gotKey3 = true;
        }
        else {
            in.setstate(std::ios::failbit);
            return in;
        }

        skipSpaces(in);
        if (in.get() != ':') {
            in.setstate(std::ios::failbit);
            return in;
        }
    }

    if (in.get() != ')') {
        in.setstate(std::ios::failbit);
        return in;
    }

    if (!gotKey1 || !gotKey2 || !gotKey3) {
        in.setstate(std::ios::failbit);
        return in;
    }

    ds = tmp;
    return in;
}

std::ostream& operator<<(std::ostream& out, const DataStruct& ds)
{
    out << "(:key1 " << std::fixed << std::setprecision(1) << ds.key1 << "d";
    out << ":key2 0x" << std::uppercase << std::hex << ds.key2 << std::dec;
    out << ":key3 \"" << ds.key3 << "\":)";
    return out;
}

int main()
{
    std::vector<DataStruct> data;

    while (std::cin)
    {
        std::copy(
            std::istream_iterator<DataStruct>(std::cin),
            std::istream_iterator<DataStruct>(),
            std::back_inserter(data)
        );

        if (std::cin.fail() && !std::cin.eof())
        {
            std::cin.clear();
            std::string badLine;
            std::getline(std::cin, badLine);
        }
    }

    std::sort(data.begin(), data.end(),
        [](const DataStruct& a, const DataStruct& b) {
            if (a.key1 != b.key1)
                return a.key1 < b.key1;
            if (a.key2 != b.key2)
                return a.key2 < b.key2;
            return a.key3.size() < b.key3.size();
        });

    std::copy(
        data.begin(),
        data.end(),
        std::ostream_iterator<DataStruct>(std::cout, "\n")
    );

    return 0;
}
