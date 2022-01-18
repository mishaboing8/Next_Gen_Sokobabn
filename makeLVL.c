#include <stdio.h>
#include <SDL.h>
#include <SDL_timer.h>
#include <SDL_image.h>
#include <stdbool.h>

int TEX_X_COUNT;
int TEX_Y_COUNT;

int WIDTH_RELATIVETY;
int HEIGHT_RELATIVETY;
int const resourcesCount = 14;
int coolCount = 7;    

char const *resources[resourcesCount] = {
                            "resources/background.png", 
                            "resources/box.png", "resources/wall.png","resources/target.png",
                            "resources/builder.png", "resources/win.png", "resources/icon.png", 
                            "resources/coolBoxOn.png","resources/coolBoxOff.png",
                            "resources/coolBuilder.png", "resources/coolGoogles.png", "resources/coolWall.png",
                            "resources/coolWallUp.png", "resources/coolTarget.png"
                            };
int const imgCount = 10;

char const *numbersPaths[imgCount] = {
                    "resources/numbers/one.png", "resources/numbers/two.png", 
                    "resources/numbers/three.png", "resources/numbers/four.png", 
                    "resources/numbers/five.png", "resources/numbers/six.png", 
                    "resources/numbers/seven.png", "resources/numbers/eight.png", 
                    "resources/numbers/nine.png", "resources/numbers/zero.png"};

int const CHOOSING_WINDOW_WIDTH = 720;
int const CHOOSING_WINDOW_HEIGHT = 720;

static FILE* input;
static char FILE_NAME[] = "maps.txt";//file with created maps


#define MAX_WIDTH 30//max witdth or height of reading maps
#define MAX_HEIGHT 30

SDL_Texture *numText[imgCount];
SDL_Texture *notNumTex[resourcesCount];
bool hasRect(int x, int y, int length, SDL_Rect objects[length]);

int findLVLCount();


int combineNumTexture(SDL_Renderer *rend, int digit, SDL_Texture *targetTexture, int texWidth, int texHeight);

