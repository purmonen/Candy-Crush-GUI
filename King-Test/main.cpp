
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "CandyCrush.hpp"
#include <chrono>
#include <SDL2/SDL.h>
#include <SDL2_image/SDL_image.h>
#include <SDL2_ttf/SDL_ttf.h>

struct GameEngine {
    
    std::unordered_map<CandyCrush::Cell, SDL_Surface*> cellImages;
    SDL_Surface* backgroundImage = nullptr;
    SDL_Window* window = nullptr;
    SDL_Surface* screenSurface = nullptr;
    
    
    CandyCrush game;
    const SDL_Rect drawArea = SDL_Rect{340,110,320,320};
    const int cellHeight = drawArea.h / (int)game.getGameBoard().rows;
    
    TTF_Font* Sans;
    const int cellWidth = drawArea.w / (int)game.getGameBoard().columns;
    
    int lastX = -1;
    int lastY = -1;
    
    const int windowWidth = 755;
    const int windowHeight = 600;
    
    
    SDL_Surface* surfaceForText(std::string text) {
        //this opens a font style and sets a size
        SDL_Color White = {255, 255, 255};  // this is the color in rgb format, maxing out all would give you the color white, and it will be your text's color
        SDL_Surface* surfaceMessage = TTF_RenderText_Solid(Sans, text.c_str(), White); // as TTF_RenderText_Solid could only be used on SDL_Surface then you have to create the surface first
        if (surfaceMessage == nullptr) {
            std::cout << "SDL_Surface Error: " << SDL_GetError() << std::endl;
        }
        return surfaceMessage;
    }
    
    GameEngine() {
        if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
            printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
            throw;
        }
        
        if (TTF_Init() < 0) {
            printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
            throw;
        }
        
