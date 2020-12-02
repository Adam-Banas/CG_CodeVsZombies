#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <map>
#include <random>
#include <sstream>
#include <string>
#include <vector>

using std::begin;
using std::cerr;
using std::cout;
using std::end;
using std::endl;
using std::istream;
using std::ostream;
using std::string;
using std::vector;

//#define TESTS
#define DEBUG
#pragma GCC optimize "-O3"

//-------------------------------------------------------------------------------
//--------------------------------- Configuration -------------------------------
//-------------------------------------------------------------------------------

constexpr int TIMEOUT = 98;

//-------------------------------------------------------------------------------
//----------------------------------- Constants ---------------------------------
//-------------------------------------------------------------------------------

constexpr int ASH_SPEED    = 1000;
constexpr int SHOT_RANGE   = 2000;
constexpr int SHOT_RANGE2  = SHOT_RANGE * SHOT_RANGE;
constexpr int ZOMBIE_SPEED = 400;
constexpr double PI        = std::acos(-1);

//-------------------------------------------------------------------------------
//----------------------------------- Utilities ---------------------------------
//-------------------------------------------------------------------------------

inline auto& debugLog() noexcept
{
#ifdef DEBUG
    return std::cerr;
#else
    static std::stringstream sink;
    return sink;
#endif
}

int fibonacci[20];

void initializeFibonacci() noexcept
{
    fibonacci[0] = 1;
    fibonacci[1] = 2;
    for (int i = 2; i < 20; ++i)
    {
        fibonacci[i] = fibonacci[i - 1] + fibonacci[i - 2];
    }
}

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

class Point
{
public:
    constexpr explicit Point(int x = 0, int y = 0) noexcept : x(x), y(y) {}

public:
    int x;
    int y;
};

