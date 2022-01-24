#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "kbhit.h"
#include <unistd.h>
#define MAX_WIDTH 40
#define MAX_HEIGHT 40

static char field[MAX_HEIGHT][MAX_WIDTH] = {{'0'}};
int height = 0, width = 0;
int currLVL = 0;

static FILE* input;
static char FILE_NAME[256] = "maps.txt";

int targetXPos[MAX_HEIGHT*MAX_WIDTH] = {0}, targetYPos[MAX_HEIGHT*MAX_WIDTH] = {0};//all targets and their positions
int targetCount = 0;

int xPos = 0, yPos = 0;//main character position

/*
empty space = '0' (zero)
wall = '-' '|' '/' '\'
main hero = 'I'
box = 'B'
box target = 'X'
(just in file) box on target = '*', printed will be like 'B'
*/

int loadField(int lvl);//finds and loads level from file
void loadLevel();//same as loadField, but with error catching loop
void printField();
void findHero();//finds main character on map(if there exists more than one character, main will be the most uppier(up) and leftier(left))

void stepLeft();
void stepRight();
void stepUp();
void stepDown();

int readyTargetCount();//returns all targets count, that are with boxes (box on target)
bool isOnTarget(int x, int y);//returns true if object on this coord is on target


int main(){

    if (access(FILE_NAME, F_OK ) != 0){//if file maps.txt doesnt exists, than changes small path to large path in root of all folders
        char cwd[256];                     // maps.txt -> /Users/mihails/Desktop/prozeduale_Prog/pers_Proj/maps.txt
        getcwd(cwd, sizeof(cwd));
        sprintf(FILE_NAME, "%s/%s", cwd, "pers_Proj/maps.txt");
    }

    if(access(FILE_NAME, F_OK ) != 0) return 1;//if changed path does not exists -> error
    system("clear");

    xPos = 0, yPos = 0;
    targetCount = 0;

    loadLevel();
    char side;

    while (true) {
        system("clear");
        printField();

        while(!kbhit());//read char from keyboard
        side = getchar();

        switch (side) {//choose option
            case 'a':
                stepLeft();
                break;
            case 'd':
                stepRight();
                break;
            case 'w':
                stepUp();
                break;
            case 's':
                stepDown();
                break;
            case 'r':
                loadField(currLVL);
                break;
            case 'l':
                loadLevel();
                break;
            case 'e':
                printf("\rCHAO!!!\n");// \r is to erase current line (if without 'r' will be eCHAO instead of CHAO)
                exit(1);
                break;
            default:
                break;
        }

        if(readyTargetCount() == targetCount){//all targets are readey (boxes are on their positions)
            system("clear");
            printField();
            printf("You won!!\nPlay again?(Type y): ");
            if(getchar() == 'y'){
                system("clear");
                loadLevel();
            }
            else break;
        }
    }
}

void printField(){
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++)
            printf("%c", field[i][j] == '0' ? ' ' : field[i][j]);
        printf("\n");
    }
}

void loadLevel(){
    int lvl, errorCode = 0;
    do{
        printf("\rPlease enter existing lvl: ");
        scanf("%d", &lvl);
        errorCode = loadField(lvl);
    } while(errorCode == 0);
}

int loadField(int level){
    char currChar;
    targetCount = 0;
    
    input = fopen(FILE_NAME, "r");
    char line[256];
    int prevLVL = 10;//if there is infinity loop(if lvl not exist), it will roll with same lvl again and again
                        //so if previous lvl == current level -> return error code
    while(1){
        fscanf(input, "%d", &currLVL);
        fscanf(input, "%d", &width);
        fscanf(input, "%d", &height);
        if(currLVL == prevLVL) return 0;
        if (currLVL == level) {
            fscanf(input, "%c", &currChar);//skip new lines char 
            break;//arrived to neccesary level
        } else 
            for (int i = 0; i < height; i++)
                fscanf(input, "%s", line);//skip current level lines
            
        prevLVL = currLVL; 
    }


    for(int i = 0; i < height; i++)
        for(int j = 0; j < width+1; j++){//+1 -> inclusive new line char
            fscanf(input, "%c", &currChar);

            if(currChar == '\n') continue;//skip new line
            else {
                if(currChar == 'X' || currChar == '*'){
                    targetXPos[targetCount] = j;
                    targetYPos[targetCount] = i;
                    targetCount++;
                }
                
                field[i][j] = currChar == '*' ? 'B' : currChar;
            }
        }

        findHero();
        fclose(input);
        return 1;
}

