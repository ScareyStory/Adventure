/***************************************************************************
** Author:      Story Caplain
** Date:        Nov 10 2019
** Description: This program lets the user traverse the rooms created by
**              buildrooms. Upon entering the final room the game is won,
**              Entering "time" will give the user the time.
**              This functionality is implemented via mutexes/threading.
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

/* define room struct */
struct Room {
    char name[8];
    char type[11];
    char outbound[6][8];
    int connections;
};

/* create thread and mutex */
pthread_mutex_t lock;
pthread_t thread;

/* holds most recent room, avoids playing old rooms */
char newestDirName[256]; 

void* timecheck(void *arg) {

    pthread_mutex_lock(&lock);

    /* implemented from: */ 
    /* https://stackoverflow.com/questions/5141960/get-the-current-time-in-c */
    time_t t;
    struct tm* timeinfo;
    time(&t);
    timeinfo = localtime(&t);

    /* display time */
    printf("\n%s",asctime(timeinfo));

    /* go back up one directory */
    chdir("..");

   /* write to currentTime.txt */
    FILE* fp = fopen("currentTime.txt", "w");
    if(fp==NULL) {
        printf("\nCould not create currentTime.txt\n");
        exit(EXIT_FAILURE);
    } 
    fprintf(fp, "%s", asctime(timeinfo));
    fclose(fp);

    /* go back to game directory */
    chdir(newestDirName);
    
    pthread_mutex_unlock(&lock);

    return NULL;
}