int main(){
    int maxLVL = MAX_HEIGHT > MAX_WIDTH ? MAX_HEIGHT : MAX_WIDTH;
    const char *window_title = "SOKOBAN";

    SDL_Window  *window = NULL;
    SDL_Renderer *rend = NULL;

    SDL_Texture *allNumText[maxLVL];

    SDL_Surface *numImg[imgCount];
    SDL_Surface *notNumImg[resourcesCount];


    SDL_Rect xObj = {0,0,0,0};
    SDL_Rect xObjects[MAX_HEIGHT*MAX_WIDTH] = {xObj};
    int objText[MAX_HEIGHT*MAX_WIDTH] = {0};
    char field[MAX_HEIGHT][MAX_WIDTH];

    for(int x = 0; x < MAX_HEIGHT; x++) for(int y = 0; y < MAX_WIDTH; y++) field[y][x] = '0';
                        

    bool error = false;

    Uint32 render_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
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
    SDL_Surface *icon = IMG_Load(resources[6]);
    for(int i = 0; i < imgCount; i++){
        numImg[i] = IMG_Load(numbersPaths[i]);
        if (!numImg[i] && !icon) error = true;
    }
    
    SDL_SetWindowIcon(window, icon);
    SDL_FreeSurface(icon);

    for(int i = 0; i < resourcesCount; i++){
        notNumImg[i] = IMG_Load(resources[i]);
        if(!notNumImg[i]) error = true;
    }

    if(error){
        printf("Failed to load images\n");
        //Destroys initalized components and quits
        SDL_DestroyRenderer(rend);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    int texWidth = CHOOSING_WINDOW_WIDTH/10, texHeight = CHOOSING_WINDOW_HEIGHT/10;

    for(int i = 0; i < maxLVL; i++){
        allNumText[i] = SDL_CreateTexture(rend, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, texWidth, texHeight);
    }

    for(int i = 0; i < imgCount; i++){
        numText[i] = SDL_CreateTextureFromSurface(rend, numImg[i]);
        SDL_FreeSurface(numImg[i]);
        if (!numText[i]) error = true;
    }

    for(int i = 0; i < resourcesCount; i++){
        notNumTex[i] = SDL_CreateTextureFromSurface(rend, notNumImg[i]);
        SDL_FreeSurface(notNumImg[i]);
        if(!notNumTex[i]) error = true;
    }


    for(int i = 0; i < maxLVL; i++){
        if (!allNumText[i]) error = true;
    }

    if (error) {
        printf("Failed to texturize images\n");
        SDL_DestroyRenderer(rend);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    for (int i = 1; i <= maxLVL; i++) 
        combineNumTexture(rend, i, allNumText[i-1], texWidth, texHeight);
    

    SDL_Rect xBackground = {0,0, CHOOSING_WINDOW_WIDTH, CHOOSING_WINDOW_HEIGHT};
    int request_quit = 0;
    SDL_Rect movRect = {texWidth,texHeight,texWidth,texHeight};
    int frameCount = 0, currTex = 0, fieldLength = 0, clickCount = 0;

    bool grow = false;
    bool choosingBorders = true;

    bool setBuilder = false; 
    int placedBoxCount = 0, placedTargetCount = 0;

    int mouseX = 0, mouseY = 0;


    while(!request_quit)
    {
        if(frameCount == 65){
            grow = !grow;
            frameCount = 0;
        }
        if(frameCount%2 == 0 && !choosingBorders){
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
                request_quit = 1;
                break;
            case SDL_MOUSEBUTTONDOWN:
                if(choosingBorders) break;
                
                int x = movRect.x - movRect.x%texWidth, y = movRect.y - movRect.y%texHeight;

                if(hasRect(x, y, clickCount, xObjects)) break;

                xObjects[clickCount] = movRect;
                xObjects[clickCount].x = x;
                xObjects[clickCount].y = y;
                xObjects[clickCount].w = texWidth;
                xObjects[clickCount].h = texHeight;
                objText[clickCount] = currTex;
                char c = '0';
                switch (currTex)
                {
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
                    if(!setBuilder){
                        c = 'I';
                        setBuilder = true;
                    }
                    break;
                default:
                    c = '0';
                    break;
                }
                field[xObjects[clickCount].y/texHeight][xObjects[clickCount].x / texWidth] = c;
                clickCount++;
                break;
            case SDL_MOUSEMOTION:
                if(!choosingBorders){
                mouseX = event.motion.x;
                mouseY = event.motion.y;

                movRect.x += mouseX <= movRect.x ? -texWidth : mouseX >= movRect.x && mouseX <= movRect.x + texWidth ? 0 : texWidth;
                movRect.y += mouseY <= movRect.y ? -texHeight : mouseY >= movRect.y && mouseY <= movRect.y + texHeight ? 0 : texHeight;
                movRect.x += movRect.x < 0 ? texWidth : movRect.x > CHOOSING_WINDOW_WIDTH ? -texWidth : 0;
                movRect.y += movRect.y < 0 ? texHeight : movRect.y > CHOOSING_WINDOW_HEIGHT ? -texHeight : 0;
                }
                break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.scancode)
                {
                case SDL_SCANCODE_ESCAPE:
                    request_quit = 1;
                    break;
                case SDL_SCANCODE_A:
                case SDL_SCANCODE_LEFT:
                    if(!choosingBorders){
                        if(currTex > 1) currTex--;
                    } else {
                        if(currTex >= 1) {
                            currTex--;
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
                    if(!choosingBorders){
                        if(currTex < 4)currTex++;
                    } else {
                        if(currTex < maxLVL){
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
                case SDL_SCANCODE_RETURN://aka enter
                    if(!choosingBorders) {
                        if(!setBuilder || placedTargetCount != placedBoxCount) break;

                        request_quit = 2;
                    }
                    else {
                        if(currTex == 0) break;

                        fieldLength = currTex;
                        currTex = 1;
                        choosingBorders = false;
                        texWidth = xObjects[0].w;
                        texHeight = xObjects[0].h;
                        movRect.h = texHeight;
                        movRect.w = texWidth;
                        movRect.x = 0;
                        movRect.y = 0;   
                    }
                    break;
                case SDL_SCANCODE_P:
                    printf("\n");
                    for(int y = 0; y < fieldLength; y++){
                        for(int x = 0; x < fieldLength; x++){
                            printf("%c ", field[y][x]);
                        }
                        printf("\n"); 
                    }
                default:
                    break;  
                }

            default:
                break;
            }
        }

        SDL_RenderClear(rend);
        if(choosingBorders){
            if(currTex < 5){
                SDL_RenderCopy(rend, notNumTex[1], NULL, &xBackground);
                for (int i = 0; i < currTex; i++) {
                    SDL_RenderCopy(rend, notNumTex[2], NULL, &xObjects[i]);
                }
                SDL_RenderCopy(rend, numText[currTex == 0 ? 9 : currTex-1], NULL, &xBackground);
            }
            else{
                SDL_RenderCopy(rend, allNumText[currTex-1], NULL, &xBackground);
                for (int i = 0; i < currTex; i++) {
                    SDL_RenderCopy(rend, notNumTex[2], NULL, &xObjects[i]);
                }
            }
        } else {

            SDL_RenderCopy(rend, notNumTex[0], NULL, &xBackground);
            for(int i = 0; i < clickCount; i++){
                SDL_RenderCopy(rend, notNumTex[objText[i]], NULL, &xObjects[i]);
            }

            SDL_RenderCopy(rend, notNumTex[currTex], NULL, &movRect);
        }
        SDL_RenderPresent(rend);

        SDL_Delay(1000 / 120);
    }



    for(int i = 0; i < imgCount; i++)
        SDL_DestroyTexture(numText[i]);

    for(int i = 0; i < maxLVL; i++)
        SDL_DestroyTexture(allNumText[i]);

    for(int i = 0; i < resourcesCount; i++){
        SDL_DestroyTexture(notNumTex[i]);
    }


    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(window);
    SDL_Quit();
    if(request_quit == 1) return 1;

    int lvlCount = findLVLCount();

    input = fopen(FILE_NAME, "a");

    fprintf(input, "\n%d %d %d\n", lvlCount+1, fieldLength, fieldLength);
    for(int y = 0; y < fieldLength; y++){
        for(int x = 0; x < fieldLength; x++){
            fprintf(input, "%c", field[y][x]);
        }
        fprintf(input, "\n");
    }

    fclose(input);

    
}

int combineNumTexture(SDL_Renderer *rend, int digit, SDL_Texture *targetTexture, int texWidth, int texHeight){
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
    SDL_Rect xPart = {0, texHeight/2 - texHeight/(2*strLen), texWidth/strLen, texHeight/(strLen)};
    SDL_RenderCopy(rend, notNumTex[1], NULL, &xTar);
    int texPos = 0;//0 == 48 , 9 == 57
    for (int i = 0; i < strLen; i++) {
        texPos = str[i]-48 == 0 ? 10 : str[i]-48;//1234567890 0 -> 10
        SDL_RenderCopy(rend, numText[texPos - 1], NULL, &xPart);
        xPart.x += xPart.w;
    }
    SDL_SetRenderTarget(rend, NULL);
    SDL_RenderClear(rend);

    return 0;
}

bool hasRect(int x, int y, int length, SDL_Rect objects[length]){
    for(int i = 0; i < length; i++)
        if(x == objects[i].x && y == objects[i].y) return true;
    return false;
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
        if(currLVL == prevLVL) return currLVL;

        for (int i = 0; i < height; i++)
            fscanf(input, "%s", line);//skip current level lines
            
        prevLVL = currLVL; 
    }
    fclose(input);
}