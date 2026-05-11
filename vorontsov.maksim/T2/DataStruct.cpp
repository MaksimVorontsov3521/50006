#include<ios>
#include<cctype>
#include <iomanip>

#include "DataStruct.h"
#include "StreamGuard.h"

std::istream& operator>>(std::istream& stream, DataStruct& data) {
    std::istream::sentry sentry(stream);
    if (!sentry) {
        return stream;
    }

    StreamGuard guard(stream);
    char ch{};

    if (!stream.get(ch) || ch != '(') {
        stream.setstate(std::ios::failbit);
        return stream;
    }
    if (!stream.get(ch) || ch != ':') {
        stream.setstate(std::ios::failbit);
        return stream;
    }

    bool hasKey1 = false;
    bool hasKey2 = false;
    bool hasKey3 = false;

    while (true) {
        const int nextChar = stream.peek();

        if (nextChar == ')') {
            stream.get(ch);
            break;
        }

        if (nextChar == std::char_traits<char>::eof() ||
            std::isspace(static_cast<unsigned char>(nextChar))) {
            stream.setstate(std::ios::failbit);
            return stream;
        }

        std::string fieldName{};
        stream >> fieldName;
        if (stream.fail() || fieldName.empty()) {
            stream.setstate(std::ios::failbit);
            return stream;
        }

        if (!stream.get(ch) || ch != ' ') {
            stream.setstate(std::ios::failbit);
            return stream;
        }

        if (std::isspace(static_cast<unsigned char>(stream.peek()))) {
            stream.setstate(std::ios::failbit);
            return stream;
        }

        if (fieldName == "key1") {
            unsigned long long value{};
            stream >> std::oct >> value;
            if (stream.fail()) {
                return stream;
            }
            data.key1_ = value;
            hasKey1 = true;
        }
        else if (fieldName == "key2") {
            char hash{}, cChar{}, openParen{};
            if (!stream.get(hash) || !stream.get(cChar) || !stream.get(openParen) ||
                hash != '#' || cChar != 'c' || openParen != '(') {
                stream.setstate(std::ios::failbit);
                return stream;
            }
            double real{}, imag{};
            stream >> real >> imag;
            if (stream.fail()) {
                return stream;
            }
            char closeParen{};
            stream >> closeParen;
            if (stream.fail() || closeParen != ')') {
                stream.setstate(std::ios::failbit);
                return stream;
            }
            data.key2_ = std::complex<double>(real, imag);
            hasKey2 = true;
        }
        else if (fieldName == "key3") {
            char quote{};
            if (!stream.get(quote) || quote != '"') {
                stream.setstate(std::ios::failbit);
                return stream;
            }
            std::getline(stream, data.key3_, '"');
            if (stream.fail()) {
                return stream;
            }
            hasKey3 = true;
        }
        else {
            stream.setstate(std::ios::failbit);
            return stream;
        }

        if (std::isspace(static_cast<unsigned char>(stream.peek())) ||
            stream.peek() == std::char_traits<char>::eof()) {
            stream.setstate(std::ios::failbit);
            return stream;
        }
        stream.get(ch);
        if (ch != ':') {
            stream.setstate(std::ios::failbit);
            return stream;
        }
    }

    if (!hasKey1 || !hasKey2 || !hasKey3) {
        stream.setstate(std::ios::failbit);
    }
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const DataStruct& data) {
    std::ostream::sentry sentry(stream);
    if (sentry) {
        StreamGuard guard(stream);
        stream << "(:key1 ";
        if (data.key1_ == 0) {
            stream << "00";
        }
        else {
            stream << std::showbase << std::oct << data.key1_;
        }
        stream << ":key2 #c(" << std::fixed << std::setprecision(1) << data.key2_.real() << " ";
        stream << std::fixed << data.key2_.imag() << "):key3 \"" << data.key3_ << "\":)";
    }
    return stream;
}

