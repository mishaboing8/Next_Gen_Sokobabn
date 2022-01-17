#include <stdio.h>
#include <SDL.h>
#include <SDL_timer.h>
#include <SDL_image.h>
int const CHOOSING_WINDOW_WIDTH = 720;
int const CHOOSING_WINDOW_HEIGHT = 720;

int combineNumTexture(int digit);
int const imgCount = 13;
int notNum = 3;

char const *numbersPaths[imgCount] = {"resources/background.png","resources/box.png","resources/target.png", 
                    "resources/numbers/one.png", "resources/numbers/two.png", 
                    "resources/numbers/three.png", "resources/numbers/four.png", 
                    "resources/numbers/five.png", "resources/numbers/six.png", 
                    "resources/numbers/seven.png", "resources/numbers/eight.png", 
                    "resources/numbers/nine.png", "resources/numbers/zero.png"};

int main(int argc, char* argv[]){
    combineNumTexture(1);
}

int combineNumTexture(int digit){


    const char *window1_title = "SOKOBAN";

    Uint32 render_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;

    SDL_Window  *window = NULL;
    SDL_Renderer *rend = NULL;

    SDL_Surface *numImg[imgCount];
    SDL_Texture *numText[imgCount];
    SDL_Rect xNum[imgCount];

    SDL_Init(SDL_INIT_EVERYTHING);

    if(SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        printf("Failed to initialize SDL: %s\n", SDL_GetError());

        //Quits SDL
        SDL_Quit();
        return 1;
    }

    window = SDL_CreateWindow(window1_title, SDL_WINDOWPOS_CENTERED,
                                              SDL_WINDOWPOS_CENTERED,
                                      CHOOSING_WINDOW_WIDTH, CHOOSING_WINDOW_HEIGHT, 0);

    //Prints error message if SDL failed to create window1
    if(!window)
    {
        printf("Failed to initalize window: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    //Initializes renderer
    rend = SDL_CreateRenderer(window, -1, render_flags);
    if(!rend)
    {
        printf("Failed to initialize renderer: %s\n", SDL_GetError());

        //Destroys window
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

        for(int i = 0; i < imgCount; i++){
        numImg[i] = IMG_Load(numbersPaths[i]);
        if (!numImg[i]){
            printf("Failed to load images\n");
            //Destroys initalized components and quits
            SDL_DestroyRenderer(rend);
            SDL_DestroyWindow(window);
            SDL_Quit();
            return 1;
        }
    }
    
    int texWidth = CHOOSING_WINDOW_WIDTH/10, texHeight = CHOOSING_WINDOW_HEIGHT/10;
    char str[10];
    sprintf(str, "%d", digit);
    int strLen = 0;
    for (int i = 0; i < sizeof(str); i++){
        if(str[i] == 0) break;
        else strLen++;
    }

    SDL_Texture *targetTexture = SDL_CreateTexture(rend, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, texWidth, texHeight);

    for(int i = 0; i < imgCount; i++){
        numText[i] = SDL_CreateTextureFromSurface(rend, numImg[i]);
        SDL_FreeSurface(numImg[i]);
        if (!numText[i] && !targetTexture){
            printf("Failed to texturize images\n");
            for(int j = i; j < imgCount; j++) SDL_FreeSurface(numImg[j]);
            SDL_DestroyRenderer(rend);
            SDL_DestroyWindow(window);
            SDL_Quit();
            return 1;
        }
    }


    for(int i = 0; i < imgCount-notNum; i++){
        xNum[i].x = 0;
        xNum[i].y = 0;
    }

    
    for(int i = 0; i < imgCount-notNum; i++){//1 is background
        xNum[i].w = texWidth;
        xNum[i].h = texHeight;
        xNum[i].x = (i == 0 || i%5 == 0) ? texWidth/2: xNum[i-1].x+texWidth*2;
        xNum[i].y = i < 5 ? texHeight/2 : xNum[i-5].h + texHeight;
    }

    SDL_Rect *xBackground = {0,0,CHOOSING_WINDOW_WIDTH,CHOOSING_WINDOW_HEIGHT};



    SDL_RenderClear(rend);

    SDL_SetRenderTarget(rend, targetTexture);

    SDL_Rect xTar = {0,0,texWidth, texHeight};
    SDL_Rect xPart = {0, texHeight/2 - texHeight/(2*strLen), texWidth/strLen, texHeight/(strLen)};
    SDL_RenderCopy(rend, numText[1], NULL, &xTar);
    int texPos = 0;//0 == 48 , 9 == 57
    for (int i = 0; i < strLen; i++) {
        texPos = str[i]-48 == 0 ? 10 : str[i]-48;//1234567890 0 -> 10
        SDL_RenderCopy(rend, numText[notNum + texPos - 1], NULL, &xPart);
        xPart.x += xPart.w;
    }
    SDL_SetRenderTarget(rend, NULL);
    SDL_RenderClear(rend);
    

    int request_quit = 0;
    int lvlCount = 1;
    while(!request_quit)
    {
        //Processes events
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                request_quit = 1;
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.scancode)
                {
                    case SDL_SCANCODE_A:
                    case SDL_SCANCODE_LEFT:
                        if(--lvlCount < 0) lvlCount++;      
                        break;
                    case SDL_SCANCODE_D:
                    case SDL_SCANCODE_RIGHT:
                        lvlCount++;
                        break;
                    default: break;
                }
            default: break;
            }
        }
    
    SDL_RenderClear(rend);

    SDL_RenderCopy(rend, numText[0], NULL, xBackground);

    for(int i = 0; i < lvlCount; i++){
        //SDL_RenderCopy(rend, numText[1], NULL, &xNum[i]);
        //SDL_RenderCopy(rend, numText[notNum + i], NULL, &xNum[i]);
        SDL_RenderCopy(rend, targetTexture, NULL, &xTar);
    }
    SDL_RenderPresent(rend);

    SDL_Delay(1000/60);
    }
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}