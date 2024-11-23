#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h> 
#include <iostream>
#include <vector>

using namespace std;

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int SQUARE_SIZE = 20;
const int SNAKE_SPEED = 100;
const int BONUS_RADIUS = 2 * SQUARE_SIZE / 2;  // Radius for bonus food (double size)
const int REGULAR_RADIUS = SQUARE_SIZE / 2;  // Radius for regular food
int highScore = 0;

struct Segment {
    int x, y;
};

bool checkCollision(const Segment& a, const Segment& b) {
    return a.x == b.x && a.y == b.y;
}

void drawCircle(SDL_Renderer* renderer, int x, int y, int radius, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w; // horizontal offset
            int dy = radius - h; // vertical offset
            if ((dx * dx + dy * dy) <= (radius * radius)) {
                SDL_RenderDrawPoint(renderer, x + dx, y + dy);
            }
        }
    }
}

void renderText(SDL_Renderer* renderer, TTF_Font* font, const string& text, SDL_Color color, int x, int y) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    int textWidth = surface->w;
    int textHeight = surface->h;
    SDL_Rect dstRect = {x, y, textWidth, textHeight};

    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
    SDL_DestroyTexture(texture);
}

void showIntro(SDL_Renderer* renderer, TTF_Font* font, bool& gameStarted) {
    SDL_Color white = {255, 255, 255, 255};  // White color for text
    SDL_Event event;
    bool introActive = true;

    // Button dimensions
    SDL_Rect playButtonRect = { (SCREEN_WIDTH - 200) / 2, (SCREEN_HEIGHT - 200) / 2, 200, 50 }; // Play button
    SDL_Rect exitButtonRect = { (SCREEN_WIDTH - 200) / 2, (SCREEN_HEIGHT - 100) / 2, 200, 50 }; // Exit button
    SDL_Color buttonTextColor = {255, 255, 255, 255};  // White color for button text

    // Load the intro image
    SDL_Surface* surface = IMG_Load("image/cover.png");  // Load PNG image
    SDL_Texture* texture = nullptr;
    if (surface) {
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }

    // Initialize SDL_mixer
    Mix_Init(MIX_INIT_MP3);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

    // Load intro music
    Mix_Music* music = Mix_LoadMUS("sound/intro.mp3");  // Replace with your audio file path
    if (!music) {
        cout << "Failed to load intro music: " << Mix_GetError() << endl;
    } else {
        Mix_PlayMusic(music, -1);  // Play music indefinitely
    }

    while (introActive) {
        SDL_RenderClear(renderer);

        // Render intro image as the background
        if (texture) {
            SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        }

        // Render button text on top of the intro image
        TTF_Font* boldFont = TTF_OpenFont("font.ttf", 28);  // Increase font size
        renderText(renderer, boldFont, "Play Game", buttonTextColor, playButtonRect.x + 40, playButtonRect.y + 10);
        renderText(renderer, boldFont, "Exit", buttonTextColor, exitButtonRect.x + 70, exitButtonRect.y + 10);
        TTF_CloseFont(boldFont);

        SDL_RenderPresent(renderer);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                introActive = false;
                gameStarted = false;
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    int mouseX = event.button.x;
                    int mouseY = event.button.y;

                    // Check if the mouse click is within the Play button bounds
                    if (mouseX >= playButtonRect.x && mouseX <= playButtonRect.x + playButtonRect.w &&
                        mouseY >= playButtonRect.y && mouseY <= playButtonRect.y + playButtonRect.h) {
                        gameStarted = true;
                        introActive = false;
                    }
                    // Check if the mouse click is within the Exit button bounds
                    if (mouseX >= exitButtonRect.x && mouseX <= exitButtonRect.x + exitButtonRect.w &&
                        mouseY >= exitButtonRect.y && mouseY <= exitButtonRect.y + exitButtonRect.h) {
                        gameStarted = false;
                        introActive = false;
                        SDL_Quit(); // Exit the game
                        exit(0);  // Exit the application
                    }
                }
            }
        }
    }

    // Stop music and clean up
    if (music) {
        Mix_HaltMusic();
        Mix_FreeMusic(music);
    }
    Mix_CloseAudio();
    Mix_Quit();

    // Clean up the texture
    if (texture) {
        SDL_DestroyTexture(texture);
    }
}


