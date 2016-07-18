
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
    
    std::unordered_map<CandyCrush::Cell, SDL_Texture*> cellImages;
    SDL_Texture* backgroundImage = nullptr;
    SDL_Window* window = nullptr;
    SDL_Renderer * renderer;
    
    CandyCrush game;
    const SDL_Rect drawArea = SDL_Rect{340,110,320,320};
    const int cellHeight = drawArea.h / (int)game.getGameBoard().rows;
    
    TTF_Font* scoreLabelFont;
    TTF_Font* timeLeftLabelFont;
    const int cellWidth = drawArea.w / (int)game.getGameBoard().columns;
    
    int lastX = -1;
    int lastY = -1;
    
    const int windowWidth = 755;
    const int windowHeight = 600;
    
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
        scoreLabelFont = TTF_OpenFont("/Library/Fonts/Phosphate.ttc", 44);
        if (!scoreLabelFont) {
            throw;
        }
        
        timeLeftLabelFont = TTF_OpenFont("/Library/Fonts/Phosphate.ttc", 54);
        if (!timeLeftLabelFont) {
            throw;
        }
        
        renderer = SDL_CreateRenderer(window, -1, 0);

        // Load assets
        cellImages = {
            {CandyCrush::Blue, SDL_CreateTextureFromSurface(renderer, IMG_Load("assets/Blue.png"))},
            {CandyCrush::Green, SDL_CreateTextureFromSurface(renderer, IMG_Load("assets/Green.png"))},
            {CandyCrush::Red, SDL_CreateTextureFromSurface(renderer, IMG_Load("assets/Red.png"))},
            {CandyCrush::Purple, SDL_CreateTextureFromSurface(renderer, IMG_Load("assets/Purple.png"))},
            {CandyCrush::Yellow, SDL_CreateTextureFromSurface(renderer, IMG_Load("assets/Yellow.png"))},
        };
        
        backgroundImage = SDL_CreateTextureFromSurface(renderer, IMG_Load("assets/BackGround.jpg"));
    }
    
    ~GameEngine() {
        for (auto cellImage: cellImages) {
            SDL_DestroyTexture(cellImage.second);
        }
        SDL_DestroyTexture(backgroundImage);
    }
    
    SDL_Surface* timeLeftLabel = nullptr;
    SDL_Surface* scoreLabel = nullptr;
    
    SDL_Rect rectForCellPosition(const GameBoard::CellPosition& cellPosition, SDL_Texture * image) {
        int w, h;
        SDL_QueryTexture(image, NULL, NULL, &w, &h);
        return SDL_Rect{cellWidth*cellPosition.column+cellWidth/2-w/2 + drawArea.x, cellHeight*cellPosition.row+cellHeight/2-h/2+drawArea.y, w, h};
    }
    
    void render(CandyCrushGameBoardChange& gameBoardChange) {
        auto startTime = std::chrono::high_resolution_clock::now();
        auto finishedRendering = false;
        auto distance = 0;
        while (!finishedRendering){
            finishedRendering = true;
            SDL_RenderClear(renderer);
        
            // Render background image
            SDL_RenderCopy(renderer, backgroundImage, NULL, NULL);
            
            SDL_Color White = {255, 255, 255};
            
            // Render score label
            auto scoreText = "Score: " + std::to_string(game.getScore());
            SDL_Surface* scoreLabel = TTF_RenderText_Solid(scoreLabelFont, scoreText.c_str(), White);
            SDL_Texture* scoreLabelTexture = SDL_CreateTextureFromSurface( renderer, scoreLabel );
            auto scoreLabelRect = SDL_Rect{0,0,scoreLabel->w,scoreLabel->h};
            SDL_RenderCopy(renderer, scoreLabelTexture, NULL, &scoreLabelRect);
            SDL_DestroyTexture(scoreLabelTexture);
            
            // Render time left label
            auto timeLeftText = std::to_string(game.numberOfSecondsLeft());
            SDL_Surface* timeLeftLabel = TTF_RenderText_Solid(scoreLabelFont, timeLeftText.c_str(), White);
            auto timeLeftLabelRect = SDL_Rect{80,430,timeLeftLabel->w,timeLeftLabel->h};
            SDL_Texture* timeLeftTexture = SDL_CreateTextureFromSurface( renderer, timeLeftLabel );
            SDL_RenderCopy(renderer, timeLeftTexture, NULL, &timeLeftLabelRect);
            SDL_DestroyTexture(timeLeftTexture);
            
            // Render rectangle around selected cell
                auto selectedCell = cellPositionFromCoordinates(lastX, lastY);
            if (game.getGameBoard().isCellValid(selectedCell)) {
                auto selectedCellRect = rectForCellPosition(selectedCell, cellImages[CandyCrush::Blue]);
                SDL_SetRenderDrawColor(renderer, 255, 80, 80, 1);
                SDL_RenderDrawRect(renderer, &selectedCellRect);
            }
            
            
            for (auto removedCell: gameBoardChange.removedCells) {
                auto image = cellImages[removedCell.second];
                auto fromDestination = rectForCellPosition(removedCell.first, image);
                
                SDL_SetRenderDrawColor(renderer, 255, 80, 80, 1);
//                SDL_RenderFillRect(renderer, &fromDestination);
                
                fromDestination.x += distance/2;
                fromDestination.y += distance/2;
                fromDestination.w -= distance;
                fromDestination.h -= distance;
                
                SDL_RenderCopy(renderer, image, nullptr, &fromDestination);
            }
            
            // Render game board
            for (auto row = 0; row < game.getGameBoard().rows; row++) {
                for (auto column = 0; column < game.getGameBoard().columns; column++) {
                    auto to = GameBoard::CellPosition(row, column);
                    auto from = gameBoardChange.gameBoardChange[to].first;
                    auto cell = gameBoardChange.gameBoardChange[to].second;
                    
                    const auto& image = cellImages[cell];
                    
                    
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
                    
                    // Handle animation from over the board
                    auto cutoff = std::max(drawArea.y-fromDestination.y, 0);
                    int w, h;
                    SDL_QueryTexture(image, NULL, NULL, &w, &h);
                    SDL_Rect srcRect = {0,cutoff,w,h-cutoff};
                    
                    if (fromDestination.y < drawArea.y) {
                        fromDestination.y = drawArea.y;
                        fromDestination.h -= cutoff;
                    }
                    SDL_RenderCopy(renderer, image, &srcRect, &fromDestination);
                }
            }
            
            SDL_RenderPresent(renderer);
            if (finishedRendering) {
//                SDL_Delay(200);
            } else {
                SDL_Delay(5);
            }
            distance += 1;
        }
        auto currentTime = std::chrono::high_resolution_clock::now();
        int numberOfMilliSecondsElapsed = (int)std::chrono::duration_cast<std::chrono::milliseconds>(currentTime-startTime).count();
        std::cout << "Rendering took: " << numberOfMilliSecondsElapsed << "ms" << std::endl;
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
                        lastX = -1;
                        lastY = -1;
                        game.play(move, lamm);

                    } else {
                        std::cout << "Could not make the move" << std::endl;
                        lastX = x;
                        lastY = y;
                    }
                    auto gameBoardChange = CandyCrushGameBoardChange(game);
                    render(gameBoardChange);
                }
                
                if (e.type == SDL_MOUSEBUTTONUP && lastX != -1) {
                    isMouseDown = false;
                }
                
                if (e.type == SDL_MOUSEMOTION && isMouseDown) {
                    int x, y;
                    SDL_GetMouseState(&x, &y);
                    auto move = GameBoard::CellSwapMove(cellPositionFromCoordinates(x, y), cellPositionFromCoordinates(lastX, lastY));
                    if (game.getGameBoard().areCellsAdjacent(move.from, move.to)) {
                        lastX = -1;
                        lastY = -1;
                        game.play(move, lamm);
                        auto gameBoardChange = CandyCrushGameBoardChange(game);
                        render(gameBoardChange);
                    }
                }
                
            }
            SDL_Delay(1);
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


