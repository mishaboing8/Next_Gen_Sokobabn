#include <SDL.h>
#include <SDL_timer.h>
#include <SDL_image.h>
#include <stdio.h>
#include <stdbool.h>


int const buildersCount = 11;
char const *buildersPath[buildersCount] = {"resources/characters/builder.png","resources/characters/panda.png","resources/characters/poop.png",
                                            "resources/characters/cowBoy.png","resources/characters/nerd.png","resources/characters/skeleton.png",
                                            "resources/characters/shrek.png", "resources/characters/coolTooth.png", "resources/characters/badTooth.png",
                                            "resources/characters/malevichSquare.png","resources/characters/spongeBob.png"
                                            };


int const resourcesCount = 18;
char const *resources[resourcesCount] = {
                            "resources/background.png", 
                            "resources/box.png", "resources/wall.png","resources/target.png",
                            "resources/builder.png", "resources/win.png", "resources/icon.png", 
                            "resources/coolBoxOn.png","resources/coolBoxOff.png",
                            "resources/coolBuilder.png", "resources/coolGoogles.png", "resources/coolWall.png",
                            "resources/coolWallUp.png", "resources/coolTarget.png",
                            "resources/DIY.png",
                            "resources/pressKey.png", "resources/builderClosedEye.png", "resources/chooseBuilder.png"
                            };

int const imgCount = 10;
char const *numbersPaths[imgCount] = {
                    "resources/numbers/one.png", "resources/numbers/two.png", 
                    "resources/numbers/three.png", "resources/numbers/four.png", 
                    "resources/numbers/five.png", "resources/numbers/six.png", 
                    "resources/numbers/seven.png", "resources/numbers/eight.png", 
                    "resources/numbers/nine.png", "resources/numbers/zero.png"};
                    
int const maxLVL = 30;//maximal possible level
SDL_Texture *resTex[resourcesCount];//all textures
SDL_Texture *numText[imgCount];
SDL_Texture *allNumText[maxLVL];

SDL_Texture *buildTex[buildersCount];
int currBuilder = 0;

SDL_Surface *icon = NULL;

const char *window_title = "SOKOBAN";
Uint32 render_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;


int const CHOOSING_WINDOW_WIDTH = 720;
int const CHOOSING_WINDOW_HEIGHT = 720;
int texWidth = CHOOSING_WINDOW_WIDTH/10, texHeight = CHOOSING_WINDOW_HEIGHT/10;

int initText(SDL_Renderer *rend);
int destroyText();
int combineNumTexture(SDL_Renderer *rend, int digit, SDL_Texture *targetTexture, int texWidth, int texHeight);

int main(){
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

    SDL_Rect xBackground[3];
    SDL_Rect xPressKey = {CHOOSING_WINDOW_WIDTH/42, CHOOSING_WINDOW_HEIGHT/3, CHOOSING_WINDOW_WIDTH, CHOOSING_WINDOW_HEIGHT/4};

    SDL_Rect xChooseBuilder = {0, CHOOSING_WINDOW_HEIGHT/10, CHOOSING_WINDOW_WIDTH*4/5, CHOOSING_WINDOW_HEIGHT/5};
    xChooseBuilder.x = (CHOOSING_WINDOW_WIDTH-xChooseBuilder.w)/2;

    SDL_Rect xBuilder = {0, xPressKey.y + xPressKey.h +xPressKey.h/5, CHOOSING_WINDOW_WIDTH/4, CHOOSING_WINDOW_WIDTH/4};
    xBuilder.x = CHOOSING_WINDOW_WIDTH/2 - xBuilder.w/2;

    for(int i = 0; i < 3; i++){
        xBackground[i].x = i == 0 ? 0 : xBackground[i-1].x+CHOOSING_WINDOW_WIDTH;
        xBackground[i].y = 0;
        xBackground[i].h = CHOOSING_WINDOW_WIDTH;
        xBackground[i].w = CHOOSING_WINDOW_HEIGHT;
    }

    int request_quit = 0;

    bool grow = false;
    int frameCount = 0;

    bool choosingMode = false;

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
                    case SDL_SCANCODE_A:
                    case SDL_SCANCODE_LEFT:
                        if(!choosingMode){
                            choosingMode = true;
                            break;
                        }
                        
                        if(--currBuilder < 0) currBuilder++;
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
        if(choosingMode){
            xBuilder.x = CHOOSING_WINDOW_WIDTH/2 - xBuilder.w/2;
            xBuilder.y = xChooseBuilder .y + xChooseBuilder .h + xChooseBuilder.h/2;
            
        }

        SDL_RenderClear(rend);

        for(int i = 0; i < 3; i++)
            SDL_RenderCopy(rend, resTex[0], NULL, &xBackground[i]);
        
        if(!choosingMode){
            SDL_RenderCopy(rend, resTex[15], NULL, &xPressKey);
            SDL_RenderCopy(rend, resTex[frameCount > 50 ? 16 : 4], NULL, &xBuilder);
        } else {
            SDL_RenderCopy(rend, buildTex[currBuilder], NULL, &xBuilder);
            SDL_RenderCopy(rend, resTex[17], NULL, &xChooseBuilder );
        }
        
        SDL_RenderPresent(rend);

        for(int i = 0; i < 3; i++)
            if(--xBackground[i].x < -CHOOSING_WINDOW_WIDTH) xBackground[i].x = xBackground[i == 0 ? 2 : i-1].x + CHOOSING_WINDOW_WIDTH-10;
        
        if(frameCount == 65){
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
}


int initText(SDL_Renderer *rend){
    SDL_Surface *numImg[imgCount];
    SDL_Surface *resSurf[resourcesCount];
    SDL_Surface *buildSurf[buildersCount];

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

    SDL_FreeSurface(icon);
    return 0;
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