#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include <numeric>
#include <iomanip>
#include <cmath>
#include <limits>
#include <stdexcept>

const size_t MIN_VERTICES = 3;

class StreamGuard {
public:
    explicit StreamGuard(std::ios& stream)
        : stream_(stream), flags_(stream.flags()) {}
    ~StreamGuard() {
        stream_.flags(flags_);
    }
private:
    std::ios& stream_;
    std::ios::fmtflags flags_;
};

struct Point {
    int x;
    int y;
};

struct Polygon {
    std::vector<Point> points;
};

struct EchoAccumulator {
    std::vector<Polygon>* result_;
    int count_;
    Polygon target_;
};

struct BoundingBox {
    int minX_;
    int maxX_;
    int minY_;
    int maxY_;
};

//operator
std::istream& operator>>(std::istream& stream, Point& point) {
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

    int x{};
    stream >> x;
    if (stream.fail()) {
        return stream;
    }

    if (!stream.get(ch) || ch != ';') {
        stream.setstate(std::ios::failbit);
        return stream;
    }

    int y{};
    stream >> y;
    if (stream.fail()) {
        return stream;
    }

    if (!stream.get(ch) || ch != ')') {
        stream.setstate(std::ios::failbit);
        return stream;
    }

    point.x = x;
    point.y = y;
    return stream;
}

std::istream& operator>>(std::istream& stream, Polygon& polygon) {
    std::istream::sentry sentry(stream);
    if (!sentry) {
        return stream;
    }
    StreamGuard guard(stream);

    size_t n{};
    stream >> n;
    if (stream.fail() || n < MIN_VERTICES) {
        stream.setstate(std::ios::failbit);
        return stream;
    }

    polygon.points.clear();
    polygon.points.reserve(n);

    for (size_t i = 0; i < n; ++i) {
        Point point{};
        stream >> point;
        if (stream.fail()) {
            return stream;
        }
        polygon.points.push_back(point);
    }

    return stream;
}

std::ostream& operator<<(std::ostream& stream, const Point& point) {
    std::ostream::sentry sentry(stream);
    if (!sentry) {
        return stream;
    }
    StreamGuard guard(stream);
    stream << "(" << point.x << ";" << point.y << ")";
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const Polygon& polygon) {
    std::ostream::sentry sentry(stream);
    if (!sentry) {
        return stream;
    }
    StreamGuard guard(stream);
    stream << polygon.points.size();

    for (const auto& point : polygon.points) {
        stream << " " << point;
    }

    return stream;
}

//Polygon
bool operator==(const Polygon& left, const Polygon& right) {
    if (left.points.size() != right.points.size()) {
        return false;
    }
    return std::equal(left.points.begin(), left.points.end(),
        right.points.begin(),
        [](const Point& l, const Point& r) {
            return l.x == r.x && l.y == r.y;
        });
}

Polygon parsePolygon(const std::string& string) {
    size_t spaceCount = std::count(string.begin(), string.end(), ' ');

    std::istringstream iss(string);
    Polygon polygon{};
    
    iss >> polygon;
    if (iss.fail()) {
        throw std::runtime_error("invalid format");
    }

    if (spaceCount != polygon.points.size()) {
        throw std::runtime_error("invalid spaces");
    }

    char leftover{};
    if (iss >> leftover) {
        throw std::runtime_error("extra characters");
    }

    return polygon;
}

//Count
size_t getVertexCount(const Polygon& polygon) {
    return polygon.points.size();
}

bool isEvenVertexCount(const Polygon& polygon) {
    return polygon.points.size() % 2 == 0;
}

bool isOddVertexCount(const Polygon& polygon) {
    return polygon.points.size() % 2 != 0;
}

bool isVertexCount(const Polygon& polygon, size_t count) {
    return polygon.points.size() == count;
}

int countEven(const std::vector<Polygon>& polygons) {
    return std::count_if(polygons.begin(), polygons.end(), isEvenVertexCount);
}

