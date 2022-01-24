#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_timer.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <unistd.h>

//--------------GUI CHARACTERISTICS--------------------------
int TEX_X_COUNT;//Objects maximal possible count in x and y achse
int TEX_Y_COUNT;

int const WINDOW_WIDTH = 720;//gaming window screen
int WINDOW_HEIGHT;

int WIDTH_RELATIVETY;//how bright or long are textures
int HEIGHT_RELATIVETY;

int const buildersCount = 11;
char const *buildersPath[buildersCount] = {"resources/characters/builder.png","resources/characters/panda.png","resources/characters/poop.png",
                                            "resources/characters/cowBoy.png","resources/characters/nerd.png","resources/characters/skeleton.png",
                                            "resources/characters/shrek.png", "resources/characters/coolTooth.png", "resources/characters/badTooth.png",
                                            "resources/characters/malevichSquare.png","resources/characters/spongeBob.png"
                                            };


int const resourcesCount = 20;
char const *resources[resourcesCount] = {
                            "resources/background.png", 
                            "resources/box.png", "resources/wall.png","resources/target.png",
                            "resources/builder.png", "resources/win.png", "resources/icon.png", 
                            "resources/coolBoxOn.png","resources/coolBoxOff.png",
                            "resources/coolBuilder.png", "resources/coolGoogles.png", "resources/coolWall.png",
                            "resources/coolWallUp.png", "resources/coolTarget.png",
                            "resources/DIY.png",
                            "resources/pressKey.png", "resources/builderClosedEye.png", "resources/chooseBuilder.png",
                            "resources/car.png", "resources/wheelStep.png"
                            };

int const imgCount = 10;
char const *numbersPaths[imgCount] = {
                    "resources/numbers/one.png", "resources/numbers/two.png", 
                    "resources/numbers/three.png", "resources/numbers/four.png", 
                    "resources/numbers/five.png", "resources/numbers/six.png", 
                    "resources/numbers/seven.png", "resources/numbers/eight.png", 
                    "resources/numbers/nine.png", "resources/numbers/zero.png"};

int const keyCount = 8;
int const notDIYKeys = 1;
char const *keysPaths[keyCount] = {"resources/keys/CTRL.png","resources/keys/AD.png",
                                    "resources/keys/L.png","resources/keys/T.png",
                                    "resources/keys/E.png","resources/keys/Z.png", "resources/keys/R.png",
                                    "resources/keys/B.png"};

int const maxLVL = 30;//maximal possible level

SDL_Texture *resTex[resourcesCount];//all textures
SDL_Texture *numText[imgCount];
SDL_Texture *allNumText[maxLVL];
SDL_Texture *keysText[keyCount];

SDL_Texture *buildTex[buildersCount];
int currBuilder = 0;

SDL_Surface *icon = NULL;

const char *window_title = "SOKOBAN";
Uint32 render_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
                         


//-----------------SOKOBAN LOGIC VALUES----------------------

#define MAX_WIDTH 30//max witdth or height of reading maps
#define MAX_HEIGHT 30

static char field[MAX_HEIGHT][MAX_WIDTH] = {{'0'}};//field charactereistics (at first empty)
int height = 0, width = 0;//starting width or height, and level
int currLVL = 0;

static FILE* input;
static char FILE_NAME[256] = "maps.txt";//file with created maps

int targetXPos[MAX_HEIGHT*MAX_WIDTH] = {0}, targetYPos[MAX_HEIGHT*MAX_WIDTH] = {0};//all targets and their positions
int targetCount = 0;//target or box count (boxCount == targetCount)
int wallsCount = 0;

int xPos = 0, yPos = 0;//main character position

/*
empty space = '0' (zero)
wall = '-' '|' '/' '\'
main hero = 'I'
box = 'B'
box target = 'X'
(just in file) box on target = '*', printed will be like 'B'
*/


int initText(SDL_Renderer *rend);//init and destroy textures
int destroyText();


int loadField(int lvl);//finds and loads level from file
int loadLevel();//same as loadField, but with error catching loop
void printField(SDL_Rect* xBuilder, SDL_Rect xWalls[wallsCount], SDL_Rect xBoxes[targetCount], SDL_Rect xTargets[targetCount]);
void findHero();//finds main character on map(if there exists more than one character, main will be the most uppier(up) and leftier(left))

int stepLeft();
int stepRight();
int stepUp();
int stepDown();

int readyTargetCount();//returns all targets count, that are with boxes (box on target)
bool isOnTarget(int x, int y);//returns true if object on this coord is on target

int play();//play level
int makeLevel();//DIY mode
int choosingSokobalLVL();//choose level
int startingScreen();