void findHero(){
    //hero char == I
    for(int i = 0; i < height; i++)
        for(int j = 0; j < width; j++)
            if(field[i][j] == 'I'){
                xPos = j;
                yPos = i;
                return;
            }
}

bool isWall(int x, int y){
    return field[y][x] == '-' || field[y][x] == '|' || field[y][x] == '\\' || field[y][x] == '/';
}

void allocateTargets(){//on the places where were boxes or main character could be targets. After every step it is neccesary to place targets back(all 0 -> X)
    for(int i = 0; i < targetCount; i++)
        if (field[targetYPos[i]][targetXPos[i]] == '0')
            field[targetYPos[i]][targetXPos[i]] = 'X';
}

void stepLeft(){
    if(xPos <= 0 || isWall(xPos - 1, yPos)) // wall or out of bounds
        return;

    if(field[yPos][xPos - 1] == 'B' && (isWall(xPos - 2, yPos) || 
    field[yPos][xPos-2] == 'B' || xPos-2 < 0))//after box are wall/anoter box/out of bounds
        return;

    if(field[yPos][xPos - 1] == 'B')
        field[yPos][xPos - 2] = 'B';//move box
        
    field[yPos][xPos] = '0';
    xPos--;//move hero
    field[yPos][xPos] = 'I';

    allocateTargets();//set all targets, because on their positions can stay zero char
    return;
}

void stepRight(){
    if(xPos >= width - 1 || isWall(xPos + 1, yPos)) // wall or out of bounds
        return;

    if(field[yPos][xPos + 1] == 'B' && (isWall(xPos + 2, yPos) || field[yPos][xPos+2] == 'B' || xPos+2 > width - 1))//after box are wall/anoter box/out of bounds
        return;

    if(field[yPos][xPos + 1] == 'B')
        field[yPos][xPos + 2] = 'B';//move box
        
    field[yPos][xPos] = '0';
    xPos++;//move hero
    field[yPos][xPos] = 'I';

    allocateTargets();//set all targets, because on their positions can stay zero char
    return;
}

void stepUp(){
    if(yPos <= 0 || isWall(xPos, yPos-1)) // wall or out of bounds
        return;

    if(field[yPos-1][xPos] == 'B' && (isWall(xPos, yPos-2) || field[yPos-2][xPos] == 'B' || yPos-2 < 0))//after box are wall/anoter box/out of bounds
        return;

    if(field[yPos-1][xPos] == 'B')
        field[yPos-2][xPos] = 'B';//move box
        
    field[yPos][xPos] = '0';
    yPos--;//move hero
    field[yPos][xPos] = 'I';

    allocateTargets();//set all targets, because on their positions can stay zero char
    return;
}

void stepDown(){
    if(yPos >= height - 1 || isWall(xPos, yPos+1)) // wall or out of bounds
        return;

    if(field[yPos+1][xPos] == 'B' && (isWall(xPos, yPos+2) || field[yPos+2][xPos] == 'B' || yPos+2 > height - 1))//after box are wall/anoter box/out of bounds
        return;

    if(field[yPos+1][xPos] == 'B')
        field[yPos+2][xPos] = 'B';//move box
        
    field[yPos][xPos] = '0';
    yPos++;//move hero
    field[yPos][xPos] = 'I';

    allocateTargets();//set all targets, because on their positions can stay zero char
    return;
}

bool isOnTarget(int x, int y){
    for (int i = 0; i < targetCount; i++) 
        if(targetXPos[i] == x && targetYPos[i] == y) return true;
    
    return false;
}

int readyTargetCount(){
    int count = 0;

    for(int i = 0; i < height; i++)
        for(int j = 0; j < width; j++)
            if (field[i][j] == 'B' && isOnTarget(j, i))
                count++;

    return count;
}