int countOdd(const std::vector<Polygon>& polygons) {
    return std::count_if(polygons.begin(), polygons.end(), isOddVertexCount);
}

int countByVertexCount(const std::vector<Polygon>& polygons, size_t count) {
    auto pred = std::bind(std::equal_to<size_t>(),
        std::bind(getVertexCount, std::placeholders::_1), count);
    return std::count_if(polygons.begin(), polygons.end(), pred);
}

//Area
double area(const Polygon& polygon) {
    const auto& pts = polygon.points;
    const size_t n = pts.size();
    if (n < MIN_VERTICES) {
        return 0.0;
    }
    auto it1 = pts.begin();
    auto it2 = std::next(pts.begin());
    double sum1 = std::inner_product(it1, std::prev(pts.end()), it2, 0.0,
        std::plus<double>(),
        [](const Point& a, const Point& b) {
            return static_cast<double>(a.x) * b.y;
        });
    double sum2 = std::inner_product(it1, std::prev(pts.end()), it2, 0.0,
        std::plus<double>(),
        [](const Point& a, const Point& b) {
            return static_cast<double>(a.y) * b.x;
        });
    double last = static_cast<double>(pts.back().x) * pts.front().y -
                  static_cast<double>(pts.back().y) * pts.front().x;
    return std::abs(sum1 - sum2 + last) / 2.0;
}

double sumAreaEven(const std::vector<Polygon>& polygons) {
    auto evenArea = std::bind(std::multiplies<double>(),
        std::bind(isEvenVertexCount, std::placeholders::_1),
        std::bind(area, std::placeholders::_1));
    return std::accumulate(polygons.begin(), polygons.end(), 0.0,
        std::bind(std::plus<double>(), std::placeholders::_1,
        std::bind(evenArea, std::placeholders::_2)));
}

double sumAreaOdd(const std::vector<Polygon>& polygons) {
    auto oddArea = std::bind(std::multiplies<double>(),
        std::bind(isOddVertexCount, std::placeholders::_1),
        std::bind(area, std::placeholders::_1));
    return std::accumulate(polygons.begin(), polygons.end(), 0.0,
        std::bind(std::plus<double>(), std::placeholders::_1,
        std::bind(oddArea, std::placeholders::_2)));
}

double sumAreaByVertexCount(const std::vector<Polygon>& polygons, size_t count) {
    auto correctArea = std::bind(std::multiplies<double>(),
        std::bind(isVertexCount, std::placeholders::_1, count),
        std::bind(area, std::placeholders::_1));
    return std::accumulate(polygons.begin(), polygons.end(), 0.0,
        std::bind(std::plus<double>(), std::placeholders::_1,
        std::bind(correctArea, std::placeholders::_2)));
}

double meanArea(const std::vector<Polygon>& polygons) {
    if (polygons.empty()) {
        throw std::runtime_error("empty");
    }
    double total = std::accumulate(polygons.begin(), polygons.end(), 0.0,
        std::bind(std::plus<double>(), std::placeholders::_1,
        std::bind(area, std::placeholders::_2)));
    return total / polygons.size();
}

//Max
double maxArea(const std::vector<Polygon>& polygons) {
    if (polygons.empty()) {
        throw std::runtime_error("empty");
    }
    auto pred = std::bind(std::less<double>(),
        std::bind(area, std::placeholders::_1),
        std::bind(area, std::placeholders::_2));
    auto it = std::max_element(polygons.begin(), polygons.end(), pred);
    return area(*it);
}

size_t maxVertexCount(const std::vector<Polygon>& polygons) {
    if (polygons.empty()) {
        throw std::runtime_error("empty");
    }
    auto pred = std::bind(std::less<size_t>(),
        std::bind(getVertexCount, std::placeholders::_1),
        std::bind(getVertexCount, std::placeholders::_2));
    auto it = std::max_element(polygons.begin(), polygons.end(), pred);
    return getVertexCount(*it);
}