int main(int argc, char* argv[])
{
    if (access(FILE_NAME, F_OK ) != 0){//if file maps.txt doesnt exists, than changes small path to large path in root of all folders
        char cwd[256];                     // maps.txt -> /Users/mihails/Desktop/prozeduale_Prog/pers_Proj/maps.txt
        getcwd(cwd, sizeof(cwd));
        sprintf(FILE_NAME, "%s/%s", cwd, "pers_Proj/maps.txt");
    }

    if(access(FILE_NAME, F_OK ) != 0) return 1;//if changed path does not exists -> error

    int returnType = startingScreen();
    if(returnType != 1) return 0;
    while (true) {//main loop
        currLVL = 0;
        returnType = loadLevel();
        if(returnType == -1 ) break;//load level heigt and width of field
        if(returnType == -2){
            returnType = startingScreen();
            if(returnType == -1) break;
            continue;
        }
        if(currLVL == 0) returnType = makeLevel();
        else returnType = play();
        if(returnType == -3) break;
    }
    
    return 0;
}
int play(){
    TEX_X_COUNT = width;
    TEX_Y_COUNT = height;
    WINDOW_HEIGHT = WINDOW_WIDTH * TEX_Y_COUNT/TEX_X_COUNT;//window height = width * field sides difference(16:9)
    WIDTH_RELATIVETY = WINDOW_WIDTH/TEX_X_COUNT;
    HEIGHT_RELATIVETY = WINDOW_HEIGHT/TEX_Y_COUNT;

    SDL_Window  *window = NULL;
    SDL_Renderer*rend = NULL;

    SDL_Texture *targetTexture = NULL;
    SDL_Texture *coolTargetTexture = NULL;

    int request_quit = 0;

    //Initializes SDL
    SDL_Init(SDL_INIT_EVERYTHING);

    //Prints error message if SDL fails to initalize
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        printf("Failed to initialize SDL: %s\n", SDL_GetError());

        //Quits SDL
        SDL_Quit();
        return 1;
    }
    /*
    Creates centered window with the window title pointer, width, height, and
    no flags
    */
    window = SDL_CreateWindow(window_title, SDL_WINDOWPOS_CENTERED,
                                              SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT, 0);

    //Prints error message if SDL failed to create window1
    if(!window)
    {
        printf("Failed to initalize window: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    //Initializes renderer
    rend = SDL_CreateRenderer(window, -1, render_flags);
    if(!rend){
        printf("Failed to initialize renderer: %s\n", SDL_GetError());

        //Destroys window
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    if(initText(rend) == -1){
        SDL_DestroyRenderer(rend);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

    Mix_Music *backAudio = Mix_LoadMUS("resources/audio/dawning.mp3");
    if(!backAudio) {
        printf("Failed to initialize audio: %s\n", SDL_GetError());

        //Destroys window
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }


    //Texturizes loaded images

    SDL_SetWindowIcon(window, icon);

    targetTexture = SDL_CreateTexture(rend, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, WINDOW_WIDTH, WINDOW_HEIGHT);
    coolTargetTexture =SDL_CreateTexture(rend, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, WINDOW_WIDTH, WINDOW_HEIGHT);

    // Structs to hold coordinate positions
    SDL_Rect xBoxes[targetCount];
    SDL_Rect xTargets[targetCount];//all arrays are filled with {0,0,0,0} rect (default value)
    SDL_Rect xWalls[wallsCount];

    SDL_Rect xBackground = {0, 0, 0, 0};
    SDL_Rect xBuilder = {0, 0, 0, 0};
    SDL_Rect xWin = {0, 0, 0, 0};
    SDL_Rect xGoog = {0, 0, 0, 0};

    //hint keys
    SDL_Rect xKeys[notDIYKeys+2];//+2 -> +ctrl and R
    bool ctrlSelected = false;
    xKeys[0].w = WINDOW_WIDTH/7;
    xKeys[0].h = xKeys[0].w/3;
    xKeys[0].y = 0;
    xKeys[0].x = WINDOW_WIDTH-xKeys[0].w;
    for(int i = 1; i < notDIYKeys+1; i++){
        xKeys[i] = xKeys[0];
        xKeys[i].y = xKeys[i-1].y + xKeys[i-1].h;
    }


    //Gets scale and dimensions of textures
    SDL_QueryTexture(resTex[0], NULL, NULL, &xBackground.w, &xBackground.h);
    SDL_QueryTexture(resTex[4], NULL, NULL, &xBuilder.w, &xBuilder.h);
    SDL_QueryTexture(resTex[5], NULL, NULL, &xWin.w, &xWin.h);

    if(WINDOW_WIDTH/WINDOW_HEIGHT >= 3){//win texture is 48 : 16 (1:3)
        xWin.w = WINDOW_HEIGHT;
        xWin.h = WINDOW_HEIGHT/3;
    } else {
        xWin.w = WINDOW_WIDTH;
        xWin.h = WINDOW_WIDTH/3;
    }
        xWin.x = WINDOW_WIDTH/2 - xWin.w/2;
        xWin.y = WINDOW_HEIGHT/2 - xWin.h/2;
    
    //Reorder positions and dimensions of textures (scaling)
    xBackground.w = WINDOW_WIDTH;
    xBackground.h = WINDOW_HEIGHT;

    for (int i = 0; i < targetCount; i++) {
        xBoxes[i].w = WIDTH_RELATIVETY;
        xBoxes[i].h = HEIGHT_RELATIVETY;
        xTargets[i].w = WIDTH_RELATIVETY;
        xTargets[i].h = HEIGHT_RELATIVETY;
    }

    xBuilder.w = WIDTH_RELATIVETY;
    xBuilder.h = HEIGHT_RELATIVETY;
    xGoog.w =  xBuilder.w;
    xGoog.h = xBuilder.h;
    xGoog.x = xBuilder.x;
    xGoog.y = -xGoog.h;//out of the screen


    for(int i = 0; i < wallsCount; i++){
        xWalls[i].h = HEIGHT_RELATIVETY;
        xWalls[i].w = WIDTH_RELATIVETY;
    }
    //locate all abjects, and combine targets with normal background
    printField(&xBuilder, xWalls, xBoxes, xTargets);
    SDL_RenderClear(rend);
    SDL_SetRenderTarget(rend, targetTexture);
    
    SDL_RenderCopy(rend, resTex[0], NULL, &xBackground);

    for(int i = 0; i < targetCount; i++)
        SDL_RenderCopy(rend, resTex[3], NULL, &xTargets[i]);

    SDL_SetRenderTarget(rend, NULL);
    SDL_RenderClear(rend);

    //combine cool staff
    SDL_SetRenderTarget(rend, coolTargetTexture);
    
    SDL_RenderCopy(rend, resTex[0], NULL, &xBackground);                            
    
    for(int i = 0; i < targetCount; i++)
        SDL_RenderCopy(rend, resTex[13], NULL, &xTargets[i]);

    SDL_SetRenderTarget(rend, NULL);
    SDL_RenderClear(rend);


    int wonFrameRate = 0;//wining art timing
    bool coolMode = false, startCoolMode = false;//cool mode and google falling mode
    int coolBoxOn[targetCount];
    int coolWallOn[wallsCount];
    int frameCount = 0;
    srand(time(NULL));//randome textures by cool mode
    for(int i = 0; i < targetCount; i++)
        coolBoxOn[i] = rand() % 2 == 0 ? 0 : 1;

    for(int i = 0; i < wallsCount; i++)
        coolWallOn[i] = rand() % 2 == 0 ? 4 : 5;  
    
    //Initializes game loop
    while(!request_quit)
    {
        //locate all objects
        printField(&xBuilder, xWalls, xBoxes, xTargets);
        xGoog.x = xBuilder.x;

        if(startCoolMode && !coolMode){
            xGoog.y += xGoog.h/30;
        }

        if(xGoog.y >= xBuilder.y && !coolMode){
            Mix_PlayMusic(backAudio, -1);
            coolMode = true;
        }
        //Processes events
        SDL_Event event;

        
        while(SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                request_quit = 3;
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.scancode)
                {
                case SDL_SCANCODE_ESCAPE:
                    request_quit = 1;
                    break;
                case SDL_SCANCODE_RCTRL:
                case SDL_SCANCODE_LCTRL:
                    ctrlSelected = !ctrlSelected;
                    break;
                case SDL_SCANCODE_R://reload level
                    loadField(currLVL);
                    break;
                case SDL_SCANCODE_W:
                case SDL_SCANCODE_UP:
                    stepUp();
                    break;
                case SDL_SCANCODE_A:
                case SDL_SCANCODE_LEFT:
                    stepLeft();
                    break;
                case SDL_SCANCODE_S:
                case SDL_SCANCODE_DOWN:
                    stepDown();
                    break;
                case SDL_SCANCODE_D:
                case SDL_SCANCODE_RIGHT:
                    stepRight();
                    break;
                case SDL_SCANCODE_B:
                    startCoolMode = true;
                    break;
                }
                break;
            }
        }

        //Clears screen for renderer
        SDL_RenderClear(rend);

        //Copies textures to buffer and presents them to screen
        SDL_RenderCopy(rend, !coolMode ? targetTexture : coolTargetTexture, NULL, NULL);

        if(frameCount == 25) {//every 255th frame change cool textures(25 becouse of soundtrack ritmus)
            for(int i = 0; i < targetCount; i++)
                coolBoxOn[i] = rand() % 2 == 0 ? 7 : 8;

            for(int i = 0; i < wallsCount; i++)
                coolWallOn[i] = rand() % 2 == 0 ? 11 : 12;    

            frameCount = 0;
        }

        for(int i = 0; i < targetCount; i++)//show textures cool or normal
            SDL_RenderCopy(rend, !coolMode ? resTex[1] : resTex[coolBoxOn[i]], NULL, &xBoxes[i]);

        for(int i = 0; i < wallsCount; i++)
            SDL_RenderCopy(rend, !coolMode ? resTex[2] : resTex[coolWallOn[i]], NULL, &xWalls[i]);

        SDL_RenderCopy(rend, !coolMode ? buildTex[currBuilder] : resTex[9], NULL, &xBuilder);

        
        if(startCoolMode && !coolMode)
            SDL_RenderCopy(rend, resTex[10], NULL, &xGoog);
        
        if(ctrlSelected){
            for(int i = 0; i < notDIYKeys+1; i++)
                SDL_RenderCopy(rend, keysText[i + (keyCount -1 - notDIYKeys)], NULL, &xKeys[i]);
        } else {
            SDL_RenderCopy(rend, keysText[0], NULL, &xKeys[0]);
        }

        if(readyTargetCount() == targetCount || wonFrameRate > 0){
            SDL_RenderCopy(rend, resTex[5], NULL, &xWin);
            wonFrameRate++;
        }

        SDL_RenderPresent(rend);
        //Waits 1/60th of a second for a 60 fps lock
        SDL_Delay(1000 / 60);
        frameCount++;

        if(wonFrameRate == 100) break;
    }
    //close and free resources
    Mix_CloseAudio();
    Mix_FreeMusic(backAudio);

    destroyText();
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return request_quit == 3 ? -3 : request_quit == 1 ? -1 : 1;
}

//--------------------SOKOBAN MAIN LOGIC---------------------------

void printField(SDL_Rect* xBuilder, SDL_Rect xWalls[wallsCount], SDL_Rect xBoxes[targetCount], SDL_Rect xTargets[targetCount]){
    int bCount = 0, tCount = 0, wCount = 0;
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            switch (field[i][j])
            {
            case '*'://target with box
                xTargets[tCount].x = j * WIDTH_RELATIVETY;
                xTargets[tCount].y = i * HEIGHT_RELATIVETY;
                tCount++;
            case 'B':
                xBoxes[bCount].x = j * WIDTH_RELATIVETY;
                xBoxes[bCount].y = i * HEIGHT_RELATIVETY;
                bCount++;
                break;
            case 'I':
                xBuilder->x = j * WIDTH_RELATIVETY;
                xBuilder->y = i * HEIGHT_RELATIVETY;
                break;
            case 'X':
                xTargets[tCount].x = j * WIDTH_RELATIVETY;
                xTargets[tCount].y = i * HEIGHT_RELATIVETY;
                tCount++;
                break;
            case '\\'://walls (in console version)
            case '/':
            case '|':
            case '-':
                xWalls[wCount].x = j * WIDTH_RELATIVETY;
                xWalls[wCount].y = i * HEIGHT_RELATIVETY;
                wCount++;
                break;
            default:
                break;
            }
        }
    }
}

