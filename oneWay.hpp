#ifndef ONEWAY_HPP
#define ONEWAY_HPP

DECLARE a direction type:
    SAGO_TO_GRASS
    GRASS_TO_SAGO

DECLARE shared synchronization:
    capacity semaphore
    direction semaphore
    one group mutex for each direction
    crossing-counter mutex

DECLARE shared state:
    number admitted to the sago-to-grass group
    number admitted to the grass-to-sago group

DECLARE helper functions:
    enterDirection(direction)
    leaveDirection(direction)

DECLARE Cat and Lizard classes:
    retain the functions already used in lizard.cpp

#endif // ONEWAY_HPP    