void showGameOver(SDL_Renderer* renderer, TTF_Font* font, int score, bool& gameRestart) {
    SDL_Color white = {255, 255, 255, 255};  // White color for text
    SDL_Event event;
    bool gameOverActive = true;

    // Update high score if the current score is higher
    if (score > highScore) {
        highScore = score;
    }

    // Load the background image for the game over screen
    SDL_Surface* surface = IMG_Load("image/gameover.png");  // Load your game over image
    SDL_Texture* texture = nullptr;
    if (surface) {
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    } else {
        cout << "Failed to load game over image: " << IMG_GetError() << endl;
    }

    // Button dimensions
    SDL_Rect restartButtonRect = { (SCREEN_WIDTH - 220) / 2, (SCREEN_HEIGHT + 40) / 2, 200, 50 };  // Restart button position
    SDL_Rect exitButtonRect = { (SCREEN_WIDTH - 220) / 2, (SCREEN_HEIGHT + 140) / 2, 200, 50 };  // Exit button position
    SDL_Color buttonTextColor = {255, 255, 255, 255};  // White color for button text

    while (gameOverActive) {
        SDL_RenderClear(renderer);

        // Render the game over background image
        if (texture) {
            SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        }

        // Render game over text, high score, and buttons
        TTF_Font* boldFont = TTF_OpenFont("font.ttf", 28);  // Font size
        renderText(renderer, boldFont, "Score: " + to_string(score), white, (SCREEN_WIDTH - 120) / 2, (SCREEN_HEIGHT - 400) / 2);
        renderText(renderer, boldFont, "High Score: " + to_string(highScore), white, (SCREEN_WIDTH - 140) / 2, (SCREEN_HEIGHT - 340) / 2);
        renderText(renderer, boldFont, "Restart Game", buttonTextColor, restartButtonRect.x + 30, restartButtonRect.y + 10);
        renderText(renderer, boldFont, "Exit", buttonTextColor, exitButtonRect.x + 70, exitButtonRect.y + 10);
        TTF_CloseFont(boldFont);

        SDL_RenderPresent(renderer);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                gameOverActive = false;
                gameRestart = false;
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    int mouseX = event.button.x;
                    int mouseY = event.button.y;

                    // Check if the mouse click is within the Restart button bounds
                    if (mouseX >= restartButtonRect.x && mouseX <= restartButtonRect.x + restartButtonRect.w &&
                        mouseY >= restartButtonRect.y && mouseY <= restartButtonRect.y + restartButtonRect.h) {
                        gameRestart = true;
                        gameOverActive = false;
                    }
                    // Check if the mouse click is within the Exit button bounds
                    if (mouseX >= exitButtonRect.x && mouseX <= exitButtonRect.x + exitButtonRect.w &&
                        mouseY >= exitButtonRect.y && mouseY <= exitButtonRect.y + exitButtonRect.h) {
                        gameRestart = false;
                        gameOverActive = false;
                    }
                }
            }
        }
    }

    // Clean up the texture
    if (texture) {
        SDL_DestroyTexture(texture);
    }
}

