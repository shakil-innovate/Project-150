#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <bits/stdc++.h>

using namespace std;

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int SQUARE_SIZE = 20;
const int SNAKE_SPEED = 100;

struct Segment {
    int x, y;
};

// Function prototypes
void initializeSDL(SDL_Window*& window, SDL_Renderer*& renderer, TTF_Font*& font);
void showImage(SDL_Renderer* renderer, const char* imagePath, int displayTimeMs);
void cleanupSDL(SDL_Window* window, SDL_Renderer* renderer, TTF_Font* font);
void renderIntro(SDL_Renderer* renderer, TTF_Font* font);
void renderEnd(SDL_Renderer* renderer);
Segment generateFood();
void handleEvents(bool& quit, int& dx, int& dy);
bool checkCollision(const Segment& a, const Segment& b);
void moveSnake(vector<Segment>& snake, int dx, int dy, Segment& food, bool& foodEaten, bool& quit, int& score);
void renderGame(SDL_Renderer* renderer, const vector<Segment>& snake, const Segment& food, TTF_Font* font, int score);
void renderText(SDL_Renderer* renderer, TTF_Font* font, const string& text, int x, int y);

int main(int argc, char* argv[]) 
{
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    TTF_Font* font = nullptr;

    initializeSDL(window, renderer, font);
    renderIntro(renderer,font);

    bool quit = false;
    vector<Segment> snake = {{SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2}};
    int dx = SQUARE_SIZE;
    int dy = 0;

    Segment food = generateFood();
    Uint32 lastMove = SDL_GetTicks();
    int score = 0;

    while (!quit) {
        handleEvents(quit, dx, dy);

        if (SDL_GetTicks() - lastMove > SNAKE_SPEED) {
            lastMove = SDL_GetTicks();

            bool foodEaten = false;
            moveSnake(snake, dx, dy, food, foodEaten, quit, score);

            if (foodEaten) {
                food = generateFood();
            }

            renderGame(renderer, snake, food, font, score);
        }

        if (quit) {
           renderEnd(renderer);
        }
    }

    cleanupSDL(window, renderer, font);
    return 0;
}


void initializeSDL(SDL_Window*& window, SDL_Renderer*& renderer, TTF_Font*& font)
   {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << endl;
        exit(1);
   }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
    {
        cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << endl;
        exit(1);
    }

    if (TTF_Init() == -1) 
    {
        cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << endl;
        exit(1);
    }

    window = SDL_CreateWindow("Simple Snake Game",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              SCREEN_WIDTH,
                              SCREEN_HEIGHT,
                              SDL_WINDOW_SHOWN);

    if (!window) {
        cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << endl;
        exit(1);
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) 
    {
        cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << endl;
        exit(1);
    }

    font = TTF_OpenFont("fonts/atop-font.ttf", 24); 
    if (!font) 
    {
        cerr << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << endl;
        exit(1);
    }
}


void showImage(SDL_Renderer* renderer, const char* imagePath, int displayTimeMs) 
   {
    SDL_Surface* surface = IMG_Load(imagePath);
    if (!surface) {
        cerr << "Error: Could not load image " << imagePath << " " << IMG_GetError() << endl;
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


void cleanupSDL(SDL_Window* window, SDL_Renderer* renderer, TTF_Font* font) {
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

// Display intro image and wait for any key press
void renderIntro(SDL_Renderer* renderer, TTF_Font* font)
 {
    
    //intro image
    showImage(renderer,"image/cover.png",0);

   //define button with it's position
    SDL_Rect buttonRect = {SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 25, 200, 50};
    SDL_Color buttonColor = {0, 128, 255, 255};
    SDL_Color textColor = {255, 255, 255, 255};

    bool introDone = false;
 
    while (!introDone) 
    {
        SDL_Event event;

        // color the button
        SDL_SetRenderDrawColor(renderer, buttonColor.r, buttonColor.g, buttonColor.b, buttonColor.a);
        SDL_RenderFillRect(renderer, &buttonRect);

        // Render "Play Game" text
        renderText(renderer, font, "Play Game", buttonRect.x + 50, buttonRect.y + 10);

        SDL_RenderPresent(renderer);

        // Handle events
        while (SDL_PollEvent(&event)) 
        {
            if (event.type == SDL_QUIT) 
            {
                exit(0); // i want to quit the game
            }

            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
             {
                int mouseX = event.button.x;
                int mouseY = event.button.y;

                // Check if the button was clicked
                if (mouseX >= buttonRect.x && mouseX <= buttonRect.x + buttonRect.w &&
                    mouseY >= buttonRect.y && mouseY <= buttonRect.y + buttonRect.h) 
                {
                    introDone = true; // Exit the
                }
            }
        }
    }
}



// Display intro image and wait for any key press
void renderEnd(SDL_Renderer* renderer) {
    showImage(renderer, "image/gameover.png", 0); // Initially show the intro image for 5 seconds
    bool introDone = false;

    // Keep the intro screen until any key is pressed
    while (!introDone) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                introDone = true; // Exit if the window is closed
            }
            if (event.type == SDL_KEYDOWN) {
                introDone = true; // Exit when any key is pressed
            }
        }
    }
}


