#include "oneWay.hpp"

#include <algorithm>
#include <atomic>
#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <mutex>
#include <random>
#include <semaphore.h>
#include <system_error>
#include <vector>

using namespace std;

#define UNIDIRECTIONAL       1
#ifndef WORLDEND
#define WORLDEND             120
#endif
#define NUM_LIZARDS          20
#define NUM_CATS             2
#define MAX_LIZARD_CROSSING  4
#define MAX_LIZARD_SLEEP     3
#define MAX_CAT_SLEEP        3
#define MAX_LIZARD_EAT       5
#define CROSS_SECONDS        2

// Lizards in the same group may share the driveway; the other group waits.
struct DirectionGroup {
    mutex groupMutex;
    int admitted = 0;
};

sem_t dWaySemaphore;
sem_t directionSemaphore;
mutex countMutex;
DirectionGroup sago2MonkeyGrass;
DirectionGroup monkeyGrass2Sago;

int numCrossingSago2MonkeyGrass;
int numCrossingMonkeyGrass2Sago;
int maximumCrossing;
int debug;
atomic<bool> running;

void semaphoreError(const char *operation)
{
    cerr << operation << ": " << error_code(errno, generic_category()).message() << endl;
    abort();
}

void waitSemaphore(sem_t *semaphore)
{
    while (sem_wait(semaphore) != 0) {
        if (errno != EINTR) {
            semaphoreError("sem_wait");
        }
    }
}

void postSemaphore(sem_t *semaphore)
{
    if (sem_post(semaphore) != 0) {
        semaphoreError("sem_post");
    }
}

void enterDirection(DirectionGroup& group)
{
    {
        lock_guard<mutex> lock(group.groupMutex);
        if (group.admitted == 0) {
            waitSemaphore(&directionSemaphore);
        }
        ++group.admitted;
    }
    waitSemaphore(&dWaySemaphore);
}

void leaveDirection(DirectionGroup& group)
{
    postSemaphore(&dWaySemaphore);
    lock_guard<mutex> lock(group.groupMutex);
    if (--group.admitted == 0) {
        postSemaphore(&directionSemaphore);
    }
}

int randomSeconds(int maximum)
{
    thread_local mt19937 generator(random_device{}());
    uniform_int_distribution<int> distribution(1, maximum);
    return distribution(generator);
}

void sleepSeconds(int seconds)
{
    this_thread::sleep_for(chrono::seconds(seconds));
}

Cat::Cat(int id) : _id(id)
{
}

int Cat::getId()
{
    return _id;
}

void Cat::run()
{
    _catThread = thread(catThread, this);
}

void Cat::wait()
{
    if (_catThread.joinable()) {
        _catThread.join();
    }
}

void Cat::sleepNow()
{
    int seconds = randomSeconds(MAX_CAT_SLEEP);
    if (debug) {
        cout << "[" << _id << "] cat sleeping for " << seconds << " seconds" << endl;
    }
    sleepSeconds(seconds);
    if (debug) {
        cout << "[" << _id << "] cat awake" << endl;
    }
}

void Cat::catThread(Cat *aCat)
{
    if (debug) {
        cout << "[" << aCat->getId() << "] cat is alive" << endl;
    }

    while (running.load()) {
        aCat->sleepNow();
        lock_guard<mutex> lock(countMutex);
        if (numCrossingSago2MonkeyGrass + numCrossingMonkeyGrass2Sago > MAX_LIZARD_CROSSING) {
            cerr << "\tThe cats are happy - they have toys." << endl;
            exit(EXIT_FAILURE);
        }
    }
}

Lizard::Lizard(int id) : _id(id)
{
}

int Lizard::getId()
{
    return _id;
}

void Lizard::run()
{
    _aLizard = thread(lizardThread, this);
}

void Lizard::wait()
{
    if (_aLizard.joinable()) {
        _aLizard.join();
    }
}

void Lizard::sleepNow()
{
    int seconds = randomSeconds(MAX_LIZARD_SLEEP);
    if (debug) {
        cout << "[" << _id << "] sleeping for " << seconds << " seconds" << endl;
    }
    sleepSeconds(seconds);
    if (debug) {
        cout << "[" << _id << "] awake" << endl;
    }
}

void Lizard::sago2MonkeyGrassIsSafe()
{
    if (debug) {
        cout << "[" << _id << "] checking sago -> monkey grass" << endl;
    }
    enterDirection(sago2MonkeyGrass);
}

