# Adventure

First complile and execute caplains.buildrooms.c in c89 standard to avoid warnings

gcc -ansi caplains.buildrooms.c -o caplains.buildrooms

./caplains.buildrooms

This creates a subdirectory in the directory caplains.buildrooms in ran in each time it is ran.

A different start room and end room are chosen each time randomly. 
The connections between rooms are randomly assigned as well, however all connections are two-way.

To play the game next compile caplains.adventure.c in c89 standard to avoid warnings

gcc -ansi caplains.adventure.c -o caplains.adventure

./caplains.adventure

The game can now be played. 

Time can be requested by entering "time" at the prompt.

Here is an example run:

gcc -ansi caplains.buildrooms.c -o caplains.buildrooms
./caplains.buildrooms
gcc -ansi caplains.adventure.c -o caplains.adventure
./caplains.adventure

CURRENT LOCATION: Pluto
POSSIBLE CONNECTIONS: Saturn, Mars, Neptune, Moon.
WHERE TO? >Saturn

CURRENT LOCATION: Saturn
POSSIBLE CONNECTIONS: Pluto, Earth, Mars, Moon, Venus.
WHERE TO? >time

Thu Nov 28 13:00:33 2019

CURRENT LOCATION: Saturn
POSSIBLE CONNECTIONS: Pluto, Earth, Mars, Moon, Venus.
WHERE TO? >Moon

YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!
YOU TOOK 2 STEPS. YOUR PATH TO VICTORY WAS:
Saturn
Moon
