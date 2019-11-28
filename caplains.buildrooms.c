/***************************************************************************
** Author:      Story Caplain
** Date:        Nov 10 2019
** Description: This file is to be ran before caplains.adventure.c
**              It creates the directory of room files that adventure uses
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

struct Room {
    char* name;
    int type; /* 0 for start room, 1 for mid, 2 for end */
    int outbound[6];
};

/* helper function for seeing if an element is already in an array */
int contains(const int *a, int val, int size) {
    int itr;
    for (itr = 0; itr < size; itr++) {
        if (a[itr] == val) {
            return 1;
        }
    }
    return 0;
}

int main() {

    /* new seed for random number generator */
    srand(time(NULL));

    /* make directory named caplains.rooms.<process_id> */
    char room_dir[25];
    pid_t pid = getpid();
    sprintf(room_dir, "caplains.rooms.%d", (int)pid);
    mkdir(room_dir, 0755);

    /* make array of room names */
    char* room_names[10] = {
        "Mercury",
        "Venus",
        "Earth",
        "Moon",
        "Mars",
        "Jupiter",
        "Saturn",
        "Uranus",
        "Neptune",
        "Pluto"
    };

    /* make array for 7 room choices */
    struct Room random_rooms[7];

    /* initialize variables */
    int i,j,k,random_num;
    int found = 0;
    int curr;
    int random_connection;
    int building = 0;
    int check_room;

    /* make array all -1's to start so they get overwritten by valid values */
    int num_array[7] = {-1,-1,-1,-1,-1,-1,-1};

    /* loop through array and add numbers if not already in array */
    for(i = 0; i < 7; i++) {
        random_num = rand() % 10;

        found = contains(num_array, random_num, 7);

        if(found==0) {
            num_array[i] = random_num;
        }
        else {
            i--;
        }
        found = 0;
    }

    while(building==0) {

        /* set set each room's name */
        for(i = 0; i < 7; i++) {
            random_rooms[i].name = room_names[num_array[i]];

            /* set all outbound connections to -1 */
            for(j = 0; j < 6; j++) {
                random_rooms[i].outbound[j] = -1;
            }
            
            /* set room types based on iteration */
            if(i == 0) {
                random_rooms[i].type = 0;
            }
            else if(i == 6) {
                random_rooms[i].type = 2;
            }
            else {
                random_rooms[i].type = 1;
            }
        }

        /* loop through outbound connections, add if not already in array */
        found = 0;

        for(i = 0; i < 6; i++) {

            for(j = 0; j < 6; j++) {
                random_connection = (rand() % (7 - i)) + i;

                while(random_connection == i) {
                    random_connection = (rand() % (7 - i)) + i;
                }

                found = contains(random_rooms[i].outbound, random_connection, 6);

                if(found==0 && random_rooms[i].outbound[j] == -1) {
                    random_rooms[i].outbound[j] = random_connection;
                }
                else if(found==0 && random_rooms[i].outbound[j] != -1) {
                    /* do nothing, here to not overwrite connections */
                }
                else {
                    found = 0;
                }
            }

            /* Connect rooms from both sides, i.e. if room 0 has a room to door 4
 *             then room 4 must have a door to room 0 */
            for(j = 0; j < 6; j++) {

                curr = random_rooms[i].outbound[j];

                found = contains(random_rooms[curr].outbound, i, 6);

                if(found==0) {
                    for(k = 0; k < 6; k++) {
                        if(random_rooms[curr].outbound[k] == -1) {
                            random_rooms[curr].outbound[k] = i;
                            k = 6;
                        }
                    }
                }
                else {
                    found = 0;
                }
            }
        }

        /* uncomment to print connections 
 *         for(i=0;i<7;i++) {
 *                     printf("\n\nRoom: %d\n",i);
 *                                 for(j=0;j<6;j++) {
 *                                                 printf(" %d ", random_rooms[i].outbound[j]);
 *                                                             }
 *                                                                     }
 *                                                                             */

        /* if any room has less than 3 connections, redo the connection building */
        building = 1;
        check_room = 0;
        for(i = 0; i < 7; i++) {
            for(j = 0; j < 6; j++) {
                if(random_rooms[i].outbound[j] != -1) {
                    check_room++;
                }
            }
            if(check_room < 3) {
                building = 0;
            }
        }
    }    

    /* enter created directory from earlier */
    chdir(room_dir);

    /* will help to properly display room connection number */
    int counter = 1;

    /* Write name, connections, and room type to each file */
    for(i = 0; i < 7; i++) {

        FILE* room_file = fopen(random_rooms[i].name, "w"); 
        fprintf(room_file, "ROOM NAME: %s\n", random_rooms[i].name);
        fclose(room_file);

        room_file = fopen(random_rooms[i].name, "a");

        for(j = 0; j < 6; j++) {

            curr = random_rooms[i].outbound[j];

            if(curr != -1) {
                fprintf(room_file, "CONNECTION %d: %s\n", counter, random_rooms[curr].name);
                counter++;
            }           
        }
        counter = 1;

        if(random_rooms[i].type == 0) {
            fprintf(room_file, "ROOM TYPE: %s\n", "START_ROOM"); 
        }
        else if(random_rooms[i].type == 1) {
            fprintf(room_file, "ROOM TYPE: %s\n", "MID_ROOM"); 
        }
        else {
            fprintf(room_file, "ROOM TYPE: %s\n", "END_ROOM");
        }
        
        fclose(room_file);
    }
    return 0;
}
