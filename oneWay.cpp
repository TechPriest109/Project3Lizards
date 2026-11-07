//Barber Problem Conditional variable lock
initialize capacity semaphore to MAX_LIZARD_CROSSING
initialize direction semaphore to 1

initialize both group mutexes
initialize crossing-counter mutex

set both admitted counts to 0
set both provided crossing counters to 0

initialize thread-safe running state

FUNCTION enterDirection(direction):
    lock the group mutex for this direction

    IF this direction's admitted count is 0:
        wait for the direction semaphore

    increment this direction's admitted count
    unlock the group mutex

    wait for a capacity permit

FUNCTION leaveDirection(direction):
    release the capacity permit

    lock the group mutex for this direction
    decrement this direction's admitted count

    IF the admitted count is now 0:
        release the direction semaphore

    unlock the group mutex

FUNCTION cross in the requested direction:
    lock crossing-counter mutex
    increment the provided crossing counter for this direction
    check the opposite-direction counter if required
    unlock crossing-counter mutex

    simulate crossing

    lock crossing-counter mutex
    decrement the provided crossing counter for this direction
    unlock crossing-counter mutex

WHILE the world is running:
    sleep

    enterDirection(SAGO_TO_GRASS)
    cross from sago to grass
    leaveDirection(SAGO_TO_GRASS)

    eat

    enterDirection(GRASS_TO_SAGO)
    cross from grass to sago
    leaveDirection(GRASS_TO_SAGO)

CAT THREAD:
    WHILE the world is running:
        sleep
        lock crossing-counter mutex
        check the sum of both crossing counters
        unlock crossing-counter mutex
        terminate the program if the limit was exceeded

MAIN:
    initialize shared state and synchronization
    create and start lizard and cat threads
    wait for WORLDEND
    mark the world as no longer running
    join all threads
    destroy synchronization objects
    clean up thread and animal objects