        window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, windowWidth, windowHeight, SDL_WINDOW_SHOWN);
        if( window == nullptr) {
            printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
            throw;
        }
        Sans = TTF_OpenFont("/Library/Fonts/Arial.ttf", 54);
        screenSurface = SDL_GetWindowSurface( window );
        
        // Load assets
        cellImages = {
            {CandyCrush::Blue, IMG_Load("assets/Blue.png")},
            {CandyCrush::Green, IMG_Load("assets/Green.png")},
            {CandyCrush::Red, IMG_Load("assets/Red.png")},
            {CandyCrush::Purple, IMG_Load("assets/Purple.png")},
            {CandyCrush::Yellow, IMG_Load("assets/Yellow.png")},
        };
        backgroundImage = IMG_Load("assets/BackGround.jpg");
    }
    
    ~GameEngine() {
        for (auto cellImage: cellImages) {
            SDL_FreeSurface(cellImage.second);
        }
        SDL_FreeSurface(backgroundImage);
        if (timeLeftLabel != nullptr) {
            SDL_FreeSurface(timeLeftLabel);
        }
        if (scoreLabel != nullptr) {
            SDL_FreeSurface(scoreLabel);
        }
    }
    
    SDL_Surface* timeLeftLabel = nullptr;
    SDL_Surface* scoreLabel = nullptr;
    
    SDL_Rect rectForCellPosition(GameBoard::CellPosition cellPosition, const SDL_Surface * image) {
        return SDL_Rect{cellWidth*cellPosition.column+cellWidth/2-image->w/2 + drawArea.x, cellHeight*cellPosition.row+cellHeight/2-image->h/2+drawArea.y, cellWidth, cellHeight};
    }
    
    SDL_Rect rectForCellPosition(int row, int column, const SDL_Surface * image) {
        return SDL_Rect{cellWidth*column+cellWidth/2-image->w/2 + drawArea.x, cellHeight*row+cellHeight/2-image->h/2+drawArea.y, cellWidth, cellHeight};
    }
    
    void render(CandyCrushGameBoardChange& gameBoardChange) {
        auto finishedRendering = false;
        auto distance = 0;
        while (!finishedRendering){
            SDL_BlitSurface(backgroundImage, NULL, screenSurface, NULL );
            finishedRendering = true;
            if (scoreLabel != nullptr) {
                SDL_FreeSurface(scoreLabel);
            }
            scoreLabel = surfaceForText("Score: " + std::to_string(game.getScore()));
            auto scoreLabelRect = SDL_Rect{0,0,100,100};
            SDL_BlitSurface(scoreLabel, NULL, screenSurface, &scoreLabelRect);
            if (timeLeftLabel != nullptr) {
                SDL_FreeSurface(timeLeftLabel);
            }
            timeLeftLabel = surfaceForText(std::to_string(game.numberOfSecondsLeft()));
            auto timeLeftLabelRect = SDL_Rect{80,430,100,100};
            SDL_BlitSurface(timeLeftLabel, NULL, screenSurface, &timeLeftLabelRect);
            
            for (auto row = 0; row < game.getGameBoard().rows; row++) {
                for (auto column = 0; column < game.getGameBoard().columns; column++) {
                    auto to = GameBoard::CellPosition(row, column);
                    auto from = gameBoardChange.gameBoardChange[to].first;
                    auto cell = gameBoardChange.gameBoardChange[to].second;
                    
                    auto image = cellImages[cell];
                    
                    auto fromDestination = rectForCellPosition(from, image);
                    auto toDestination = rectForCellPosition(to, image);
                    
                    
                    if (toDestination.x > fromDestination.x) {
                        fromDestination.x = std::min(fromDestination.x + distance, toDestination.x);
                    } else {
                        fromDestination.x = std::max(fromDestination.x - distance, toDestination.x);
                    }
                    
                    if (toDestination.y > fromDestination.y) {
                        fromDestination.y = std::min(fromDestination.y + distance, toDestination.y);
                    } else {
                        fromDestination.y = std::max(fromDestination.y - distance, toDestination.y);
                    }
                    
                    if (fromDestination.x != toDestination.x || fromDestination.y != toDestination.y) {
                        finishedRendering = false;
                    }
                    
                    SDL_BlitSurface( image, NULL, screenSurface, &fromDestination );
                }
            }
            SDL_UpdateWindowSurface(window);
            if (finishedRendering) {
                SDL_Delay(200);
            } else {
                SDL_Delay(17);
            }
            distance += 3;
        }
    }
    
    GameBoard::CellPosition cellPositionFromCoordinates(int x, int y) const {
        return GameBoard::CellPosition((y-drawArea.y)/cellHeight, (x-drawArea.x)/cellWidth);
    }
    
    void run() {
        bool quit = false;
        SDL_Event e;
        bool isMouseDown = false;
        
        
        auto lamm = [&](CandyCrushGameBoardChange gameBoardChange) {
            render(gameBoardChange);
        };
        auto numberOfSecondsLeft = -1;
        while( !quit ) {
            
            if (game.numberOfSecondsLeft() != numberOfSecondsLeft) {
                numberOfSecondsLeft = game.numberOfSecondsLeft();
                auto gameBoardChange = CandyCrushGameBoardChange(game);
                render(gameBoardChange);
            }
            
            while( SDL_PollEvent( &e ) != 0 ) {
                
                
                if( e.type == SDL_QUIT ) {
                    quit = true;
                }
                
                if (e.type == SDL_MOUSEBUTTONDOWN){
                    isMouseDown = true;
                    int x, y;
                    SDL_GetMouseState(&x, &y);
                    
                    auto move = GameBoard::CellSwapMove(cellPositionFromCoordinates(x, y), cellPositionFromCoordinates(lastX, lastY));
                    
                    std::cout << move << std::endl;
                    
                    if (game.getGameBoard().areCellsAdjacent(move.from, move.to)) {
                        game.play(move, lamm);
                        lastX = -1;
                        lastY = -1;
                    } else {
                        std::cout << "Could not make the move" << std::endl;
                        lastX = x;
                        lastY = y;
                    }
                }
                
                if (e.type == SDL_MOUSEBUTTONUP && lastX != -1) {
                    isMouseDown = false;
                }
                
                if (e.type == SDL_MOUSEMOTION && isMouseDown) {
                    int x, y;
                    SDL_GetMouseState(&x, &y);
                    auto move = GameBoard::CellSwapMove(cellPositionFromCoordinates(x, y), cellPositionFromCoordinates(lastX, lastY));
                    if (game.getGameBoard().areCellsAdjacent(move.from, move.to)) {
                        game.play(move, lamm);
                        lastX = -1;
                        lastY = -1;
                    }
                }
            }
            SDL_Delay(10);
        }
    }
};


#include <array>
int main( int argc, char* args[] )
{
    GameEngine gameEngine;
    gameEngine.run();
    return 0;
}


