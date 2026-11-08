#ifndef ONEWAY_HPP
#define ONEWAY_HPP

#include <thread>

class Cat {
    int _id;
    std::thread _catThread;

public:
    explicit Cat(int id);
    int getId();
    void run();
    void wait();

private:
    void sleepNow();
    static void catThread(Cat *aCat);
};

class Lizard {
    int _id;
    std::thread _aLizard;

public:
    explicit Lizard(int id);
    int getId();
    void run();
    void wait();

private:
    void sago2MonkeyGrassIsSafe();
    void crossSago2MonkeyGrass();
    void madeIt2MonkeyGrass();
    void eat();
    void monkeyGrass2SagoIsSafe();
    void crossMonkeyGrass2Sago();
    void madeIt2Sago();
    void sleepNow();
    static void lizardThread(Lizard *aLizard);
};

#endif // ONEWAY_HPP