//Min
double minArea(const std::vector<Polygon>& polygons) {
    if (polygons.empty()) {
        throw std::runtime_error("empty");
    }
    auto pred = std::bind(std::less<double>(),
        std::bind(area, std::placeholders::_1),
        std::bind(area, std::placeholders::_2));
    auto it = std::min_element(polygons.begin(), polygons.end(), pred);
    return area(*it);
}

size_t minVertexCount(const std::vector<Polygon>& polygons) {
    if (polygons.empty()) {
        throw std::runtime_error("empty");
    }
    auto pred = std::bind(std::less<size_t>(),
        std::bind(getVertexCount, std::placeholders::_1),
        std::bind(getVertexCount, std::placeholders::_2));
    auto it = std::min_element(polygons.begin(), polygons.end(), pred);
    return getVertexCount(*it);
}

//ECHO
EchoAccumulator echoStep(EchoAccumulator acc, const Polygon& p) {
    acc.result_->push_back(p);
    if (p == acc.target_) {
        acc.result_->push_back(p);
        acc.count_++;
    }
    return acc;
}

int duplicateEcho(std::vector<Polygon>& polygons, const Polygon& target) {
    std::vector<Polygon> result{};
    result.reserve(polygons.size() * 2);
    EchoAccumulator acc{&result, 0, target};
    EchoAccumulator finalAcc = std::accumulate(
        polygons.begin(), polygons.end(), acc, echoStep);
    polygons = std::move(result);
    return finalAcc.count_;
}

//INFRAME
BoundingBox getPolyBoundingBox(const Polygon& p) {
    auto cmpX = [](const Point& a, const Point& b) { return a.x < b.x; };
    auto cmpY = [](const Point& a, const Point& b) { return a.y < b.y; };
    auto minItX = std::min_element(p.points.begin(), p.points.end(), cmpX);
    auto maxItX = std::max_element(p.points.begin(), p.points.end(), cmpX);
    auto minItY = std::min_element(p.points.begin(), p.points.end(), cmpY);
    auto maxItY = std::max_element(p.points.begin(), p.points.end(), cmpY);
    return {minItX->x, maxItX->x, minItY->y, maxItY->y};
}

BoundingBox mergeBoundingBoxes(BoundingBox acc, const Polygon& p) {
    BoundingBox pbb = getPolyBoundingBox(p);
    return {
        std::min(acc.minX_, pbb.minX_),
        std::max(acc.maxX_, pbb.maxX_),
        std::min(acc.minY_, pbb.minY_),
        std::max(acc.maxY_, pbb.maxY_)
    };
}

bool isInsideBoundingBox(const Point& p, const BoundingBox& bb) {
    return p.x >= bb.minX_ && p.x <= bb.maxX_ &&
           p.y >= bb.minY_ && p.y <= bb.maxY_;
}

bool inFrame(const std::vector<Polygon>& polygons, const Polygon& target) {
    if (polygons.empty()) {
        throw std::runtime_error("empty");
    }
    BoundingBox initBB = {
        std::numeric_limits<int>::max(),
        std::numeric_limits<int>::lowest(),
        std::numeric_limits<int>::max(),
        std::numeric_limits<int>::lowest()
    };
    BoundingBox bb = std::accumulate(
        polygons.begin(), polygons.end(), initBB, mergeBoundingBoxes);
    auto checkPoint = std::bind(isInsideBoundingBox,
        std::placeholders::_1, std::cref(bb));
    return std::all_of(target.points.begin(), target.points.end(), checkPoint);
}

//Processing
std::vector<std::string> parseCommand(const std::string& line) {
    std::istringstream iss(line);
    std::vector<std::string> words{};
    std::string word{};
    while (iss >> word) {
        words.push_back(word);
    }
    return words;
}

