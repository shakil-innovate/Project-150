    #include <SDL2/SDL.h>
    #include <SDL2/SDL_image.h>
    #include <SDL2/SDL_ttf.h>
    #include <SDL2/SDL_mixer.h>
    #include <fstream> 
    #include <bits/stdc++.h>

    using namespace std;

    const int SCREEN_WIDTH = 640;
    const int SCREEN_HEIGHT = 480;
    const int SQUARE_SIZE = 20;
    const int SNAKE_SPEED = 120;
    const int BONUS_FOOD_SIZE = SQUARE_SIZE * 2;

    Mix_Chunk* gameOverSound = nullptr;
    Mix_Chunk* eatingSound = nullptr;
    Mix_Chunk* bonusSound = nullptr;

    SDL_Texture* appleTexture = nullptr;
    SDL_Texture* snakeHeadTexture = nullptr;

    bool bonusFoodActive = false;
    Uint32 bonusFoodStartTime = 0;
    const int BONUS_FOOD_DURATION = 4000;
    int foodCounter = 0; 
    SDL_Texture* bonusFoodTexture = nullptr;

    struct Segment {
        int x, y;
    };

   
    void suruKor(SDL_Window*& window, SDL_Renderer*& renderer, TTF_Font*& font, SDL_Texture*& appleTexture);
    void cobiDekha(SDL_Renderer* renderer, const char* imagePath, int displayTimeMs);
    void cleanupSDL(SDL_Window* window, SDL_Renderer* renderer, TTF_Font* font,SDL_Texture* appleTexturet);
    void firstCobiDekha(SDL_Renderer* renderer, TTF_Font* font);
    void sesCobiDekha(SDL_Renderer* renderer,TTF_Font* font,int score,int high_score);
    Segment khabarToiriKor();
    Segment bonusFood;
    void handleEvents(bool& quit, int& dx, int& dy);
    bool checkCollision(const Segment& a, const Segment& b);
    bool checkBonusCollision(const Segment& a, const Segment& b);
    void moveSnake(vector<Segment>& snake, int dx, int dy, Segment& food, bool& foodEaten, bool& quit, int& score);
    void renderGame(SDL_Renderer* renderer, const vector<Segment>& snake, const Segment& food, TTF_Font* font, int score,SDL_Texture* appleTexture);
    void renderText(SDL_Renderer* renderer, TTF_Font* font, const string& text, int x, int y);
    void saveHighScore(int highScore);
    int loadHighScore();

    int main(int argc, char* argv[]) 
    {
        SDL_Window* window = nullptr;
        SDL_Renderer* renderer = nullptr;
        TTF_Font* font = nullptr;

        suruKor(window, renderer, font,appleTexture);
        firstCobiDekha(renderer,font);

        bool quit = false;
        vector<Segment> snake = {{SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2}};
        int dx = SQUARE_SIZE;
        int dy = 0;

        Segment food = khabarToiriKor();
        Uint32 lastMove = SDL_GetTicks();
        int score = 0;

         int highScore = loadHighScore();
       

        while (!quit) {
            handleEvents(quit, dx, dy);

            if (SDL_GetTicks() - lastMove > SNAKE_SPEED) {
                lastMove = SDL_GetTicks();

                bool foodEaten = false;
                moveSnake(snake, dx, dy, food, foodEaten, quit, score);

                if (foodEaten) {
                    food = khabarToiriKor();
                }

                renderGame(renderer, snake, food, font, score,appleTexture);
            }

            if (quit) {

                 if (score > highScore) {
                highScore = score;
                saveHighScore(highScore);  
            }
            sesCobiDekha(renderer,font,score,highScore);
            }
        }

        cleanupSDL(window, renderer, font,appleTexture);
        return 0;
    }


    void suruKor(SDL_Window*& window, SDL_Renderer*& renderer, TTF_Font*& font, SDL_Texture*& appleTexture)
    {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << endl;
            exit(1);
    }

        if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
        {
            cout << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << endl;
            exit(1);
        }

        if (TTF_Init() == -1) 
        {
            cout << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << endl;
            exit(1);
        }

        if (Mix_Init(MIX_INIT_MP3) == 0) 
        {
        cout << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << endl;
        exit(1);
        }

       if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) == -1)
       {
        cout << "SDL_mixer could not open audio! SDL_mixer Error: " << Mix_GetError() << endl;
        exit(1);
       }

    gameOverSound = Mix_LoadWAV("sound/gameover.wav"); 
    if (!gameOverSound) {
        cout << "Error loading game over sound: " << Mix_GetError() << endl;
    }

    eatingSound = Mix_LoadWAV("sound/eating.wav");
     if (!eatingSound) 
    {
    cout << "Error loading eating sound: " << Mix_GetError() << endl;
    }

    bonusSound = Mix_LoadWAV("sound/bonus.wav");

        window = SDL_CreateWindow("Simple Snake Game",
                                SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED,
                                SCREEN_WIDTH,
                                SCREEN_HEIGHT,
                                SDL_WINDOW_SHOWN);

        if (!window)
         {
            cout << "Window could not be created! SDL_Error: " << SDL_GetError() << endl;
            exit(1);
         }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer) 
        {
            cout << "Renderer could not be created! SDL_Error: " << SDL_GetError() << endl;
            exit(1);
        }

        font = TTF_OpenFont("fonts/atop-font.ttf", 24); 
        if (!font) 
        {
            cout << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << endl;
            exit(1);
        }

        
    SDL_Surface* appleSurface = IMG_Load("image/apple.png");
    if (!appleSurface) {
        cout << "Failed to load apple image: " << IMG_GetError() << endl;
        exit(1);
    }
    appleTexture = SDL_CreateTextureFromSurface(renderer, appleSurface);
    SDL_FreeSurface(appleSurface);

    if (!appleTexture) 
    {
        cout << "Failed to create apple texture: " << SDL_GetError() << endl;
        exit(1);
    }

     SDL_Surface* snakeHeadSurface = IMG_Load("image/snake_head.png");
    if (!snakeHeadSurface) {
        cout << "Failed to load snake head image: " << IMG_GetError() << endl;
        exit(1);
    }
    snakeHeadTexture = SDL_CreateTextureFromSurface(renderer, snakeHeadSurface);
    SDL_FreeSurface(snakeHeadSurface);

    if (!snakeHeadTexture) {
        cout << "Failed to create snake head texture: " << SDL_GetError() << endl;
        exit(1);
    }

    SDL_Surface* bonusFoodSurface = IMG_Load("image/apple.png");
    if (!bonusFoodSurface) 
    {
        cout << "Failed to load bonus food image: " << IMG_GetError() << endl;
        exit(1);
    }
    bonusFoodTexture = SDL_CreateTextureFromSurface(renderer, bonusFoodSurface);
    SDL_FreeSurface(bonusFoodSurface);

    if (!bonusFoodTexture) 
    {
        cout << "Failed to create bonus food texture: " << SDL_GetError() << endl;
        exit(1);
    }
 }



    void cobiDekha(SDL_Renderer* renderer, const char* imagePath,int displayTimeMs)
    {
        SDL_Surface* surface = IMG_Load(imagePath);
        if (!surface) {
            cout << "Error: Could not load image " << imagePath << " " << IMG_GetError() << endl;
            return;
        }

        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);

        SDL_Delay(displayTimeMs);

        SDL_DestroyTexture(texture);
    }



    void firstCobiDekha(SDL_Renderer* renderer, TTF_Font* font) 
    {
    Mix_Music* music = Mix_LoadMUS("sound/intro.mp3");

    if (music == nullptr)
     {
         cout << "Error loading music: " << Mix_GetError() << endl;
     }
     else
     {
        Mix_PlayMusic(music, -1);  
     }

        cobiDekha(renderer, "image/cover.png", 0);

        
        SDL_Rect playButtonRect = {SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 +15, 200, 50};
        SDL_Rect quitButtonRect = {SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 +75, 200, 50}; 

        SDL_Color buttonColor = {0, 0,200, 255}; 
        SDL_Color textColor = {255, 255, 255, 255}; 

        bool introDone = false;
    
        while (!introDone) 
        {
            SDL_Event event;

            
            SDL_SetRenderDrawColor(renderer, buttonColor.r, buttonColor.g, buttonColor.b, buttonColor.a);
            SDL_RenderFillRect(renderer, &playButtonRect);
            renderText(renderer, font, "Play Game", playButtonRect.x + 50, playButtonRect.y + 10);

            SDL_RenderFillRect(renderer, &quitButtonRect);
            renderText(renderer, font, "Quit", quitButtonRect.x + 75, quitButtonRect.y + 10);

            SDL_RenderPresent(renderer);
        
            while (SDL_PollEvent(&event))
             {
                if (event.type == SDL_QUIT) 
                {
                    exit(0); 
                }

                
                if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
                {
                    int mouseX = event.button.x;
                    int mouseY = event.button.y;

                    
                    if (mouseX >= playButtonRect.x && mouseX <= playButtonRect.x + playButtonRect.w &&
                        mouseY >= playButtonRect.y && mouseY <= playButtonRect.y + playButtonRect.h)
                        {
                        introDone = true; 
                         Mix_HaltMusic();
                        }

                  
                    if (mouseX >= quitButtonRect.x && mouseX <= quitButtonRect.x + quitButtonRect.w &&
                        mouseY >= quitButtonRect.y && mouseY <= quitButtonRect.y + quitButtonRect.h) 
                        {
                        exit(0);
                       }
                }
            }
        }
    }



    void sesCobiDekha(SDL_Renderer* renderer,TTF_Font* font,int score,int high_score)
    {
        cobiDekha(renderer, "image/gameover.png", 0); 
        
        bool introDone = false;

        SDL_Rect RestartButtonRect = {SCREEN_WIDTH / 2 - 130, SCREEN_HEIGHT / 2 +110, 250, 50};
        SDL_Rect quitButtonRect = {SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 +170, 200, 50};
        
        SDL_Color buttonColor = {0, 0,0, 255}; 
        SDL_Color textColor = {255, 255, 255, 255}; 

         renderText(renderer, font, "Score: " + to_string(score), SCREEN_WIDTH / 2 - 70, 20);
         renderText(renderer, font, "High Score: " + to_string(high_score), SCREEN_WIDTH / 2 - 100, 70);

        
        while (!introDone) {
            SDL_Event event;

            SDL_SetRenderDrawColor(renderer, buttonColor.r, buttonColor.g, buttonColor.b, buttonColor.a);
            SDL_RenderFillRect(renderer, &RestartButtonRect);

            renderText(renderer, font, "Restart Game", RestartButtonRect.x + 50, RestartButtonRect.y + 10);

            SDL_RenderFillRect(renderer, &quitButtonRect);
            renderText(renderer, font, "Quit", quitButtonRect.x + 75, quitButtonRect.y + 10);

            SDL_RenderPresent(renderer);


            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) 
                {
                    introDone = true; 
                }
                
                if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
                {
                    int mouseX = event.button.x;
                    int mouseY = event.button.y;

                    if (mouseX >= RestartButtonRect.x && mouseX <= RestartButtonRect.x + RestartButtonRect.w &&
                        mouseY >= RestartButtonRect.y && mouseY <= RestartButtonRect.y + RestartButtonRect.h) {
                        introDone = true; 
                        main(0, nullptr);
                    }

                    if (mouseX >= quitButtonRect.x && mouseX <= quitButtonRect.x + quitButtonRect.w &&
                        mouseY >= quitButtonRect.y && mouseY <= quitButtonRect.y + quitButtonRect.h) 
                    {
                        exit(0);
                    }
                }

            }
        }
    }



    Segment khabarToiriKor() 
    {
        return {
            rand() % ((SCREEN_WIDTH - 4 * SQUARE_SIZE) / SQUARE_SIZE) * SQUARE_SIZE + SQUARE_SIZE,
            rand() % ((SCREEN_HEIGHT - 4 * SQUARE_SIZE) / SQUARE_SIZE) * SQUARE_SIZE + SQUARE_SIZE
        };
    }


    void handleEvents(bool& quit, int& dx, int& dy) 
    {
        SDL_Event event;
        while (SDL_PollEvent(&event)) 
        {
            if (event.type == SDL_QUIT) 
            {
                quit = true;
            } 
            else if (event.type == SDL_KEYDOWN) 
            {
                switch (event.key.keysym.sym)
                {
                    case SDLK_UP:
                        if (dy == 0) 
                        {
                            dx = 0;
                            dy = -SQUARE_SIZE;
                        }
                        break;
                    case SDLK_DOWN:
                        if (dy == 0)
                        {
                            dx = 0;
                            dy = SQUARE_SIZE;
                        }
                        break;
                    case SDLK_LEFT:
                        if (dx == 0) 
                        {
                            dx = -SQUARE_SIZE;
                            dy = 0;
                        }
                        break;
                    case SDLK_RIGHT:
                        if (dx == 0) 
                        {
                            dx = SQUARE_SIZE;
                            dy = 0;
                        }
                        break;
                }
            }
        }
    }


    bool checkCollision(const Segment& a, const Segment& b) 
    {
        return a.x == b.x && a.y == b.y;
    }

    bool checkBonusCollision(const Segment& a, const Segment& b)
     {
    const int BONUS_FOOD_SIZE = 20; 
    return abs(a.x - b.x) < BONUS_FOOD_SIZE && abs(a.y - b.y) < BONUS_FOOD_SIZE;
    }




    void moveSnake(vector<Segment>& snake, int dx, int dy, Segment& food, bool& foodEaten, bool& quit, int& score)
    {
        Segment newHead = {snake[0].x + dx, snake[0].y + dy};
        snake.insert(snake.begin(), newHead);

        
        if (checkCollision(snake[0], food)) 
        {
            foodEaten = true;
            score += 10; 
             foodCounter++;

            if (eatingSound) 
            {
            Mix_PlayChannel(-1, eatingSound, 0);
            }

           if (!bonusFoodActive && foodCounter % 5 == 0)
           {
            bonusFood = khabarToiriKor();
            Mix_PlayChannel(-1, bonusSound, 0);

            foodCounter=0;
            bonusFoodActive = true;
            bonusFoodStartTime = SDL_GetTicks();
           }

           if (bonusFoodActive && SDL_GetTicks() - bonusFoodStartTime > BONUS_FOOD_DURATION)
            {
              bonusFood = khabarToiriKor();
            Mix_PlayChannel(-1, bonusSound, 0);

            foodCounter=0;
            bonusFoodActive = true;
            bonusFoodStartTime = SDL_GetTicks();
            
           }
        } 

        else if (bonusFoodActive && checkBonusCollision(snake[0], bonusFood))
         {
        score += 50; 
        bonusFoodActive = false; 
        if (eatingSound) 
         {
            Mix_PlayChannel(-1, eatingSound, 0);
         }
        } 
        else
        {
            snake.pop_back();
        }

    
        for (size_t i = 1; i < snake.size(); i++) 
        {
            if (checkCollision(snake[0], snake[i])) 
            {
                quit = true;
                foodCounter=0;
                break;
            }
        }

      
        if (snake[0].x < SQUARE_SIZE || snake[0].x >= SCREEN_WIDTH - SQUARE_SIZE ||
            snake[0].y < SQUARE_SIZE || snake[0].y >= SCREEN_HEIGHT - SQUARE_SIZE) 
            {
            foodCounter=0;
            quit = true;
            }

        if (quit && gameOverSound) 
        {
        Mix_PlayChannel(-1, gameOverSound, 0); 
       }
    }

    void renderGame(SDL_Renderer* renderer, const vector<Segment>& snake, const Segment& food, TTF_Font* font, int score,SDL_Texture* appleTexture)
     {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        SDL_Rect walls[] = {
            {0, 0, SCREEN_WIDTH, SQUARE_SIZE},
            {0, SCREEN_HEIGHT - SQUARE_SIZE, SCREEN_WIDTH, SQUARE_SIZE},
            {0, 0, SQUARE_SIZE, SCREEN_HEIGHT},
            {SCREEN_WIDTH - SQUARE_SIZE, 0, SQUARE_SIZE, SCREEN_HEIGHT}
        };

        for (const auto& wall : walls) 
        {
            SDL_RenderFillRect(renderer, &wall);
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        SDL_Rect walls2[] = {
            {500, 80,100 , SQUARE_SIZE},
            {580, 80, SQUARE_SIZE, 300},
            {100, 80, SQUARE_SIZE, 300},
            {100, 380,80, SQUARE_SIZE}
        };

        for (const auto& wall : walls2) 
        {
            SDL_RenderFillRect(renderer, &wall);
        }

       
        SDL_Rect foodRect = {food.x, food.y, SQUARE_SIZE, SQUARE_SIZE};
       SDL_RenderCopy(renderer, appleTexture, nullptr, &foodRect);

         if (bonusFoodActive) 
         {
        SDL_Rect bonusFoodRect = {bonusFood.x, bonusFood.y, BONUS_FOOD_SIZE, BONUS_FOOD_SIZE};
        SDL_RenderCopy(renderer, bonusFoodTexture, nullptr, &bonusFoodRect);
        }


     SDL_Rect headRect = {snake[0].x, snake[0].y, SQUARE_SIZE, SQUARE_SIZE};
     SDL_RenderCopy(renderer, snakeHeadTexture, nullptr, &headRect);

   
SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); 


auto drawCircle = [&](int centerX, int centerY, int radius) {
    for (int y = -radius; y <= radius; ++y) {
        for (int x = -radius; x <= radius; ++x) {
            if (x * x + y * y <= radius * radius) {
                SDL_RenderDrawPoint(renderer, centerX + x, centerY + y);
            }
        }
    }
};


for (size_t i = 1; i < snake.size(); ++i) 
{
    drawCircle(snake[i].x + SQUARE_SIZE / 2, snake[i].y + SQUARE_SIZE / 2, SQUARE_SIZE / 2);
}
       
        renderText(renderer, font, "Score: " + to_string(score), 500, 0);

        SDL_RenderPresent(renderer);
 }

  
    void renderText(SDL_Renderer* renderer, TTF_Font* font, const string& text, int x, int y) {
        SDL_Color white = {255, 255, 255, 255};
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, text.c_str(), white);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

        SDL_Rect textRect = {x, y, textSurface->w, textSurface->h};
        SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);

        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
    }

    void saveHighScore(int highScore) 
    {
    std::ofstream outFile("highscore.txt"); 
    if (outFile.is_open()) {
        outFile << highScore; 
        outFile.close();  
    }
     else 
    {
        cout << "Error: Could not open highscore file for writing!" << endl;
    }
   }

int loadHighScore()
 {
    std::ifstream inFile("highscore.txt"); 
    int highScore = 0;

    if (inFile.is_open())
     {
        inFile >> highScore;  
        inFile.close(); 
     }

    return highScore; 
}


    void cleanupSDL(SDL_Window* window, SDL_Renderer* renderer, TTF_Font* font,SDL_Texture* appleTexturet) {
        TTF_CloseFont(font);
         SDL_DestroyTexture(appleTexture);
         SDL_DestroyTexture(snakeHeadTexture);
         SDL_DestroyTexture(bonusFoodTexture);
        Mix_FreeChunk(gameOverSound);
        Mix_FreeChunk(eatingSound);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        IMG_Quit();
        Mix_Quit(); 
        SDL_Quit();
    }