bool operator==(const Point& lhs, const Point& rhs) noexcept
{
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

bool operator!=(const Point& lhs, const Point& rhs) noexcept
{
    return !(lhs == rhs);
}

auto operator+(const Point& lhs, const Point& rhs) noexcept
{
    return Point(lhs.x + rhs.x, lhs.y + rhs.y);
}

// NOTICE: Needed for sorting algorithms
bool operator<(const Point& lhs, const Point& rhs)
{
    if (lhs.x < rhs.x)
    {
        return true;
    }
    else if (lhs.x > rhs.x)
    {
        return false;
    }
    else
    {
        return lhs.y < rhs.y;
    }
}

std::ostream& operator<<(ostream& out, const Point& obj)
{
    out << obj.x << ' ' << obj.y;
    return out;
}

std::istream& operator>>(istream& in, Point& obj)
{
    in >> obj.x >> obj.y;
    in.ignore();
    return in;
}

// Distance squared, for faster calculations
inline uint dist2(const Point& p1, const Point& p2) noexcept
{
    float dX = abs(p1.x - p2.x);
    float dY = abs(p1.y - p2.y);
    return dX * dX + dY * dY;
}

inline double dist(const Point& p1, const Point& p2) noexcept
{
    return sqrt(dist2(p1, p2));
}

auto toString(const Point& rhs)
{
    string result;

    result = std::to_string(rhs.x) + " " + std::to_string(rhs.y);

    return result;
}

Point randomMove()
{
    auto randomDistance = randomNumber(0, ASH_SPEED);
    auto randomAngle    = randomNumber(0, 359) * PI / 180;
    return Point(randomDistance * sin(randomAngle),
                 randomDistance * cos(randomAngle));
}

Point randomPosition(const Point& position)
{
    auto result = position + randomMove();
    result.x = std::clamp(result.x, 0, 18000);
    result.y = std::clamp(result.y, 0, 9000);
    return result;
}

bool flipCoin() noexcept
{
    return randomNumber(0, 1) == 1;
}

std::chrono::high_resolution_clock::time_point turnStart;

inline double msPassed() noexcept
{
    return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(
               std::chrono::high_resolution_clock::now() - turnStart)
        .count();
}

//-------------------------------------------------------------------------------
//------------------------------------- Model -----------------------------------
//-------------------------------------------------------------------------------

namespace model {

class Zombie
{
public:
    Zombie(int id, Point position) : id(id), position(position) {}

public:
    void simulateMovement(const Point& target) noexcept
    {
        auto distance = dist(position, target);
        if (distance <= ZOMBIE_SPEED)
        {
            position = target;
        }
        else
        {
            auto mult = ZOMBIE_SPEED / distance;
            position.x += ((int)target.x - (int)position.x) * mult;
            position.y += ((int)target.y - (int)position.y) * mult;
        }
    }

public:
    int id;
    Point position;
};

std::ostream& operator<<(ostream& out, const Zombie& obj)
{
    out << "id: " << obj.id << ", pos: " << obj.position;

    return out;
}

class Human
{
public:
    explicit Human(int id, Point position) noexcept : id(id), position(position)
    {
    }

public:
    int id;
    Point position;
};

std::ostream& operator<<(ostream& out, const Human& obj)
{
    out << "id: " << obj.id << ", pos: " << obj.position;

    return out;
}

class Ash
{
public:
    explicit Ash(Point position = Point(0, 0)) noexcept : position(position) {}

public:
    void simulateMovement(const Point& target) noexcept
    {
        auto distance = dist(position, target);
        if (distance <= ASH_SPEED)
        {
            position = target;
        }
        else
        {
            auto mult = ASH_SPEED / distance;
            position.x += ((int)target.x - (int)position.x) * mult;
            position.y += ((int)target.y - (int)position.y) * mult;
        }
    }

    bool isInShootingRange(const Zombie& zombie) const noexcept
    {
        return dist2(position, zombie.position) <= SHOT_RANGE2;
    }

public:
    Point position;
};

class State
{
public:
    explicit State(Ash&& ash                = Ash(),
                   vector<Human>&& humans   = vector<Human>(),
                   vector<Zombie>&& zombies = vector<Zombie>()) noexcept
        : ash(std::move(ash))
        , humans(std::move(humans))
        , zombies(std::move(zombies))
        , score(0)
    {
    }

public:
    void simulateTurn(const Point& ashTarget) noexcept
    {
        simulateZombiesMovement();
        simulateAshMovement(ashTarget);
        simulateAshShot();
        simulateZombiesEat();
    }

    bool isZombieAlive(int id) const noexcept
    {
        return std::any_of(begin(zombies), end(zombies),
                           [id](const auto& elem) { return elem.id == id; });
    }

    auto findZombie(int id) const noexcept
    {
        return std::find_if(begin(zombies), end(zombies),
                            [id](const auto& elem) { return elem.id == id; });
    }

    auto getScore() const noexcept
    {
        if (humans.empty())
        {
            return 0;
        }
        return score;
    }

private:
    void simulateZombiesMovement() noexcept
    {
        for (auto& zombie : zombies)
        {
            simulateZombieMovement(zombie);
        }
    }

    void simulateZombieMovement(Zombie& zombie) noexcept
    {
        Point target;
        auto closest =
            std::min_element(begin(humans), end(humans),
                             [&zombie](const auto& a, const auto& b) {
                                 return dist2(zombie.position, a.position)
                                        < dist2(zombie.position, b.position);
                             });
        if (dist2(zombie.position, closest->position)
            < dist2(zombie.position, ash.position))
        {
            target = closest->position;
        }
        else
        {
            target = ash.position;
        }
        zombie.simulateMovement(target);
    }

    void simulateAshMovement(const Point& target) noexcept
    {
        ash.simulateMovement(target);
    }

    void simulateAshShot() noexcept
    {
        constexpr int ZOMBIE_BASE_SCORE = 10;
        int zombiesKilled               = 0;
        auto zombieScore = humans.size() * humans.size() * ZOMBIE_BASE_SCORE;
        for (int i = zombies.size() - 1; i >= 0; --i)
        {
            if (ash.isInShootingRange(zombies.at(i)))
            {
                //                debugLog() << "Ash shoots zombie " <<
                //                zombies.at(i).id << endl;
                zombies.erase(begin(zombies) + i);
                score += zombieScore * fibonacci[zombiesKilled++];
            }
        }
    }

    void simulateZombiesEat() noexcept
    {
        for (int i = humans.size() - 1; i >= 0; --i)
        {
            auto& human = humans.at(i);
            for (const auto& zombie : zombies)
            {
                if (human.position == zombie.position)
                {
                    //                    debugLog() << "Zombie " << zombie.id
                    //                    << " eats human "
                    //                               << human.id << endl;
                    humans.erase(begin(humans) + i);
                    break;
                }
            }
        }
    }

public:
    Ash ash;
    vector<Human> humans;
    vector<Zombie> zombies;

private:
    int score;
};

std::ostream& operator<<(ostream& out, const State& obj)
{
    out << "Ash: " << obj.ash.position << ", humans: {" << endl;
    for (auto& elem : obj.humans)
    {
        out << elem << endl;
    }
    out << "}, zombies: {" << endl;
    for (auto& elem : obj.zombies)
    {
        out << elem << endl;
    }
    out << "}";

    return out;
}

} // namespace model

//-------------------------------------------------------------------------------
//------------------------------------ View -------------------------------------
//-------------------------------------------------------------------------------

namespace view {
auto readAsh(istream& in)
{
    Point position;
    in >> position;
    return model::Ash(position);
}

auto readHuman(istream& in)
{
    int id;
    Point position;
    in >> id >> position;
    return model::Human(id, position);
}

auto readHumans(istream& in)
{
    int humanCount;
    in >> humanCount;
    in.ignore();
    vector<model::Human> humans;
    humans.reserve(humanCount);
    for (int i = 0; i < humanCount; ++i)
    {
        humans.push_back(readHuman(in));
    }
    return humans;
}

auto readZombie(istream& in)
{
    int id;
    Point current, next;
    in >> id >> current >> next;
    return model::Zombie(id, current);
}

auto readZombies(istream& in)
{
    int zombieCount;
    in >> zombieCount;
    in.ignore();
    vector<model::Zombie> zombies;
    zombies.reserve(zombieCount);

    for (int i = 0; i < zombieCount; i++)
    {
        zombies.push_back(readZombie(in));
    }
    return zombies;
}

auto readTurnInput(istream& in)
{
    auto ash     = readAsh(in);
    auto humans  = readHumans(in);
    auto zombies = readZombies(in);

    return model::State(std::move(ash), std::move(humans), std::move(zombies));
}
} // namespace view

//-------------------------------------------------------------------------------
//--------------------------- Artificial Intelligence ---------------------------
//-------------------------------------------------------------------------------

namespace ai {

bool canSave(const model::State& state, const model::Human& human) noexcept
{
    auto myTurns = ceil(dist(state.ash.position, human.position) / ASH_SPEED);

    for (const auto& zombie : state.zombies)
    {
        auto turns = ceil(dist(zombie.position, human.position) / ZOMBIE_SPEED);
        if (turns < myTurns)
        {
            return false;
        }
    }
    return true;
}

class MonteCarlo
{
public:
    auto calcMove(model::State state) noexcept
    {
        std::pair<int, Point> best = {-1, Point()}, current;
        int states                 = 0;
        while (msPassed() < TIMEOUT)
        {
            ++states;
            //            debugLog() << "states: " << states << endl;
            current = oneSimulation(state);
            if (current.first > best.first)
            {
                best = current;
            }
        }
        debugLog() << "Simulation end, states: " << states << endl;
        return best.second;
    }

private:
    std::pair<int, Point> oneSimulation(model::State state) const noexcept
    {
        Point result;
        bool firstTurn = true;

        if (flipCoin())
        {
            result = randomPosition(state.ash.position);
            state.simulateTurn(result);
            firstTurn = false;
        }

        auto killOrder = state.zombies;
        std::random_shuffle(begin(killOrder), end(killOrder));

        int i = 0;
        while (!(state.humans.empty() || state.zombies.empty()))
        {
            //            debugLog() << "i: " << i << ", state: " << state <<
            //            endl << endl;
            while (!killOrder.empty()
                   && !state.isZombieAlive(killOrder.back().id))
            {
                killOrder.pop_back();
            }

            auto move = state.findZombie(killOrder.back().id)->position;
            if (firstTurn)
            {
                result    = move;
                firstTurn = false;
            }
            state.simulateTurn(move);
            ++i;
        }

        return {state.getScore(), result};
    }
};

} // namespace ai

//-------------------------------------------------------------------------------
//------------------------------------ Main -------------------------------------
//-------------------------------------------------------------------------------

#ifndef TESTS

int main()
{
    initializeFibonacci();
    int score(0);

    ai::MonteCarlo monteCarlo;
    std::chrono::duration<double, std::milli> lastTurnTime;
    while (true)
    {
        auto state = view::readTurnInput(std::cin);
        debugLog() << "Last turn time: " << lastTurnTime.count() << " ms"
                   << endl;
        debugLog() << "Score: " << score << endl;
        turnStart = std::chrono::high_resolution_clock::now();

        auto move = monteCarlo.calcMove(state);

        cout << move << endl;

        state.simulateTurn(move);

        score += state.getScore();

        lastTurnTime = std::chrono::high_resolution_clock::now() - turnStart;
    }
}

#else

class Test
{
public:
    static bool ashMovementTest()
    {
        const model::Ash ash(Point(9000, 4500));
        const std::map<Point, Point> destinations{
            {Point(8000, 4500), Point(8000, 4500)},
            {Point(9300, 4800), Point(9300, 4800)},
            {Point(8700, 4200), Point(8700, 4200)},
            {Point(10000, 5500), Point(9707, 5207)},
            {Point(10000, 4000), Point(9894, 4052)},
            {Point(3000, 0), Point(8200, 3900)},
            {Point(3000, 5500), Point(8013, 4664)}};

        for (const auto& elem : destinations)
        {
            auto newAsh = ash;
            newAsh.simulateMovement(elem.first);
            if (newAsh.position != elem.second)
            {
                cout << "Test failed, target: " << elem.first
                     << ", result: " << newAsh.position
                     << ", desired result: " << elem.second << endl;
                return false;
            }
        }

        return true;
    }

    static bool zombieEatsHumanTest()
    {
        model::State state;
        state.ash.position = Point(0, 0);
        state.humans.push_back(model::Human(1, Point(5000, 5000)));
        state.humans.push_back(model::Human(0, Point(7000, 7000)));
        state.zombies.push_back(model::Zombie(10, Point(6000, 6000)));
        state.zombies.push_back(model::Zombie(11, Point(4900, 4900)));
        state.simulateTurn(Point(0, 0));
        if (state.humans.size() != 1)
        {
            cout << "Wrong number of humans eaten\n";
            return false;
        }
        if (state.humans.front().id != 0)
        {
            //            cout << state << endl;
            cout << "Wrong human eaten\n";
            return false;
        }
        //        cout << state << endl;
        return true;
    }

    static bool ashShootZombieTest()
    {
        model::State state;
        state.ash.position = Point(4500, 4500);
        state.humans.push_back(model::Human(0, Point(7000, 7000)));
        state.humans.push_back(model::Human(1, Point(5000, 5000)));
        state.zombies.push_back(model::Zombie(10, Point(8000, 8000)));
        state.zombies.push_back(model::Zombie(11, Point(4900, 4900)));
        state.simulateTurn(Point(4500, 4500));
        if (state.zombies.size() != 1)
        {
            cout << "Wrong number of zombies shot\n";
            return false;
        }
        if (state.zombies.front().id != 10)
        {
            cout << "Wrong zombie shot\n";
            return false;
        }
        //        cout << state << endl;
        return true;
    }

    static bool randomMoveTest()
    {
        bool firstQ = false, secondQ = false, thirdQ = false, fourthQ = false;

        for (int i = 0; i < 10000; ++i)
        {
            auto move = randomMove();
            if (move.x > 0 && move.y > 0)
            {
                firstQ = true;
            }
            else if (move.x > 0 && move.y < 0)
            {
                secondQ = true;
            }
            else if (move.x < 0 && move.y < 0)
            {
                thirdQ = true;
            }
            else if (move.x < 0 && move.y > 0)
            {
                fourthQ = true;
            }
            if (firstQ && secondQ && thirdQ && fourthQ)
            {
                return true;
            }
        }
        return false;
    }
};

int main()
{
    bool result;
    result = Test::ashMovementTest();
    cout << "Ash movement test result: " << std::boolalpha << result << endl;

    result = Test::zombieEatsHumanTest();
    cout << "Zombie eats human test result: " << std::boolalpha << result
         << endl;

    result = Test::ashShootZombieTest();
    cout << "Ash shoot zombie test result: " << std::boolalpha << result << endl;

    result = Test::randomMoveTest();
    cout << "Random move test result: " << std::boolalpha << result << endl;
}

#endif