int main(void) {

    /* much of the code borrowed from manipulating directories lecture */
    int newestDirTime = -1;
    char targetDirPrefix[32] = "caplains.rooms.";
    memset(newestDirName, '\0', sizeof(newestDirName)); /* fill with null */


    int i,j,k; /* iterators if needed */
    DIR* dirToCheck;
    struct dirent *fileInDir;
    struct stat dirAttributes;

    dirToCheck = opendir(".");

    /* if opendir doesn't fail enter while loop reading directory contents */
    if(dirToCheck != 0) {
        while ((fileInDir = readdir(dirToCheck)) != NULL) {

            /* if we find "caplains.rooms." */
            if (strstr(fileInDir->d_name, targetDirPrefix) != NULL) {

                /* get the directory's info */
                stat(fileInDir->d_name, &dirAttributes);

                /* if dir found that is newer than the one stored, use it */
                if ((int)dirAttributes.st_mtime > newestDirTime) {
                    newestDirTime = (int)dirAttributes.st_mtime;
                    memset(newestDirName, '\0', sizeof(newestDirName));
                    strcpy(newestDirName, fileInDir->d_name);
                }
            }
        }
    }
    closedir(dirToCheck);

    /* go into proper folder */
    chdir(newestDirName);
    dirToCheck = opendir(".");

    /* make array of char arrays, will hold room names */
    char name_arr[7][32];

    for(i = 0; i < 7; i++) {
        memset(name_arr[i], '\0', 32*sizeof(char)); /* fill with null */
    }

    /* include rooms names only, not "." or ".." */
    i = 0;
    while ((fileInDir = readdir(dirToCheck)) != NULL) {
        if(strcmp(fileInDir->d_name,"..")!=0 && 
          (strcmp(fileInDir->d_name,".")!=0)) {
            strcpy(name_arr[i], fileInDir->d_name);
            i++;
        }
    }

    FILE* fp;

    char line[32];
      memset(line, '\0', sizeof(line)); /* fill with null */

    char start_room[8];
      memset(start_room, '\0', sizeof(start_room)); /* fill with null */

    char end_room[8];
      memset(end_room, '\0', sizeof(end_room)); /* fill with null */

    /* loop through each line of each file until start room found */
    for(i = 0; i < 7; i++) {
        fp = fopen(name_arr[i], "r");
        while(fgets(line, 32, fp)) {
            if(strncmp(line,"ROOM TYPE: START_ROOM",21)==0) {
                strcpy(start_room, name_arr[i]);
            }
            else if(strncmp(line,"ROOM TYPE: END_ROOM",19)==0) {
                strcpy(end_room, name_arr[i]);
            }
        }
        fclose(fp);
    }
   
    closedir(dirToCheck);

    struct Room game_rooms[7];

    /* fill struct char arrays with null */
    for(i = 0; i < 7; i++) {

        memset(game_rooms[i].name,'\0',sizeof(game_rooms[i].name));
        memset(game_rooms[i].type,'\0',sizeof(game_rooms[i].type));

        /* fill outbound connections with null */
        for(j = 0; j < 6; j++) {
            memset(game_rooms[i].outbound[j],'\0',
                   sizeof(game_rooms[i].outbound[j]));
        }
    }

    memset(line, '\0', sizeof(line)); /* fill with null */

    int size;

    /* going to loop through room structs */
    for(i = 0; i < 7; i++) {

        /* store room name */
        strcpy(game_rooms[i].name, name_arr[i]);

        /* for connection incrementing  */
        j = 0;

        /* open file at current iteration */
        fp = fopen(name_arr[i], "r");

        /* while reading the file look for its connections and type */
        while(fgets(line, 32, fp)) {

            /* change newline to null */
            size = sizeof(line) / sizeof(line[0]);
            for(k=0;k<size;k++) {
                if(line[k]=='\n') {line[k] = '\0';}
            }

            /* if connection line found store it */
            if(strncmp(line, "CONNECTION ", 11)==0) {
                strcpy(game_rooms[i].outbound[j], (line + 14));
                j++;

            }
            /* if room type line found store it */
            else if(strncmp(line, "ROOM TYPE: ", 11)==0) {
                strcpy(game_rooms[i].type, (line + 11));
            }
        }
        /* j will hold the number of connections now, store it */
        game_rooms[i].connections = j;

        /* close current file */
        fclose(fp);
    }

    /* used for playing loop */
    int finished = 0;

    /* will hold user input */
    /* adapted from c for dummies */
    char* buffer;
    size_t b_size = 256;
    buffer = (char *)malloc(b_size * sizeof(char));
    if(buffer==NULL) {
        perror("\nMemory allocation failed\n");
        exit(1);
    }
    /* holds number of chars entered with newline */
    size_t chars;

    /* holds current room name */
    char curr_room_name[8];
    memset(curr_room_name, '\0', sizeof(curr_room_name));

    /* store start_room in curr_room_name to start */
    strcpy(curr_room_name, start_room);

    /* will hold the current room */
    struct Room current;
    for(i = 0; i < 7; i++) {
        if(strcmp(game_rooms[i].name, start_room)==0) {
            current = game_rooms[i]; 
        }
    }

    /* will hold all rooms visited */
    char path[100][10];
    for(i = 0; i < 100; i++) {
        memset(path[i],'\0', 10*sizeof(char));
    }

    /* will be used for searching */
    int found = 0;

    /* keeps track of user's steps */
    int steps = 0;

    /* will hold result of pthread_create */
    int error;

    /* This will be the playing loop */
    while(finished == 0) {

        /* sleep to avoid issue with threading */
        usleep(10000);

         /* display location to user */
        printf("\nCURRENT LOCATION: %s", current.name);

        /* print out connections */
        printf("\nPOSSIBLE CONNECTIONS:");
        printf(" %s",current.outbound[0]);
        for(i = 1; i < current.connections; i++) {
            printf(", %s",current.outbound[i]);
        }
        printf(".");

        /* store input in buffer, chars-- to avoid newline */
        printf("\nWHERE TO? >");
        chars = getline(&buffer,&b_size,stdin);
        chars--;

        /* if user requests time */
        if(strncmp(buffer,"time",4)==0) {

            /* init the thread and check if successful */
            error = pthread_mutex_init(&lock, NULL);
            if(error != 0) {
                printf("\nmutex init failed!\n");exit(1);
            }

            /* lock the mutex */
            pthread_mutex_lock(&lock);

            /* create thread, check if successful */
            error = pthread_create(&thread,NULL,&timecheck,NULL);
            if(error != 0) {
                printf("\nthread creation failed!\n");exit(1);
            }
            usleep(1);
            /* unlock mutex and destroy */
            pthread_mutex_unlock(&lock);
            pthread_mutex_destroy(&lock);

        }

        /* if user entered anything besides time */
        else {
            /* see if they entered a room name */
            found = 0;
            for(i = 0; i < current.connections; i++) {
                if(strncmp(current.outbound[i], buffer, chars)==0) {
                    found = 1;
                }
            }
            /* if valid room connection chosen, add current room to path */
            /* then store chosen room in current */
            if(found==1) {
                for(i = 0; i < 7; i++) {
                    if(strncmp(game_rooms[i].name, buffer, chars)==0) {
                        current = game_rooms[i];
                        strcpy(path[steps], current.name);
                        steps++;
                    }
                }
            }
            /* if bad input inform user */
            else {printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n");}
        }
        /* if at end room end playing loop */
        if(strcmp(current.type, "END_ROOM")==0) {
            finished = 1;
        }
    }

    /* if game ended, display the win message, step count and rooms visited */
    printf("\nYOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
    printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:",steps);
    for(i = 0; i < steps; i++) {
        printf("\n%s",path[i]);
    }
    printf("\n");

    return 0;
}
