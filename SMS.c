//
//  main.c
//  3ugers 2018 Januar.
//
//  Created by Lasse Christensen on 4/1/2018.
//  Copyright © 2018 Lasse Christensen. All rights reserved.
//

#include <stdio.h>
#include <time.h>
#include <stdlib.h>

const char *listG[]={"Hej", "Hallo", "Halløj", "Goddag", "Yo", "heyhey"};
const char *listA[]={"har", "kan", "vil", "er"};
const char *listS[] = {"du", "Camilla", "i", "katten"};
const char *listV[] = {"lyst", "tid", "mulighed"};
const char *listB[] = {"til kaffe","til at spille fodbold", "en date"};

int SMS = 0;
int seSMS=0;

int main(void)
{
    //Til at simulere en btnvalue bliver 1
    scanf("%d",&SMS);
    time_t t;
    srand((unsigned) time(&t));

    if(SMS==1){
        //alarm lyd indsat
        printf("\a\n");
        printf("Besked modtaget, tryk 1 for at se den");
        //igen simulation af btnvalue.
        scanf("%d",&seSMS);

        if(seSMS==1){
            printf("%s ", listG[rand() % sizeof(listG) / sizeof(listG[0])]);
            printf("%s ", listA[rand() % sizeof(listA) / sizeof(listA[0])]);
            printf("%s ", listS[rand() % sizeof(listS) / sizeof(listS[0])]);
            printf("%s ", listV[rand() % sizeof(listV) / sizeof(listV[0])]);
            printf("%s ", listB[rand() % sizeof(listB) / sizeof(listB[0])]);

            SMS=0;
            seSMS=0;
        }
    }
    return 0;
}




