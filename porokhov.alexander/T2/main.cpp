#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <limits>
#include <iomanip>
#include <utility>

struct DelimeterIO
{
    char exp;
};

struct SllLitIO
{
    long long &ref;
};

struct RatLspIO
{
    std::pair<long long, unsigned long long> &ref;
};

struct StringIO
{
    std::string &ref;
};

std::istream &operator>>(std::istream &in, DelimeterIO &&dest)
{
    std::istream::sentry sentry(in);
    if (!sentry)
    {
        return in;
    }

    char c = '\0';
    in >> c;
    if (in && c != dest.exp)
    {
        in.setstate(std::ios::failbit);
    }

    return in;
}

std::istream &operator>>(std::istream &in, SllLitIO &&dest)
{
    std::istream::sentry sentry(in);
    if (!sentry)
    {
        return in;
    }

    long long value = 0;
    in >> value;

    if (in.peek() == 'L' || in.peek() == 'l')
    {
        char first = '\0';
        char second = '\0';
        in.get(first);
        in.get(second);
        if (first != second)
        {
            in.setstate(std::ios::failbit);
            return in;
        }
    }

    dest.ref = value;
    return in;
}

std::istream &operator>>(std::istream &in, RatLspIO &&dest)
{
    std::istream::sentry sentry(in);
    if (!sentry)
    {
        return in;
    }

    in >> DelimeterIO{'('};
    if (!in)
    {
        return in;
    }

    in >> DelimeterIO{':'};
    if (!in)
    {
        return in;
    }

    std::string label;
    in >> label;
    if (!in || label != "N")
    {
        in.setstate(std::ios::failbit);
        return in;
    }

    long long numer = 0;
    in >> numer;
    if (!in)
    {
        return in;
    }

    in >> DelimeterIO{':'};
    if (!in)
    {
        return in;
    }

    in >> label;
    if (!in || label != "D")
    {
        in.setstate(std::ios::failbit);
        return in;
    }

    unsigned long long denom = 0;
    in >> denom;
    if (!in || denom == 0)
    {
        in.setstate(std::ios::failbit);
        return in;
    }

    in >> DelimeterIO{':'};
    if (!in)
    {
        return in;
    }

    in >> DelimeterIO{')'};
    if (!in)
    {
        return in;
    }

    dest.ref = {numer, denom};
    return in;
}

std::istream &operator>>(std::istream &in, StringIO &&dest)
{
    std::istream::sentry sentry(in);
    if (!sentry)
    {
        return in;
    }

    in >> DelimeterIO{'"'};
    if (!in)
    {
        return in;
    }
    std::getline(in, dest.ref, '"');

    return in;
}

class iofguard
{
public:
    iofguard(std::basic_ios<char> &s) : s_(s),
                                        fill_(s.fill()),
                                        precision_(s.precision()),
                                        fmt_(s.flags())
    {
    }

    ~iofguard()
    {
        s_.fill(fill_);
        s_.precision(precision_);
        s_.flags(fmt_);
    }

private:
    std::basic_ios<char> &s_;
    char fill_;
    std::streamsize precision_;
    std::basic_ios<char>::fmtflags fmt_;
};

struct DataStruct
{
    long long key1;
    std::pair<long long, unsigned long long> key2;
    std::string key3;
};

struct DataStructComparator
{
    bool operator()(const DataStruct &left, const DataStruct &right) const
    {
        if (left.key1 != right.key1)
        {
            return left.key1 < right.key1;
        }

        if (left.key2.first != right.key2.first)
        {
            return left.key2.first < right.key2.first;
        }

        double leftVal = static_cast<double>(left.key2.first) / left.key2.second;
        double rightVal = static_cast<double>(right.key2.first) / right.key2.second;

        if (leftVal != rightVal)
        {
            return leftVal < rightVal;
        }

        return left.key3.length() < right.key3.length();
    }
};

std::istream &operator>>(std::istream &in, DataStruct &dataStruct)
{
    std::istream::sentry sentry(in);
    if (!sentry)
    {
        return in;
    }

    DataStruct temp;
    bool hasKey1 = false;
    bool hasKey2 = false;
    bool hasKey3 = false;

    in >> DelimeterIO{'('};
    if (!in)
    {
        return in;
    }

    while (!(hasKey1 && hasKey2 && hasKey3))
    {
        in >> DelimeterIO{':'};
        if (!in)
        {
            break;
        }

        std::string label;
        in >> label;
        if (!in)
        {
            break;
        }

        if (label == "key1" && !hasKey1)
        {
            in >> SllLitIO{temp.key1};
            if (!in)
            {
                break;
            }
            hasKey1 = true;
        }
        else if (label == "key2" && !hasKey2)
        {
            in >> RatLspIO{temp.key2};
            if (!in)
            {
                break;
            }
            hasKey2 = true;
        }
        else if (label == "key3" && !hasKey3)
        {
            in >> StringIO{temp.key3};
            if (!in)
            {
                break;
            }
            hasKey3 = true;
        }
        else
        {
            in.setstate(std::ios::failbit);
            break;
        }
    }

    in >> DelimeterIO{':'};
    if (!in)
    {
        return in;
    }

    in >> DelimeterIO{')'};
    if (!in)
    {
        return in;
    }

    if (hasKey1 && hasKey2 && hasKey3)
    {
        dataStruct = temp;
    }
    else
    {
        in.setstate(std::ios::failbit);
    }

    return in;
}

std::ostream &operator<<(std::ostream &out, const DataStruct &dataStruct)
{
    std::ostream::sentry sentry(out);
    if (!sentry)
    {
        return out;
    }

    iofguard guard(out);

    out << "(:";
    out << "key1 " << dataStruct.key1 << "ll:";
    out << "key2 (:N " << dataStruct.key2.first << ":D " << dataStruct.key2.second << ":):";
    out << "key3 \"" << dataStruct.key3 << "\":";
    out << ")";

    return out;
}

int main()
{
    std::vector<DataStruct> data;

    while (!std::cin.eof())
    {
        std::copy(std::istream_iterator<DataStruct>(std::cin),
                  std::istream_iterator<DataStruct>(),
                  std::back_inserter(data));

        if (!std::cin.eof() && std::cin.fail())
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    };

    std::sort(data.begin(), data.end(), DataStructComparator());

    std::copy(
        data.begin(),
        data.end(),
        std::ostream_iterator<DataStruct>(std::cout, "\n"));

    return 0;
}