// Function to load a texture from a file
SDL_Texture* loadTexture(SDL_Renderer* renderer, const string& filePath) {
    SDL_Surface* surface = IMG_Load(filePath.c_str());
    if (!surface) {
        cout << "Failed to load image: " << IMG_GetError() << endl;
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();

    SDL_Window* window = SDL_CreateWindow("Simple Snake Game",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          SCREEN_WIDTH,
                                          SCREEN_HEIGHT,
                                          SDL_WINDOW_SHOWN);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    TTF_Font* font = TTF_OpenFont("font.ttf", 20);  // Replace with the path to your font file

    SDL_Texture* snakeHeadTexture = loadTexture(renderer, "image/snake_head.png");
    if (!snakeHeadTexture) {
        cout << "Failed to load snake head texture!" << endl;
        return -1;
    }

    bool gameRestart = true;

    while (gameRestart) {
        bool gameStarted = false;
        showIntro(renderer, font, gameStarted);

        if (gameStarted) {
            vector<Segment> snake = {{SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2}};
            int dx = SQUARE_SIZE;
            int dy = 0;
            int score = 0;
            int foodEaten = 0;
            bool bonusActive = false;
            Segment bonus;
            Uint32 bonusStartTime = 0;
            auto generateFood = []() -> Segment {
                return {
                    rand() % ((SCREEN_WIDTH - 2 * SQUARE_SIZE) / SQUARE_SIZE) * SQUARE_SIZE + SQUARE_SIZE,
                    rand() % ((SCREEN_HEIGHT - 2 * SQUARE_SIZE) / SQUARE_SIZE) * SQUARE_SIZE + SQUARE_SIZE
                };
            };
            Segment food = generateFood();
            Uint32 lastMove = SDL_GetTicks();

            bool quit = false;
            bool paused = false;  // Pause state
            SDL_Event event;
            while (!quit) {
                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_QUIT) {
                        quit = true;
                    } else if (event.type == SDL_KEYDOWN) {
                        switch (event.key.keysym.sym) {
                            case SDLK_UP:
                                if (dy == 0 && !paused) {
                                    dx = 0;
                                    dy = -SQUARE_SIZE;
                                }
                                break;
                            case SDLK_DOWN:
                                if (dy == 0 && !paused) {
                                    dx = 0;
                                    dy = SQUARE_SIZE;
                                }
                                break;
                            case SDLK_LEFT:
                                if (dx == 0 && !paused) {
                                    dx = -SQUARE_SIZE;
                                    dy = 0;
                                }
                                break;
                            case SDLK_RIGHT:
                                if (dx == 0 && !paused) {
                                    dx = SQUARE_SIZE;
                                    dy = 0;
                                }
                                break;
                            case SDLK_SPACE:
                                paused = !paused;  // Toggle pause
                                break;
                        }
                    }
                }

                if (!paused && SDL_GetTicks() - lastMove > SNAKE_SPEED) {
                    lastMove = SDL_GetTicks();
                    Segment newHead = {snake[0].x + dx, snake[0].y + dy};
                    snake.insert(snake.begin(), newHead);

                    if (checkCollision(snake[0], food)) {
                        score += 5;
                        foodEaten++;
                        food = generateFood();
                        if (foodEaten % 5 == 0) {
                            bonus = generateFood();
                            bonusActive = true;
                            bonusStartTime = SDL_GetTicks();
                        }
                    } else {
                        snake.pop_back();
                    }

                    if (bonusActive && checkCollision(snake[0], bonus)) {
                        score += 25;
                        bonusActive = false;
                    }

                    if (bonusActive && SDL_GetTicks() - bonusStartTime > 5000) {
                        bonusActive = false;
                    }

                    for (size_t i = 1; i < snake.size(); i++) {
                        if (checkCollision(snake[0], snake[i])) {
                            quit = true;
                            break;
                        }
                    }

                    if (snake[0].x < SQUARE_SIZE || snake[0].x >= SCREEN_WIDTH - SQUARE_SIZE || 
                        snake[0].y < SQUARE_SIZE || snake[0].y >= SCREEN_HEIGHT - SQUARE_SIZE) {
                        quit = true;
                    }

                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                    SDL_RenderClear(renderer);

                    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
                    SDL_Rect topWall = {0, 0, SCREEN_WIDTH, SQUARE_SIZE};
                    SDL_Rect bottomWall = {0, SCREEN_HEIGHT - SQUARE_SIZE, SCREEN_WIDTH, SQUARE_SIZE};
                    SDL_Rect leftWall = {0, 0, SQUARE_SIZE, SCREEN_HEIGHT};
                    SDL_Rect rightWall = {SCREEN_WIDTH - SQUARE_SIZE, 0, SQUARE_SIZE, SCREEN_HEIGHT};
                    SDL_RenderFillRect(renderer, &topWall);
                    SDL_RenderFillRect(renderer, &bottomWall);
                    SDL_RenderFillRect(renderer, &leftWall);
                    SDL_RenderFillRect(renderer, &rightWall);

                    SDL_Color foodColor = {255, 255, 255, 255};
                    drawCircle(renderer, food.x + REGULAR_RADIUS, food.y + REGULAR_RADIUS, REGULAR_RADIUS, foodColor);

                    if (bonusActive) {
                        SDL_Color bonusColor = {255, 0, 0, 255};
                        drawCircle(renderer, bonus.x + BONUS_RADIUS, bonus.y + BONUS_RADIUS, BONUS_RADIUS, bonusColor);
                    }

                    // Render the snake's head with the image
                    SDL_Rect headRect = { snake[0].x, snake[0].y, SQUARE_SIZE, SQUARE_SIZE };
                    SDL_RenderCopy(renderer, snakeHeadTexture, nullptr, &headRect);

                    // Render the rest of the snake
                    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                    for (size_t i = 1; i < snake.size(); i++) {
                        drawCircle(renderer, snake[i].x + REGULAR_RADIUS, snake[i].y + REGULAR_RADIUS, REGULAR_RADIUS, {0, 255, 0, 255});
                    }

                    renderText(renderer, font, "Score: " + to_string(score), {255, 255, 255, 255}, 540, 0);

                    SDL_RenderPresent(renderer);
                }
            }

            showGameOver(renderer, font, score, gameRestart);
        }
    }

    SDL_DestroyTexture(snakeHeadTexture);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}