void processCommands(std::vector<Polygon>& polygons) {
    std::string line{};
    while (std::getline(std::cin, line)) {
        if (line.empty()) {
            continue;
        }
        std::vector<std::string> words = parseCommand(line);
        if (words.empty()) {
            continue;
        }
        const std::string& cmd = words[0];
        try {
            if (cmd == "COUNT") {
                if (words.size() != 2) throw std::runtime_error("");
                const std::string& arg = words[1];
                if (arg == "EVEN") {
                    std::cout << countEven(polygons) << "\n";
                } else if (arg == "ODD") {
                    std::cout << countOdd(polygons) << "\n";
                } else {
                    size_t count = static_cast<size_t>(std::stoi(arg));
                    if (count < MIN_VERTICES) throw std::runtime_error("");
                    std::cout << countByVertexCount(polygons, count) << "\n";
                }
            } else if (cmd == "AREA") {
                if (words.size() != 2) throw std::runtime_error("");
                const std::string& arg = words[1];
                if (arg == "EVEN") {
                    std::cout << sumAreaEven(polygons) << "\n";
                } else if (arg == "ODD") {
                    std::cout << sumAreaOdd(polygons) << "\n";
                } else if (arg == "MEAN") {
                    std::cout << meanArea(polygons) << "\n";
                } else {
                    size_t count = static_cast<size_t>(std::stoi(arg));
                    if (count < MIN_VERTICES) throw std::runtime_error("");
                    std::cout << sumAreaByVertexCount(polygons, count) << "\n";
                }
            } else if (cmd == "MAX") {
                if (words.size() != 2) throw std::runtime_error("");
                const std::string& arg = words[1];
                if (arg == "AREA") {
                    std::cout << maxArea(polygons) << "\n";
                } else if (arg == "VERTEXES") {
                    std::cout << maxVertexCount(polygons) << "\n";
                } else {
                    std::cout << "<INVALID COMMAND>" << "\n";
                }
            } else if (cmd == "MIN") {
                if (words.size() != 2) throw std::runtime_error("");
                const std::string& arg = words[1];
                if (arg == "AREA") {
                    std::cout << minArea(polygons) << "\n";
                } else if (arg == "VERTEXES") {
                    std::cout << minVertexCount(polygons) << "\n";
                } else {
                    std::cout << "<INVALID COMMAND>" << "\n";
                }
            } else if (cmd == "ECHO") {
                if (words.size() < 2) throw std::runtime_error("");
                std::string polygonStr = std::accumulate(
                    words.begin() + 1, words.end(), std::string(""),
                    [](const std::string& a, const std::string& b) {
                        return a.empty() ? b : a + " " + b;
                    });
                Polygon target = parsePolygon(polygonStr);
                std::cout << duplicateEcho(polygons, target) << "\n";
            } else if (cmd == "INFRAME") {
                if (words.size() < 2) throw std::runtime_error("");
                std::string polygonStr = std::accumulate(
                    words.begin() + 1, words.end(), std::string(""),
                    [](const std::string& a, const std::string& b) {
                        return a.empty() ? b : a + " " + b;
                    });
                Polygon target = parsePolygon(polygonStr);
                if (inFrame(polygons, target)) {
                    std::cout << "<TRUE>" << "\n";
                } else {
                    std::cout << "<FALSE>" << "\n";
                }
            } else {
                std::cout << "<INVALID COMMAND>" << "\n";
            }
        } catch (const std::exception&) {
            std::cout << "<INVALID COMMAND>" << "\n";
        }
    }
}

std::vector<Polygon> readShapesFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("file not found");
    }
    std::vector<Polygon> result{};
    std::string line{};
    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }
        try {
            result.push_back(parsePolygon(line));
        } catch (...) {}
    }
    return result;
}

//main
int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Error: Invalid arguments. Usage: "
                  << argv[0] << " <filename>\n";
        return 1;
    }
    try {
        std::vector<Polygon> figures = readShapesFromFile(argv[1]);
        std::cout << std::fixed << std::setprecision(1);
        processCommands(figures);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
