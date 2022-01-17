#include <stdio.h>
#include <SDL.h>
#include <SDL_timer.h>
#include <SDL_image.h>

static FILE* input;
static char FILE_NAME[] = "maps.txt";

int const imgCount = 13;
int const notNum = 3;

char *numbersPaths[imgCount] = {"resources/background.png","resources/box.png","resources/target.png",
                    "resources/numbers/zero.png", "resources/numbers/one.png", 
                    "resources/numbers/two.png", "resources/numbers/three.png", 
                    "resources/numbers/four.png", "resources/numbers/five.png", 
                    "resources/numbers/six.png", "resources/numbers/seven.png", 
                    "resources/numbers/eight.png", "resources/numbers/nine.png"};

int const CHOOSING_WINDOW_WIDTH = 720;
int const CHOOSING_WINDOW_HEIGHT = 720;

int findLVLCount();

int choosingSokobalLVL(){
    int lvlCount = findLVLCount();
    
    const char *window_title = "SOKOBAN";

    SDL_Window  *window = NULL;
    SDL_Renderer *rend = NULL;

    SDL_Surface *numImg[imgCount];
    SDL_Texture *numText[imgCount];
    SDL_Rect xNum[imgCount];


    Uint32 render_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
    SDL_Init(SDL_INIT_EVERYTHING);

    if(SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        printf("Failed to initialize SDL: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    window = SDL_CreateWindow(window_title, SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,CHOOSING_WINDOW_WIDTH, CHOOSING_WINDOW_HEIGHT, 0);

    if(!window)
    {
        printf("Failed to initalize window: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    rend = SDL_CreateRenderer(window, -1, render_flags);
    if(!rend)
    {
        printf("Failed to initialize renderer: %s\n", SDL_GetError());
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
    
    for(int i = 0; i < imgCount; i++){
        numText[i] = SDL_CreateTextureFromSurface(rend, numImg[i]);
        SDL_FreeSurface(numImg[i]);
        if (!numText[i]){
            printf("Failed to texturize images\n");
            for(int j = i; j < imgCount; j++) SDL_FreeSurface(numImg[j]);
            SDL_DestroyRenderer(rend);
            SDL_DestroyWindow(window);
            SDL_Quit();
            return 1;
        }
    }


    SDL_Rect *xBackground = {0,0,CHOOSING_WINDOW_WIDTH,CHOOSING_WINDOW_HEIGHT};
    SDL_Rect xTargeted = {0,0,0,0};


    for(int i = 0; i < imgCount-notNum; i++){
        xNum[i].x = 0;
        xNum[i].y = 0;
    }
    int texWidth = CHOOSING_WINDOW_WIDTH/10, texHeight = CHOOSING_WINDOW_HEIGHT/10;
    for(int i = 0; i < imgCount-notNum; i++){//1 is background
        xNum[i].w = texWidth;
        xNum[i].h = texHeight;
        xNum[i].x = (i == 0 || i%5 == 0) ? texWidth/2: xNum[i-1].x+texWidth*2;
        xNum[i].y = i < 5 ? texHeight/2 : xNum[i-5].h + texHeight;
    }

    xTargeted.w = xNum[5].w*4/5;
    xTargeted.h = xNum[5].h*4/5;

    int request_quit = 0;
    int currLVL = 0;
    lvlCount = 10;
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
                request_quit = 1;
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
                    if(--currLVL < 0) currLVL = lvlCount-1;                    
                    break;
                case SDL_SCANCODE_D:
                case SDL_SCANCODE_RIGHT:
                    if(++currLVL > lvlCount) currLVL = 0; 
                    break;
                case SDL_SCANCODE_S:
                case SDL_SCANCODE_DOWN:
                    currLVL+=5;
                    if(currLVL > lvlCount) currLVL -= 5;
                    break;
                case SDL_SCANCODE_W:
                case SDL_SCANCODE_UP:
                    currLVL-=5;
                    if (currLVL < 0) currLVL += 5;
                    break;
                case SDL_SCANCODE_RETURN://aka enter
                    request_quit = 1;
                    choosedLVL = currLVL+1;
                }
            default:
            break;
            }
        }
    SDL_RenderClear(rend);

    SDL_RenderCopy(rend, numText[0], NULL, xBackground);
    
    for(int i = 0; i < lvlCount; i++){
        if(i == currLVL) {
            SDL_RenderCopy(rend, numText[2], NULL, &xNum[i]);
            xTargeted.x = xNum[i].x + xNum[i].w/10;
            xTargeted.y = xNum[i].y + xNum[i].h/10;
            SDL_RenderCopy(rend, numText[1], NULL, &xTargeted);
            SDL_RenderCopy(rend, numText[notNum + i], NULL, &xTargeted);
        } else {
            SDL_RenderCopy(rend, numText[1], NULL, &xNum[i]);
            SDL_RenderCopy(rend, numText[notNum + i], NULL, &xNum[i]);
        }
    }
    SDL_RenderPresent(rend);
    }

    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
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