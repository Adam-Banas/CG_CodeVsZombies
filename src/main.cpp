#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <functional>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <optional>
#include <queue>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <vector>


//#define TESTS
//#define DEBUG
//#ifndef DEBUG
//#pragma GCC optimize "-O3"
//#endif

//-------------------------------------------------------------------------------
//----------------------------------- Constants ---------------------------------
//-------------------------------------------------------------------------------

constexpr const uint MAX_PLAYERS        = 2;
constexpr const uint PLAYER_INDEX_ME    = 0;
constexpr const uint PLAYER_INDEX_ENEMY = 1;

//-------------------------------------------------------------------------------
//--------------------------------- Configuration -------------------------------
//-------------------------------------------------------------------------------



//-------------------------------------------------------------------------------
//----------------------------------- Utilities ---------------------------------
//-------------------------------------------------------------------------------

// Returns a random number in [min, max]
inline int randomNumber(int min, int max)
{
    static std::random_device dev;
    static std::mt19937 gen{dev()};

    std::uniform_int_distribution<> dis(min, max);

    return dis(gen);
}

template <typename T>
inline const T& sample(const std::vector<T>& obj)
{
    return obj[randomNumber(0, obj.size() - 1)];
}

inline auto toChar(int value)
{
    return static_cast<char>(value + 48);
}

class Point
{
public:
    friend std::istream& operator>>(std::istream&, Point&);

public:
    constexpr explicit Point(int x = 0, int y = 0) noexcept : x{x}, y{y} {}

public:
    auto getX() const noexcept { return x; }
    auto getY() const noexcept { return y; }

private:
    int x;
    int y;
};

bool operator==(const Point& lhs, const Point& rhs)
{
    return lhs.getX() == rhs.getX() && lhs.getY() == rhs.getY();
}

bool operator!=(const Point& lhs, const Point& rhs)
{
    return !(lhs == rhs);
}

// NOTICE: Needed for sorting algorithms
bool operator<(const Point& lhs, const Point& rhs)
{
    return ((lhs.getX() - rhs.getX()) * 1000 + lhs.getY() - rhs.getY()) < 0;
}

std::ostream& operator<<(std::ostream& out, const Point& obj)
{
    out << obj.getX() << ' ' << obj.getY();
    return out;
}

std::istream& operator>>(std::istream& in, Point& obj)
{
    in >> obj.x >> obj.y;
    in.ignore();
    return in;
}

constexpr const auto MOVE_NORTH = Point{0, -1};
constexpr const auto MOVE_EAST  = Point{1, 0};
constexpr const auto MOVE_SOUTH = Point{0, 1};
constexpr const auto MOVE_WEST  = Point{-1, 0};

const std::vector<Point> MOVE_DIRECTIONS = {MOVE_NORTH, MOVE_EAST, MOVE_SOUTH,
                                          MOVE_WEST};

constexpr const Point INCORRECT_POS{-1, -1};

template <typename T>
bool contains(const std::vector<T>& collection, const T& elem)
{
    return std::end(collection)
           != std::find(std::begin(collection), std::end(collection), elem);
}

auto split(std::string input, char delimiter = ' ')
{
    std::vector<std::string> result;
    std::istringstream ss(input);
    std::string token;
    while (std::getline(ss, token, delimiter))
    {
        result.push_back(std::move(token));
    }

    return result;
}

//-------------------------------------------------------------------------------
//---------------------------------- Game state ---------------------------------
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
//----------------------------------- Actions -----------------------------------
//-------------------------------------------------------------------------------

class Action
{
public:
    virtual ~Action() = default;
    void print(std::ostream& out) const noexcept
    {
        doPrint(out);
        if (msg)
        {
            out << " " << *msg;
        }
    }

    void addAdditionalMessage(std::string obj) noexcept { msg = obj; }

protected:
    virtual void doPrint(std::ostream& out) const noexcept = 0;

private:
    std::optional<std::string> msg;
};

std::ostream& operator<<(std::ostream& out, const Action& obj)
{
    obj.print(out);
    return out;
}

using Actions = std::vector<std::unique_ptr<Action>>;

std::ostream& operator<<(std::ostream& out, const Actions& obj)
{
    std::for_each(std::begin(obj), std::end(obj),
                  [&out](const auto& elem) { out << *elem << '|'; });
    return out;
}

//-------------------------------------------------------------------------------
//---------------------------------- Simulation ---------------------------------
//-------------------------------------------------------------------------------



//-------------------------------------------------------------------------------
//--------------------------- Artificial Intelligence ---------------------------
//-------------------------------------------------------------------------------


class ArtificialIntelligence
{
public:
    void update() noexcept
    {

    }

    auto calcPlay() { return Actions{}; }
};

//-------------------------------------------------------------------------------
//-------------------------------- Main function --------------------------------
//-------------------------------------------------------------------------------

#ifndef TESTS

int main()
{
    int factoryCount; // the number of factories
    std::cin >> factoryCount; std::cin.ignore();
    int linkCount; // the number of links between factories
    std::cin >> linkCount; std::cin.ignore();
    for (int i = 0; i < linkCount; i++) {
        int factory1;
        int factory2;
        int distance;
        std::cin >> factory1 >> factory2 >> distance; std::cin.ignore();
        std::cerr << factory1 << " " << factory2 << " " << distance << std::endl;
    }

    // game loop
    while (1) {
        int entityCount; // the number of entities (e.g. factories and troops)
        std::cin >> entityCount; std::cin.ignore();
        for (int i = 0; i < entityCount; i++) {
            int entityId;
            std::string entityType;
            int arg1;
            int arg2;
            int arg3;
            int arg4;
            int arg5;
            std::cin >> entityId >> entityType >> arg1 >> arg2 >> arg3 >> arg4
                >> arg5;
            std::cin.ignore();
        }

        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;


        // Any valid action, such as "WAIT" or "MOVE source destination cyborgs"
        std::cout << "WAIT" << std::endl;
    }
}

#else

class Test
{
public:
    static bool exampleTest()
    {
        return true;
    }
};

int main()
{
    std::cout << "Example test result: "
              << Test::exampleTest() << std::endl;
}

#endif