void Lizard::crossSago2MonkeyGrass()
{
    {
        lock_guard<mutex> lock(countMutex);
        ++numCrossingSago2MonkeyGrass;
        int total = numCrossingSago2MonkeyGrass + numCrossingMonkeyGrass2Sago;
        maximumCrossing = max(maximumCrossing, total);
        if (numCrossingMonkeyGrass2Sago && UNIDIRECTIONAL) {
            cerr << "\tCrash! Lizards are crossing in both directions." << endl;
            exit(EXIT_FAILURE);
        }
        if (total > MAX_LIZARD_CROSSING) {
            cerr << "\tThe cats are happy - they have toys." << endl;
            exit(EXIT_FAILURE);
        }
        if (debug) {
            cout << "[" << _id << "] crossing sago -> monkey grass; total=" << total << endl;
        }
    }
    sleepSeconds(CROSS_SECONDS);
    {
        lock_guard<mutex> lock(countMutex);
        --numCrossingSago2MonkeyGrass;
    }
}

void Lizard::madeIt2MonkeyGrass()
{
    if (debug) {
        cout << "[" << _id << "] made the sago -> monkey grass crossing" << endl;
    }
    leaveDirection(sago2MonkeyGrass);
}

void Lizard::eat()
{
    int seconds = randomSeconds(MAX_LIZARD_EAT);
    if (debug) {
        cout << "[" << _id << "] eating for " << seconds << " seconds" << endl;
    }
    sleepSeconds(seconds);
    if (debug) {
        cout << "[" << _id << "] finished eating" << endl;
    }
}

void Lizard::monkeyGrass2SagoIsSafe()
{
    if (debug) {
        cout << "[" << _id << "] checking monkey grass -> sago" << endl;
    }
    enterDirection(monkeyGrass2Sago);
}

void Lizard::crossMonkeyGrass2Sago()
{
    {
        lock_guard<mutex> lock(countMutex);
        ++numCrossingMonkeyGrass2Sago;
        int total = numCrossingSago2MonkeyGrass + numCrossingMonkeyGrass2Sago;
        maximumCrossing = max(maximumCrossing, total);
        if (numCrossingSago2MonkeyGrass && UNIDIRECTIONAL) {
            cerr << "\tCrash! Lizards are crossing in both directions." << endl;
            exit(EXIT_FAILURE);
        }
        if (total > MAX_LIZARD_CROSSING) {
            cerr << "\tThe cats are happy - they have toys." << endl;
            exit(EXIT_FAILURE);
        }
        if (debug) {
            cout << "[" << _id << "] crossing monkey grass -> sago; total=" << total << endl;
        }
    }
    sleepSeconds(CROSS_SECONDS);
    {
        lock_guard<mutex> lock(countMutex);
        --numCrossingMonkeyGrass2Sago;
    }
}

void Lizard::madeIt2Sago()
{
    if (debug) {
        cout << "[" << _id << "] made the monkey grass -> sago crossing" << endl;
    }
    leaveDirection(monkeyGrass2Sago);
}

void Lizard::lizardThread(Lizard *aLizard)
{
    if (debug) {
        cout << "[" << aLizard->getId() << "] lizard is alive" << endl;
    }

    while (running.load()) {
        aLizard->sleepNow();
        aLizard->sago2MonkeyGrassIsSafe();
        aLizard->crossSago2MonkeyGrass();
        aLizard->madeIt2MonkeyGrass();
        aLizard->eat();
        aLizard->monkeyGrass2SagoIsSafe();
        aLizard->crossMonkeyGrass2Sago();
        aLizard->madeIt2Sago();
    }
}

int main(int argc, char **argv)
{
    vector<unique_ptr<Cat>> allCats;
    vector<unique_ptr<Lizard>> allLizards;

    debug = argc > 1 && strncmp(argv[1], "-d", 2) == 0;
    numCrossingSago2MonkeyGrass = 0;
    numCrossingMonkeyGrass2Sago = 0;
    maximumCrossing = 0;
    running.store(true);

    if (sem_init(&dWaySemaphore, 0, MAX_LIZARD_CROSSING) != 0) {
        semaphoreError("sem_init driveway");
    }
    if (sem_init(&directionSemaphore, 0, 1) != 0) {
        semaphoreError("sem_init direction");
    }

    for (int i = 0; i < NUM_LIZARDS; ++i) {
        allLizards.emplace_back(new Lizard(i));
    }
    for (int i = 0; i < NUM_CATS; ++i) {
        allCats.emplace_back(new Cat(i));
    }
    for (const auto& lizard : allLizards) {
        lizard->run();
    }
    for (const auto& cat : allCats) {
        cat->run();
    }

    sleepSeconds(WORLDEND);
    running.store(false);

    for (const auto& lizard : allLizards) {
        lizard->wait();
    }
    for (const auto& cat : allCats) {
        cat->wait();
    }

    if (sem_destroy(&directionSemaphore) != 0) {
        semaphoreError("sem_destroy direction");
    }
    if (sem_destroy(&dWaySemaphore) != 0) {
        semaphoreError("sem_destroy driveway");
    }
    return EXIT_SUCCESS;
}