// Generate random food position
Segment generateFood() {
    return {
        rand() % ((SCREEN_WIDTH - 2 * SQUARE_SIZE) / SQUARE_SIZE) * SQUARE_SIZE + SQUARE_SIZE,
        rand() % ((SCREEN_HEIGHT - 2 * SQUARE_SIZE) / SQUARE_SIZE) * SQUARE_SIZE + SQUARE_SIZE
    };
}


// Handle user input
void handleEvents(bool& quit, int& dx, int& dy) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            quit = true;
        } else if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_UP:
                    if (dy == 0) {
                        dx = 0;
                        dy = -SQUARE_SIZE;
                    }
                    break;
                case SDLK_DOWN:
                    if (dy == 0) {
                        dx = 0;
                        dy = SQUARE_SIZE;
                    }
                    break;
                case SDLK_LEFT:
                    if (dx == 0) {
                        dx = -SQUARE_SIZE;
                        dy = 0;
                    }
                    break;
                case SDLK_RIGHT:
                    if (dx == 0) {
                        dx = SQUARE_SIZE;
                        dy = 0;
                    }
                    break;
            }
        }
    }
}

// Check if two segments collide
bool checkCollision(const Segment& a, const Segment& b) {
    return a.x == b.x && a.y == b.y;
}

// Move the snake and check for collisions
void moveSnake(vector<Segment>& snake, int dx, int dy, Segment& food, bool& foodEaten, bool& quit, int& score) {
    Segment newHead = {snake[0].x + dx, snake[0].y + dy};
    snake.insert(snake.begin(), newHead);

    // Check collision with food
    if (checkCollision(snake[0], food)) {
        foodEaten = true;
        score += 10; // Increment score
    } else {
        snake.pop_back();
    }

    // Check collision with itself
    for (size_t i = 1; i < snake.size(); i++) {
        if (checkCollision(snake[0], snake[i])) {
            quit = true;
            break;
        }
    }

    // Check collision with walls
    if (snake[0].x < SQUARE_SIZE || snake[0].x >= SCREEN_WIDTH - SQUARE_SIZE ||
        snake[0].y < SQUARE_SIZE || snake[0].y >= SCREEN_HEIGHT - SQUARE_SIZE) {
        quit = true;
    }
}

// Render the game scene
void renderGame(SDL_Renderer* renderer, const vector<Segment>& snake, const Segment& food, TTF_Font* font, int score) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Draw walls
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    SDL_Rect walls[] = {
        {0, 0, SCREEN_WIDTH, SQUARE_SIZE},
        {0, SCREEN_HEIGHT - SQUARE_SIZE, SCREEN_WIDTH, SQUARE_SIZE},
        {0, 0, SQUARE_SIZE, SCREEN_HEIGHT},
        {SCREEN_WIDTH - SQUARE_SIZE, 0, SQUARE_SIZE, SCREEN_HEIGHT}
    };
    for (const auto& wall : walls) {
        SDL_RenderFillRect(renderer, &wall);
    }

    // Draw food
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_Rect foodRect = {food.x, food.y, SQUARE_SIZE, SQUARE_SIZE};
    SDL_RenderFillRect(renderer, &foodRect);

    // Draw snake
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    for (const auto& segment : snake) {
        SDL_Rect snakeRect = {segment.x, segment.y, SQUARE_SIZE, SQUARE_SIZE};
        SDL_RenderFillRect(renderer, &snakeRect);
    }

    // Render the score
    renderText(renderer, font, "Score: " + to_string(score), 500, 0);

    SDL_RenderPresent(renderer);
}

// Render text on the screen
void renderText(SDL_Renderer* renderer, TTF_Font* font, const string& text, int x, int y) {
    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, text.c_str(), white);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

    SDL_Rect textRect = {x, y, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}