int loadLevel(){
    int lvl = choosingSokobalLVL();
    if(lvl < 0) return lvl == -3 ? -1 : lvl;
    if(lvl == 0) return 1;//DIY
    if(loadField(lvl) == -1) return -1;
    return 1;
}

int loadField(int level){
    char currChar;
    targetCount = 0;
    wallsCount = 0;

    input = fopen(FILE_NAME, "r");
    char line[256];
    int prevLVL = 10;//if there is infinity loop(if lvl not exist), it will roll with same lvl again and again
                        //so if previous lvl == current level -> return error code
    while(1){
        fscanf(input, "%d", &currLVL);//read lvl width and height of level
        fscanf(input, "%d", &width);
        fscanf(input, "%d", &height);
        if(currLVL == prevLVL) return -1;//if loop -> return error
        if (currLVL == level) {
            fscanf(input, "%c", &currChar);//skip new lines char 
            break;//arrived to neccesary level
        }
        else for (int i = 0; i < height; i++)
                fscanf(input, "%s", line);//skip current level lines
            
        prevLVL = currLVL; 
    }


    for(int i = 0; i < height; i++){
        for(int j = 0; j < width+1; j++){//+1 -> inclusive new line char
            fscanf(input, "%c", &currChar);

            if(currChar == '\n') continue;//skip new line
        
            if(currChar == 'X' || currChar == '*'){
                targetXPos[targetCount] = j;
                targetYPos[targetCount] = i;
                targetCount++;
            }
            if(currChar == '-' || currChar == '|' || currChar == '\\' || currChar == '/') wallsCount++;

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

int stepLeft(){
    if(xPos <= 0 || isWall(xPos - 1, yPos)) // wall or out of bounds
        return 0;

    if(field[yPos][xPos - 1] == 'B' && (isWall(xPos - 2, yPos) || field[yPos][xPos-2] == 'B' || xPos-2 < 0))//after box are wall/anoter box/out of bounds
        return 0;

    if(field[yPos][xPos - 1] == 'B')
        field[yPos][xPos - 2] = 'B';//move box
        
    field[yPos][xPos] = '0';
    xPos--;//move hero
    field[yPos][xPos] = 'I';

    allocateTargets();//set all targets, because on their positions can stay zero char
    return 1;
}

int stepRight(){
    if(xPos >= width - 1 || isWall(xPos + 1, yPos)) // wall or out of bounds
        return 0;

    if(field[yPos][xPos + 1] == 'B' && (isWall(xPos + 2, yPos) || field[yPos][xPos+2] == 'B' || xPos+2 > width - 1))//after box are wall/anoter box/out of bounds
        return 0;

    if(field[yPos][xPos + 1] == 'B')
        field[yPos][xPos + 2] = 'B';//move box
        
    field[yPos][xPos] = '0';
    xPos++;//move hero
    field[yPos][xPos] = 'I';

    allocateTargets();//set all targets, because on their positions can stay zero char
    return 1;
}

int stepUp(){
    if(yPos <= 0 || isWall(xPos, yPos-1)) // wall or out of bounds
        return 0;

    if(field[yPos-1][xPos] == 'B' && (isWall(xPos, yPos-2) || field[yPos-2][xPos] == 'B' || yPos-2 < 0))//after box are wall/anoter box/out of bounds
        return 0;

    if(field[yPos-1][xPos] == 'B')
        field[yPos-2][xPos] = 'B';//move box
        
    field[yPos][xPos] = '0';
    yPos--;//move hero
    field[yPos][xPos] = 'I';

    allocateTargets();//set all targets, because on their positions can stay zero char
    return 1;
}

int stepDown(){
    if(yPos >= height - 1 || isWall(xPos, yPos+1)) // wall or out of bounds
        return 0;

    if(field[yPos+1][xPos] == 'B' && (isWall(xPos, yPos+2) || field[yPos+2][xPos] == 'B' || yPos+2 > height - 1))//after box are wall/anoter box/out of bounds
        return 0;

    if(field[yPos+1][xPos] == 'B')
        field[yPos+2][xPos] = 'B';//move box
        
    field[yPos][xPos] = '0';
    yPos++;//move hero
    field[yPos][xPos] = 'I';

    allocateTargets();//set all targets, because on their positions can stay zero char
    return 1;
}

bool isOnTarget(int x, int y){
    for (int i = 0; i < targetCount; i++) 
        if(targetXPos[i] == x && targetYPos[i] == y) return true;
    
    return false;
}

int readyTargetCount(){//boxes on target count
    int count = 0;
    for(int i = 0; i < height; i++)
        for(int j = 0; j < width; j++)
            if (field[i][j] == 'B' && isOnTarget(j, i))
                count++;

    return count;
}


//---------------------CHOOOSE SOKOBAN LELEVEL --------------------

int const CHOOSING_WINDOW_WIDTH = WINDOW_WIDTH;
int const CHOOSING_WINDOW_HEIGHT = WINDOW_WIDTH;
int texWidth = CHOOSING_WINDOW_WIDTH/10, texHeight = CHOOSING_WINDOW_HEIGHT/10;

int findLVLCount();
int combineNumTexture(SDL_Renderer *rend, int digit, SDL_Texture *targetTexture, int texWidth, int texHeight);

int choosingSokobalLVL(){
    int lvlCount = findLVLCount()+1;//because +DIY
    SDL_Window  *window = NULL;
    SDL_Renderer *rend = NULL;

    SDL_Rect xNum[lvlCount+1];//+1 because +DIY

    SDL_Init(SDL_INIT_EVERYTHING);

    if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("Failed to initialize SDL: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    window = SDL_CreateWindow(window_title, SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,CHOOSING_WINDOW_WIDTH, CHOOSING_WINDOW_HEIGHT, 0);

    if(!window) {
        printf("Failed to initalize window: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    rend = SDL_CreateRenderer(window, -1, render_flags);
    if(!rend) {
        printf("Failed to initialize renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    if(initText(rend) == -1){
        SDL_DestroyRenderer(rend);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    SDL_SetWindowIcon(window, icon);

    int texWidth = CHOOSING_WINDOW_WIDTH/10, texHeight = CHOOSING_WINDOW_HEIGHT/10;
    
    SDL_Rect xBackground = {0,0,CHOOSING_WINDOW_WIDTH,CHOOSING_WINDOW_HEIGHT};
    SDL_Rect xTargeted = {0,0,0,0};


    xTargeted.w = texWidth*4/5;
    xTargeted.h = texHeight*4/5;

    int request_quit = 0;
    int currLVL = 0;
    int choosedLVL = -1;
    
    while(!request_quit)
    { 
        //Processes events
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                request_quit = 3;
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.scancode)
                {
                case SDL_SCANCODE_ESCAPE:
                    request_quit = 1;
                    choosedLVL = -1;
                    break;
                case SDL_SCANCODE_A:
                case SDL_SCANCODE_LEFT:
                    if(--currLVL < 0) currLVL = lvlCount-1;//if out of bounds -> take last
                    break;
                case SDL_SCANCODE_D:
                case SDL_SCANCODE_RIGHT:
                    if(++currLVL >= lvlCount) currLVL = 0; //out of bounds take first
                    break;
                case SDL_SCANCODE_S:
                case SDL_SCANCODE_DOWN:
                    currLVL+=5;//skip line
                    if(currLVL >= lvlCount) currLVL = lvlCount-1;
                    break;
                case SDL_SCANCODE_W:
                case SDL_SCANCODE_UP:
                    currLVL-=5;//skip line
                    if (currLVL < 0) currLVL = 0;
                    break;
                case SDL_SCANCODE_RETURN://aka enter
                    request_quit = 1;
                    choosedLVL = currLVL;
                }
            default:
            break;
            }
        }
    SDL_RenderClear(rend);
    SDL_RenderCopy(rend, resTex[0], NULL, &xBackground);

    for(int i = 0; i < lvlCount; i++){//locate numbers in ordered lines with 5 numbers in line
        xNum[i].w = texWidth;
        xNum[i].h = texHeight;
        xNum[i].x = i == 0 ? texWidth/2 : i%5 == 0 ? xNum[i-5].x : xNum[i-1].x+texWidth*2;
        xNum[i].y = i == 0 ? texHeight/2 : i%5 == 0 ? xNum[i-5].y + texHeight*3/2: xNum[i-1].y;
    }
    
    for(int i = 0; i < lvlCount; i++){
        if(i == currLVL) {//if target on choosed texture
            SDL_RenderCopy(rend, resTex[3], NULL, &xNum[i]);
            xTargeted.x = xNum[i].x + xNum[i].w/10;
            xTargeted.y = xNum[i].y + xNum[i].h/10;
            SDL_RenderCopy(rend, i == 0 ? resTex[14] : allNumText[i-1], NULL, &xTargeted);
        } else {
            SDL_RenderCopy(rend, i == 0 ? resTex[14] : allNumText[i-1], NULL, &xNum[i]);
        }
    }
    SDL_RenderPresent(rend);
    }


    destroyText();
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return request_quit == 3 ? -3 : choosedLVL == -1 ? -2 : choosedLVL;
}

int findLVLCount(){
    int height, currLVL = 0, prevLVL = 10;

    input = fopen(FILE_NAME, "r");
    char line[256];
    //if there is infinity loop(if lvl not exist), it will roll with same lvl again and again
    //so if previous lvl == current level -> return error code
    while(1){
        fscanf(input, "%d", &currLVL);
        fscanf(input, "%d", &height);
        fscanf(input, "%d", &height);
        if(currLVL == prevLVL || currLVL == maxLVL) return currLVL;

        for (int i = 0; i < height; i++)
            fscanf(input, "%s", line);//skip current level lines
            
        prevLVL = currLVL; 
    }
    fclose(input);
}

int combineNumTexture(SDL_Renderer *rend, int digit, SDL_Texture *targetTexture, int texWidth, int texHeight){
    //transforms digit to string(char array)
    char str[10];
    sprintf(str, "%d", digit);
    int strLen = 0;
    for (int i = 0; i < 10; i++){
        if(str[i] == 0) break;
        else strLen++;
    }


    SDL_RenderClear(rend);

    SDL_SetRenderTarget(rend, targetTexture);

    SDL_Rect xTar = {0,0,texWidth, texHeight};
    SDL_Rect xPart = {0, texHeight/2 - texHeight/(2*strLen), texWidth/strLen, texHeight/(strLen)};//xPart is 1/(textureWidth/height)th
    SDL_RenderCopy(rend, resTex[1], NULL, &xTar);                                                   //example 1 is 1/3 of 213

    int texPos = 0;//0 == 48 , 9 == 57
    for (int i = 0; i < strLen; i++) {
        texPos = str[i]-48 == 0 ? 10 : str[i]-48;//1234567890 0 -> 10
        SDL_RenderCopy(rend, numText[texPos-1], NULL, &xPart);
        xPart.x += xPart.w;
    }
    SDL_SetRenderTarget(rend, NULL);
    SDL_RenderClear(rend);

    return 0;
}

//------------------DO YOUR OWN LEVEL----------------------------

bool hasRect(int x, int y, int length, SDL_Rect xObjects[length]);//if given point is in rectange array
void createRandomBorders(int fieldLength, char field[MAX_HEIGHT][MAX_WIDTH]);

int makeLevel(){
    SDL_Window  *window = NULL;
    SDL_Renderer *rend = NULL;

    SDL_Rect xObj = {0,0,0,0};
    SDL_Rect xObjects[MAX_HEIGHT*MAX_WIDTH] = {xObj};//maximal object field
    int objText[MAX_HEIGHT*MAX_WIDTH] = {0};          //and their textures
    char field[MAX_HEIGHT][MAX_WIDTH];

    for(int x = 0; x < MAX_HEIGHT; x++) for(int y = 0; y < MAX_WIDTH; y++) field[y][x] = '0';//fill array with spaces

    SDL_Init(SDL_INIT_EVERYTHING);

    if(SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        printf("Failed to initialize SDL: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    window = SDL_CreateWindow(window_title, SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, CHOOSING_WINDOW_WIDTH, CHOOSING_WINDOW_HEIGHT, 0);

    if(!window)
    {
        printf("Failed to initalize window: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    rend = SDL_CreateRenderer(window, -1, render_flags);
    if(!rend)
    {
        printf("Failed to initialize renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    if(initText(rend) == -1){
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

    Mix_Music *backAudio = Mix_LoadMUS("resources/audio/tokioDrift.mp3");
    if(!backAudio) {
        printf("Failed to initialize audio: %s\n", SDL_GetError());

        //Destroys window
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(rend);
        SDL_Quit();
        return -1;
    }

    SDL_Rect xBackground = {0,0, CHOOSING_WINDOW_WIDTH, CHOOSING_WINDOW_HEIGHT};
    
    SDL_Rect movRect = {texWidth,texHeight,texWidth,texHeight};//choosing (moving square)
    int frameCount = 0, currTex = 0, fieldLength = 0, clickCount = 0;

    bool grow = false;//growing or decreasing mode(moving square)
    bool choosingBorders = true;//demension mode or drawing

    bool placedBuilder = false;//if there exist builder and target and box count (targetC must be boxC)
    int placedBoxCount = 0, placedTargetCount = 0;

    int mouseX = 0, mouseY = 0;

    //hint Keys
    bool ctrlPressed = false;
    SDL_Rect xKeys[keyCount];

    //hint keys end

    //Drift Mode start
    int const tireCount = 20;
    int tireAngles[tireCount] = {0};
    SDL_Rect xCar = {CHOOSING_WINDOW_WIDTH/2, CHOOSING_WINDOW_HEIGHT/2, texWidth, texHeight};
    SDL_Rect xDriver = xCar;
    SDL_Rect xTires[tireCount];

    int vecSum = 0, vertVec = 0, horVec = 0;
    int maxSpeed = 5;
    int angle = 0, angleIncr = 15;
    
    for(int i = 0; i < tireCount; i++)
        xTires[i] = xCar;
    
    bool driftMode = false;
    //drift mode end

    int request_quit = 0;
    while(!request_quit){
        if(frameCount == 65){//growing or decreasing mode (moving square)
            grow = !grow;
            frameCount = 0;
        }
        if(frameCount%2 == 0 && !choosingBorders){//better animation
            movRect.w += grow ? 1 : -1;
            movRect.h += grow ? 1 : -1;
            movRect.x += frameCount%4 == 0 ? 0 : grow ? -1 : 1;
            movRect.y += frameCount%4 == 0 ? 0 : grow ? -1 : 1;
        }

        if(!choosingBorders)frameCount++;

        //Processes events
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                request_quit = 3;
                break;
            case SDL_MOUSEBUTTONDOWN:
                if(choosingBorders) break;
                if(driftMode)break;
                
                int x = movRect.x - movRect.x%texWidth, y = movRect.y - movRect.y%texHeight;

                if(hasRect(x, y, clickCount, xObjects)) break;//finds and sees if on mouse coord are objects

                xObjects[clickCount] = movRect;//if there are no objects locates them
                xObjects[clickCount].x = x;
                xObjects[clickCount].y = y;
                xObjects[clickCount].w = texWidth;
                xObjects[clickCount].h = texHeight;
                objText[clickCount] = currTex;
                
                char c = '0';
                switch (currTex){
                case 1:
                    c = 'B';
                    placedBoxCount++;
                    break;
                case 2:
                    c = '|';
                    break;
                case 3:
                    c = 'X';
                    placedTargetCount++;
                    break;
                case 4:
                    if(placedBuilder) break;
                    
                    c = 'I';
                    break;
                default:
                    c = '0';
                    break;
                }
                if(placedBuilder && 4 == currTex) break;
                if(currTex == 4) placedBuilder = true;

                field[xObjects[clickCount].y/texHeight][xObjects[clickCount].x / texWidth] = c;
                clickCount++;
                break;
            case SDL_MOUSEMOTION:
                if(driftMode)break;
                if(choosingBorders) break;

                mouseX = event.motion.x;
                mouseY = event.motion.y;
                //if mouse is in texture reqtangle -> nothing, otherwise move
                movRect.x += mouseX <= movRect.x ? -texWidth : mouseX >= movRect.x && mouseX <= movRect.x + texWidth ? 0 : texWidth;
                movRect.y += mouseY <= movRect.y ? -texHeight : mouseY >= movRect.y && mouseY <= movRect.y + texHeight ? 0 : texHeight;
                movRect.x += movRect.x < 0 ? texWidth : movRect.x > CHOOSING_WINDOW_WIDTH ? -texWidth : 0;
                movRect.y += movRect.y < 0 ? texHeight : movRect.y > CHOOSING_WINDOW_HEIGHT ? -texHeight : 0;

                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.scancode) {
                case SDL_SCANCODE_ESCAPE:
                    request_quit = 1;
                    break;
                case SDL_SCANCODE_LCTRL:
                case SDL_SCANCODE_RCTRL:
                    if(choosingBorders) break;

                    ctrlPressed = !ctrlPressed;
                    break;

                case SDL_SCANCODE_T:
                    driftMode = !driftMode;
                    if(!driftMode) Mix_PauseMusic();
                    if(driftMode){//reset map
                        Mix_PlayMusic(backAudio, -1);
                        clickCount = 0;
                        placedTargetCount = 0;
                        placedBoxCount = 0;
                        placedBuilder = false;

                        for(int y = 0; y < fieldLength; y++) for(int x = 0; x < fieldLength; x++) field[y][x] = '0';
                    }
                    break;
                case SDL_SCANCODE_A:
                case SDL_SCANCODE_LEFT:
                    if(driftMode){
                        if(--horVec < -maxSpeed) horVec = -maxSpeed;
                        if(angle == 270) break;
                        else angle += angleIncr;
                        break;
                    }
                    if(!choosingBorders){
                        if(currTex > 1) currTex--;//box, target or box
                    } else {
                        if(currTex >= 1) {
                            currTex--;//walls count and their demensions
                            for(int i = 0; i < currTex; i++){
                                xObjects[i].w = CHOOSING_WINDOW_WIDTH/currTex;
                                xObjects[i].h = CHOOSING_WINDOW_HEIGHT/currTex;
                                xObjects[i].x = i == 0 ? 0 : xObjects[i-1].x + xObjects[i].w;
                                xObjects[i].y = i == 0 ? 0 : xObjects[i-1].y + xObjects[i].h;
                            }
                        }
                    }
                    break;
                case SDL_SCANCODE_D:
                case SDL_SCANCODE_RIGHT:
                    if(driftMode){
                        if(++horVec > maxSpeed)  horVec = maxSpeed;
                        if(angle == 90) break;
                        else angle += angleIncr;
                        break;
                    }
                    if(!choosingBorders){
                        if(currTex < 4)currTex++;//box, target or box
                    } else {
                        if(currTex < maxLVL){//walls count and their demensions
                            currTex++;
                            for(int i = 0; i < currTex; i++){
                                xObjects[i].w = CHOOSING_WINDOW_WIDTH/currTex;
                                xObjects[i].h = CHOOSING_WINDOW_HEIGHT/currTex;
                                xObjects[i].x = i == 0 ? 0 : xObjects[i-1].x + xObjects[i].w;
                                xObjects[i].y = i == 0 ? 0 : xObjects[i-1].y + xObjects[i].h;
                            }
                        }
                    }
                    break;
                case SDL_SCANCODE_W:
                    if(driftMode){
                        if(--vertVec < -maxSpeed) vertVec = -maxSpeed;
                        if(angle == 0) break;
                        else angle += angleIncr;
                        break;
                    }

                    break;
                case SDL_SCANCODE_S:
                    if(driftMode){
                        if(++vertVec > maxSpeed)  vertVec = maxSpeed;
                        if(angle == 180) break;
                        else angle += angleIncr;
                        break;
                    }

                    break;
                case SDL_SCANCODE_RETURN://aka enter
                    if(driftMode)break;


                    if(!choosingBorders) {//changing modus or finish job
                        if(!placedBuilder || placedTargetCount != placedBoxCount || placedBoxCount == 0 || placedTargetCount == 0) break;//to finish builder must be placed 
                                                                                        //and target amount must be equal to box amount
                        request_quit = 2;
                    } else {
                        if(currTex == 0) break;//cant choose field with 0 bojects

                        fieldLength = currTex;//save field length
                        currTex = 1;//to start building level texture must be box (not number)
                        choosingBorders = false;//change mode

                        //texture dimensions in choosed field
                        texWidth = xObjects[0].w;
                        texHeight = xObjects[0].h;
                        movRect.h = texHeight;
                        movRect.w = texWidth;
                        movRect.x = 0;
                        movRect.y = 0;   
                        //drift mode
                        xCar.h = texHeight;
                        xCar.w = texWidth;
                        xDriver.h = xCar.h/3;
                        xDriver.w = xCar.w/3;
                        for(int i = 0; i < tireCount; i++){
                            xTires[i].w = texWidth;
                            xTires[i].h = texHeight;
                        }
                        //drift mode end
                        
                        //ctrl mode
                        xKeys[0].w = CHOOSING_WINDOW_WIDTH/7;
                        xKeys[0].h = xKeys[0].w/3;
                        xKeys[0].y = xKeys[0].h/2;
                        xKeys[0].x = CHOOSING_WINDOW_WIDTH - xKeys[0].w;
                        xKeys[1] = xKeys[0];
                        for(int i = 2; i < keyCount-notDIYKeys; i++){
                            xKeys[i] = xKeys[0];
                            xKeys[i].y = xKeys[i-1].y + xKeys[i-1].h;
                        }

                    }
                    break;
                case SDL_SCANCODE_P://print field (moustly debugging)
                    if(choosingBorders) break;

                    printf("\n");
                    for(int y = 0; y < fieldLength; y++){
                        for(int x = 0; x < fieldLength; x++){
                            printf("%c ", field[y][x]);
                        }
                        printf("\n"); 
                    }
                    break;

                case SDL_SCANCODE_E://earese object on cursor position
                    if(driftMode)break;
                    if(choosingBorders) break;
                    if(clickCount == 0) break;

                    //find cursour position
                    int recX = movRect.x - movRect.x%texWidth, recY = movRect.y - movRect.y%texHeight;
                    bool founded = false;

                    for(int i = 0; i < clickCount; i++){//find rectangle and move all to thous previous position(pos--)
                        if(founded) {
                            xObjects[i-1] = xObjects[i];
                            objText[i-1] = objText[i];
                        }
                        if(xObjects[i].x == recX && xObjects[i].y == recY) founded = true;
                    }

                    if(!founded) break;

                    //find wat's the object was and resets all parameters(if needed)
                    char c = field[recY/texHeight][recX/texWidth];
                    field[recY/texHeight][recX/texWidth] = '0';
                    switch (c){
                    case 'I':
                        placedBuilder = false;
                        clickCount--;
                        break;
                    case 'X':
                        placedTargetCount--;
                        clickCount--;
                        break;
                    case 'B':
                        placedBoxCount--;
                        clickCount--;
                        break;
                    case '\\'://walls (in console version)
                    case '/':
                    case '|':
                    case '-':
                        clickCount--;
                        break;
                    default:
                        break;
                    }
                    break;
                case SDL_SCANCODE_Z://erase previos deccision
                    if(driftMode)break;
                    if(choosingBorders) break;
                    
                    if(clickCount <= 0) break;
                    
                    clickCount--;
                    int x = xObjects[clickCount].x/texWidth, y = xObjects[clickCount].y/texHeight;
                    switch (field[y][x]) {
                    case 'X': placedTargetCount--;
                        break;
                    case 'B': placedBoxCount--;
                        break;
                    case 'I': placedBuilder = false;
                        break;
                    default:
                        break;
                    }
                    field[y][x] = '0';
                    break;
                case SDL_SCANCODE_R://erase field
                    if(choosingBorders) break;
                    
                    clickCount = 0;
                    placedTargetCount = 0;
                    placedBoxCount = 0;
                    placedBuilder = false;

                    for(int y = 0; y < fieldLength; y++) for(int x = 0; x < fieldLength; x++) field[y][x] = '0';
                    break;

                case SDL_SCANCODE_L:
                    if(driftMode)break;
                    if(choosingBorders) break;
                    
                    //load random walls
                    //make like R button - erase all and reset
                    clickCount = 0;
                    placedTargetCount = 0;
                    placedBoxCount = 0;
                    placedBuilder = false;
                    //fill up field
                    for(int y = 0; y < fieldLength; y++) for(int x = 0; x < fieldLength; x++) field[y][x] = '0';
                    //find edges and create hull
                    createRandomBorders(fieldLength, field);

                    //create rectangles
                    for(int y = 0; y < fieldLength; y++) for(int x = 0; x < fieldLength; x++){
                        if(field[y][x] == '0') continue;

                        SDL_Rect randWall = {x*texWidth, y*texHeight, texWidth, texHeight};
                        xObjects[clickCount]= randWall;
                        objText[clickCount] = 2;
                        clickCount++;
                    }

                    break;
                default:
                    break;  
                }
            default:
                break;
            }
        }

        SDL_RenderClear(rend);

        if(driftMode){
            int x = xCar.x - xCar.x%texWidth, y = xCar.y - xCar.y%texHeight;
            int fx = xCar.x/texWidth, fy = xCar.y/texHeight;
            
            if(field[fy][fx] == '0'){
                field[fy][fx] = '|';
                
                xObjects[clickCount] = xCar; 
                xObjects[clickCount].x = x;
                xObjects[clickCount].y = y;
                xObjects[clickCount].w = texWidth;
                xObjects[clickCount].h = texHeight;
                objText[clickCount] = 2;
                clickCount++;
            }
            if(angle >= 360) angle %= 360;
            if(angle < 0) angle = 360 + angle;

            xCar.y += vertVec;
            xCar.x += horVec;

            if(xCar.x < 0) {
                xCar.x = 0;
                horVec = 0;
            }
            if(xCar.x > CHOOSING_WINDOW_WIDTH-xCar.w) {
                xCar.x = CHOOSING_WINDOW_WIDTH-xCar.w;
                horVec = 0;
            }
            if(xCar.y < 0){ 
                xCar.y = 0;
                vertVec = 0;
            }
            if(xCar.y > CHOOSING_WINDOW_HEIGHT-xCar.h) {
                xCar.y = CHOOSING_WINDOW_HEIGHT-xCar.h;
                vertVec = 0;
            }

            if(frameCount % 5 == 0){
                for(int i = tireCount-1; i > 0; i--){
                    xTires[i] = xTires[i-1];
                    tireAngles[i] = tireAngles[i-1];
                }

            xTires[0] = xCar;
            tireAngles[0] = angle+90;
        }
        SDL_RenderClear(rend);
        SDL_RenderCopy(rend, resTex[0], NULL, &xBackground);
        vecSum = abs(vertVec) + abs(horVec);
        xDriver.x = xCar.x+xCar.w/2;
        xDriver.y = xCar.y+xCar.h/2;

        for(int i = 0; i < clickCount; i++)
            SDL_RenderCopy(rend, resTex[objText[i]], NULL, &xObjects[i]);
        
        for(int i = 1; i < vecSum; i++)
            SDL_RenderCopyEx(rend, resTex[19], NULL, &xTires[i], tireAngles[i], NULL, NULL);


        SDL_RenderCopyEx(rend, resTex[18], NULL, &xCar, angle+90, NULL, NULL);
        SDL_RenderCopyEx(rend, buildTex[currBuilder], NULL, &xDriver, angle+270, NULL, NULL);

        } else if(choosingBorders){
            if(currTex < 5){//wall in back ground and numbers front or vice versa
                SDL_RenderCopy(rend, resTex[1], NULL, &xBackground);
                for (int i = 0; i < currTex; i++) {
                    SDL_RenderCopy(rend, resTex[2], NULL, &xObjects[i]);
                }
                SDL_RenderCopy(rend, numText[currTex == 0 ? 9 : currTex-1], NULL, &xBackground);
            }
            else{
                SDL_RenderCopy(rend, allNumText[currTex-1], NULL, &xBackground);
                for (int i = 0; i < currTex; i++) {
                    SDL_RenderCopy(rend, resTex[2], NULL, &xObjects[i]);
                }
            }
        } else {
            SDL_RenderCopy(rend, resTex[0], NULL, &xBackground);
            for(int i = 0; i < clickCount; i++){
                SDL_RenderCopy(rend, objText[i] == 4 ? buildTex[currBuilder] : resTex[objText[i]], NULL, &xObjects[i]);
            }

            SDL_RenderCopy(rend, currTex == 4 ? buildTex[currBuilder] : resTex[currTex], NULL, &movRect);
        }

        if(!choosingBorders){
            if(!ctrlPressed) {
                SDL_RenderCopy(rend, keysText[0], NULL, &xKeys[0]);
            } else{
                for(int i = 1; i < keyCount-notDIYKeys; i++)
                    SDL_RenderCopy(rend, keysText[i], NULL, &xKeys[i]);
            }
        }
        SDL_RenderPresent(rend);

        SDL_Delay(1000 / 120);
    }


    Mix_CloseAudio();
    Mix_FreeMusic(backAudio);

    destroyText();

    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(window);
    SDL_Quit();

    if(request_quit == 1 || request_quit == 3) return request_quit == 3 ? -3 : 1;
                                    //if user has typed esc -> escape frome DIY mode 
                                    //otherwise load filed to file
    int lvlCount = findLVLCount();//find level count to put our after

    input = fopen(FILE_NAME, "a");

    fprintf(input, "\n%d %d %d\n", lvlCount+1, fieldLength, fieldLength);//load dimensions
    for(int y = 0; y < fieldLength; y++){//load field
        for(int x = 0; x < fieldLength; x++){
            fprintf(input, "%c", field[y][x]);
        }
        fprintf(input, "\n");
    }

    fclose(input);
    return 2;
}

bool hasRect(int x, int y, int length, SDL_Rect xObjects[length]){//if in that point are objects
    for(int i = 0; i < length; i++)
        if(x == xObjects[i].x && y == xObjects[i].y) return true;
    return false;
}

//------------------MAKE RANDOM FIELD----------------------------
#include "convex_hull.h"

void makeLines(int fieldLength, char field[MAX_HEIGHT][MAX_WIDTH], int m, double x[], double y[], int c[]);

void createRandomBorders(int fieldLength, char field[MAX_HEIGHT][MAX_WIDTH]){
    int n = fieldLength/2, m, c[n];
    double x[n], y[n];

    for(int x = 0; x < fieldLength; x++)
        for(int y = 0; y < fieldLength; y++)
            field[x][y] = '0';
    
    rand_points(n, x, y, 0, fieldLength);
    m = hull(n, x, y, c);
    makeLines(fieldLength, field, m, x, y, c);
            
}

void makeLines(int fieldLength, char field[MAX_HEIGHT][MAX_WIDTH], int m, double x[], double y[], int c[]){
    for (int i = 0; i < m; i++) {
        double x1 = x[c[i]], y1 = y[c[i]], x2, y2;
        if (i == m - 1) {
            x2 = x[c[0]];
            y2 = y[c[0]];
        } else {
            x2 = x[c[i+1]];
            y2 = y[c[i+1]];
        }
        double xMin = x1 < x2 ? x1 : x2, xMax = x1 < x2 ? x2 : x1;
        double yMin = y1 < y2 ? y1 : y2, yMax = y1 < y2 ? y2 : y1;

        double add = 1.0 / 1000;
        //if x1 and x2 are same it is impossible to devide by 0, but on graph it is just straight vertical line
        if (x1 == x2) {
            for (double y = yMin; y < yMax; y += add)
                field[(int)x1][(int)y] = '|';
        } else{
            for (double x = xMin, y; x < xMax; x += add) {
                y = ((x - x1) * (y2 - y1) / (x2 - x1)) + y1;
                field[(int)x][(int)y] = '|';
            }
        }
    }
}

//------------------STARTING SCREEN------------------------------
int startingScreen(){
    SDL_Window *window = NULL;
    SDL_Renderer *rend = NULL;

    SDL_Init(SDL_INIT_EVERYTHING);

    if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("Failed to initialize SDL: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    window = SDL_CreateWindow(window_title, SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,CHOOSING_WINDOW_WIDTH, CHOOSING_WINDOW_HEIGHT, 0);

    if(!window) {
        printf("Failed to initalize window: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    rend = SDL_CreateRenderer(window, -1, render_flags);
    if(!rend) {
        printf("Failed to initialize renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    if(initText(rend) == -1){
        SDL_DestroyRenderer(rend);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    SDL_SetWindowIcon(window, icon);

    SDL_Rect xBackground[3];//3 backgrounds (same texture) coming after previous (1 <- 2 <- 3 <- 1)
    SDL_Rect xPressKey = {CHOOSING_WINDOW_WIDTH/42, CHOOSING_WINDOW_HEIGHT/3, CHOOSING_WINDOW_WIDTH, CHOOSING_WINDOW_HEIGHT/4};

    SDL_Rect xChooseBuilder = {0, CHOOSING_WINDOW_HEIGHT/10, CHOOSING_WINDOW_WIDTH*4/5, CHOOSING_WINDOW_HEIGHT/5};
    xChooseBuilder.x = (CHOOSING_WINDOW_WIDTH-xChooseBuilder.w)/2;

    SDL_Rect xBuilder = {0, xPressKey.y + xPressKey.h +xPressKey.h/5, CHOOSING_WINDOW_WIDTH/4, CHOOSING_WINDOW_WIDTH/4};
    xBuilder.x = CHOOSING_WINDOW_WIDTH/2 - xBuilder.w/2;

    for(int i = 0; i < 3; i++){//3 backgrounds (same texture) coming after previous (1 <- 2 <- 3 <- 1)
        xBackground[i].x = i == 0 ? 0 : xBackground[i-1].x+CHOOSING_WINDOW_WIDTH;
        xBackground[i].y = 0;
        xBackground[i].h = CHOOSING_WINDOW_WIDTH;
        xBackground[i].w = CHOOSING_WINDOW_HEIGHT;
    }

    int request_quit = 0;

    bool grow = false;//animation grows or reduces
    int frameCount = 0;

    bool choosingMode = false;//press key screen or choose character

    while (!request_quit) {
        SDL_Event event;
        while(SDL_PollEvent(&event)){
            switch (event.type){
            case SDL_QUIT:
                request_quit = 1;
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.scancode){
                    case SDL_SCANCODE_ESCAPE:
                        request_quit = 1;
                        break;
                    case SDL_SCANCODE_RETURN:
                        if(choosingMode) request_quit = 2;//character was choosed
                        else choosingMode = true;//change mode (default vakue by all keys)
                        break;
                    case SDL_SCANCODE_A:
                    case SDL_SCANCODE_LEFT:
                        if(!choosingMode){
                            choosingMode = true;
                            break;
                        }
                        
                        if(--currBuilder < 0) currBuilder++;//change builder
                        break;
                    case SDL_SCANCODE_D:
                    case SDL_SCANCODE_RIGHT:
                        if(!choosingMode){
                            choosingMode = true;
                            break;
                        }

                        if(++currBuilder > buildersCount-1) currBuilder--;
                        break;
                    default:
                        choosingMode = true;
                        break;
                }
            default:
                break;
            }
        }
        if(choosingMode){//change position after chhanging mode
            xBuilder.x = CHOOSING_WINDOW_WIDTH/2 - xBuilder.w/2;
            xBuilder.y = xChooseBuilder .y + xChooseBuilder .h + xChooseBuilder.h/2;
        }

        SDL_RenderClear(rend);

        for(int i = 0; i < 3; i++)//render backgrounds
            SDL_RenderCopy(rend, resTex[0], NULL, &xBackground[i]);
        
        if(!choosingMode){//render builder and text
            SDL_RenderCopy(rend, resTex[15], NULL, &xPressKey);
            SDL_RenderCopy(rend, resTex[frameCount > 50 ? 16 : 4], NULL, &xBuilder);
        } else {
            SDL_RenderCopy(rend, buildTex[currBuilder], NULL, &xBuilder);
            SDL_RenderCopy(rend, resTex[17], NULL, &xChooseBuilder );
        }
        
        SDL_RenderPresent(rend);

        for(int i = 0; i < 3; i++)//move backrounds
            if(--xBackground[i].x < -CHOOSING_WINDOW_WIDTH) xBackground[i].x = xBackground[i == 0 ? 2 : i-1].x + CHOOSING_WINDOW_WIDTH-10;
        
        if(frameCount == 65){//change growing animation
            grow = !grow;
            frameCount = 0;
        }
        if(frameCount%2 == 0){
            if(!choosingMode){
                xPressKey.w += grow ? 1 : -1;
                xPressKey.h += grow ? 1 : -1;
                xPressKey.x += frameCount%4 == 0 ? 0 : grow ? -1 : 1;
                xPressKey.y += frameCount%16 == 0 ? 0 : grow ? -1 : 1;
            } else {
                xBuilder.w += grow ? 1 : -1;
                xBuilder.h += grow ? 1 : -1;
                xBuilder.x += frameCount%4 == 0 ? 0 : grow ? -1 : 1;
                xBuilder.y += frameCount%4 == 0 ? 0 : grow ? -1 : 1;
            }
        }
        
        frameCount++;
        SDL_Delay(1000 / 60);
    }
    destroyText();
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return request_quit == 1 ? -1 : 1;
}


//------------------INITIOLIZE AND DESTROY TEXTURES--------------

int initText(SDL_Renderer *rend){
    SDL_Surface *numImg[imgCount];
    SDL_Surface *resSurf[resourcesCount];
    SDL_Surface *buildSurf[buildersCount];
    SDL_Surface *keysSurf[keyCount];

    bool error = false;

    for(int i = 0; i < imgCount; i++){
        numImg[i] = IMG_Load(numbersPaths[i]);
        if (!numImg[i]) {
            error = true;
            printf("Problem in: %s\n", numbersPaths[i]);
        }
    }

    for(int i = 0; i < resourcesCount; i++){
        resSurf[i] = IMG_Load(resources[i]);
        if (!resSurf[i]) {
            error = true;
            printf("Problem in: %s\n", resources[i]);
        }
    }

    for(int i = 0; i < buildersCount; i++){
        buildSurf[i] = IMG_Load(buildersPath[i]);
        if (!buildSurf[i]) {
            error = true;
            printf("Problem in: %s\n", buildersPath[i]);
        }
    }

    for(int i = 0; i < keyCount; i++){
        keysSurf[i] = IMG_Load(keysPaths[i]);
        if (!keysSurf[i]) {
            error = true;
            printf("Problem in: %s\n", keysPaths[i]);
        }
    }

    icon = IMG_Load(resources[6]);
    if(!icon) error = true;

    if(error){
        printf("Failed to load images\n");
        return -1;
    }


    for(int i = 0; i < maxLVL; i++){
        allNumText[i] = SDL_CreateTexture(rend, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, texWidth, texHeight);
    }

    for(int i = 0; i < imgCount; i++){
        numText[i] = SDL_CreateTextureFromSurface(rend, numImg[i]);
        SDL_FreeSurface(numImg[i]);
        if (!numText[i]) error = true;
    }

    for(int i = 0; i < resourcesCount; i++){
        resTex[i] = SDL_CreateTextureFromSurface(rend, resSurf[i]);
        SDL_FreeSurface(resSurf[i]);
        if(!resTex[i]) error = true;
    }

    for(int i = 0; i < buildersCount; i++){
        buildTex[i] = SDL_CreateTextureFromSurface(rend, buildSurf[i]);
        SDL_FreeSurface(buildSurf[i]);
        if(!buildTex[i]) error = true;
    }

    for(int i = 0; i < keyCount; i++){
        keysText[i] = SDL_CreateTextureFromSurface(rend, keysSurf[i]);
        SDL_FreeSurface(keysSurf[i]);
        if(!keysText[i]) error = true;
    }

    for(int i = 0; i < maxLVL; i++){
        if (!allNumText[i]) error = true;
    }

    if (error) {
        printf("Failed to texturize images\n");
        return -1;
    }

    for (int i = 1; i <= maxLVL; i++) 
        combineNumTexture(rend, i, allNumText[i-1], texWidth, texHeight);

    return 0;
}

int destroyText(){
    for(int i = 0; i < imgCount; i++)
        SDL_DestroyTexture(numText[i]);

    for(int i = 0; i < maxLVL; i++)
        SDL_DestroyTexture(allNumText[i]);

    for(int i = 0; i < resourcesCount; i++)
        SDL_DestroyTexture(resTex[i]);

    for(int i = 0; i < buildersCount; i++)
        SDL_DestroyTexture(buildTex[i]);
    
    for(int i = 0; i < keyCount; i++)
        SDL_DestroyTexture(keysText[i]);

    SDL_FreeSurface(icon);
    return 0;
}
