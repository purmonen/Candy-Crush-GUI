
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
    CandyCrush game;
    
    bool isFirstGame = true;
    
    // The area of the window where the game board is displayed
    const SDL_Rect gameBoardRect = SDL_Rect{340,110,320,320};
    
    int lastMouseDownX = -1;
    int lastMouseDownY = -1;
    
    const int windowWidth = 755;
    const int windowHeight = 600;
    
    const int cellHeight = gameBoardRect.h / (int)game.getGameBoard().rows;
    const int cellWidth = gameBoardRect.w / (int)game.getGameBoard().columns;
    
    SDL_Window* window = nullptr;
    SDL_Renderer * renderer = nullptr;
    
    std::unordered_map<CandyCrush::Cell, SDL_Texture*> cellTextures;
    SDL_Texture* backgroundTexture = nullptr;
    
    TTF_Font* scoreLabelFont = nullptr;
    TTF_Font* timeLeftLabelFont = nullptr;
    
    
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
        scoreLabelFont = TTF_OpenFont("/Library/Fonts/Phosphate.ttc", 40);
        if (!scoreLabelFont) {
            throw;
        }
        
        timeLeftLabelFont = TTF_OpenFont("/Library/Fonts/Phosphate.ttc", 74);
        if (!timeLeftLabelFont) {
            throw;
        }
        
        renderer = SDL_CreateRenderer(window, -1, 0);
        
        // Load assets
        cellTextures = {
            {CandyCrush::Blue, SDL_CreateTextureFromSurface(renderer, IMG_Load("assets/Blue.png"))},
            {CandyCrush::Green, SDL_CreateTextureFromSurface(renderer, IMG_Load("assets/Green.png"))},
            {CandyCrush::Red, SDL_CreateTextureFromSurface(renderer, IMG_Load("assets/Red.png"))},
            {CandyCrush::Purple, SDL_CreateTextureFromSurface(renderer, IMG_Load("assets/Purple.png"))},
            {CandyCrush::Yellow, SDL_CreateTextureFromSurface(renderer, IMG_Load("assets/Yellow.png"))},
        };
        
        backgroundTexture = SDL_CreateTextureFromSurface(renderer, IMG_Load("assets/BackGround.jpg"));
    }
    
    
    ~GameEngine() {
        for (auto cellImage: cellTextures) {
            SDL_DestroyTexture(cellImage.second);
        }
        SDL_DestroyTexture(backgroundTexture);
    }
    
    
    SDL_Rect rectForCellPosition(const GameBoard::CellPosition& cellPosition, SDL_Texture * image) {
        int w, h;
        SDL_QueryTexture(image, NULL, NULL, &w, &h);
        return SDL_Rect{cellWidth*cellPosition.column+cellWidth/2-w/2 + gameBoardRect.x, cellHeight*cellPosition.row+cellHeight/2-h/2+gameBoardRect.y, w, h};
    }
    
    
    void renderText(std::string text, int x, int y, TTF_Font *font) {
        SDL_Color whiteColor = {255, 255, 255};
        SDL_Surface* label = TTF_RenderText_Solid(font, text.c_str(), whiteColor);
        auto labelRect = SDL_Rect{x, y, label->w,label->h};
        SDL_Texture* labelTexture = SDL_CreateTextureFromSurface( renderer, label );
        SDL_RenderCopy(renderer, labelTexture, NULL, &labelRect);
        SDL_DestroyTexture(labelTexture);
    }
    
    
    void renderScore() {
        renderText("Score: " + std::to_string(game.getScore()), 20, 20, scoreLabelFont);
    }
    
    
    void renderBackground() {
        SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);
    }
    
    
    void renderGameOver() {
        
        // All cells will be hidden one by one in a spiral fashion
        bool isCellVisible[game.getGameBoard().rows][game.getGameBoard().columns];
        for (auto row = 0; row < game.getGameBoard().rows; row++) {
            for (auto column = 0; column < game.getGameBoard().columns; column++) {
                isCellVisible[row][column] = true;
            }
        }
        
        // Animates hiding of the selected cell and hides all previously hidden cells
        auto renderSpiral = [&](int selectedRow, int selectedColumn) {
            for (int distance = 0; distance < cellWidth; distance += 4) {
                SDL_RenderClear(renderer);
                SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);
                renderScore();
                for (auto row = 0; row < game.getGameBoard().rows; row++) {
                    for (auto column = 0; column < game.getGameBoard().columns; column++) {
                        auto cell = game.getGameBoard()[row][column];
                        auto image = cellTextures[cell];
                        auto fromDestination = rectForCellPosition(GameBoard::CellPosition(row, column), image);
                        if (row == selectedRow && column == selectedColumn) {
                            fromDestination.x += distance/2;
                            fromDestination.y += distance/2;
                            fromDestination.w -= distance;
                            fromDestination.h -= distance;
                            isCellVisible[row][column] = false;
                            SDL_RenderCopy(renderer, image, nullptr, &fromDestination);
                        } else if (isCellVisible[row][column]) {
                            SDL_RenderCopy(renderer, image, nullptr, &fromDestination);
                        }
                        
                    }
                }
                SDL_RenderPresent(renderer);
                //                    SDL_Delay(50);
                
            }
        };
        
        // Spiral iteration over game board :)
        int depth = 0;
        while (depth <= game.getGameBoard().columns/2) {
            int topRow = depth;
            for (auto selectedColumn = depth; selectedColumn < game.getGameBoard().columns-depth; selectedColumn++) {
                renderSpiral(topRow, selectedColumn);
            }
            
            int rightColumn = (int)game.getGameBoard().columns-1-depth;
            for (int selectedRow = depth+1; selectedRow < game.getGameBoard().rows-1-depth; selectedRow++) {
                renderSpiral(selectedRow, rightColumn);
            }
            
            
            int bottomRow = (int)game.getGameBoard().rows - depth - 1;
            if (topRow != bottomRow) {
                for (int selectedColumn = (int)game.getGameBoard().columns-1-depth; selectedColumn >= depth; selectedColumn--) {
                    renderSpiral(bottomRow, selectedColumn);
                }
            }
            
            int leftColumn = depth;
            if (leftColumn != rightColumn) {
                for (int selectedRow = (int)game.getGameBoard().rows-2-depth; selectedRow >= depth+1; selectedRow--) {
                    renderSpiral(selectedRow, leftColumn);
                }
            }
            depth++;
        }
        
        SDL_Delay(500);
        
        renderBackground();
        renderText("GAME OVER", 390, 250, scoreLabelFont);
        renderScore();
    }
    
    
    void renderGameBoard(CandyCrushGameBoardChange& gameBoardChange, int move = 1) {
        auto startTime = std::chrono::high_resolution_clock::now();
        auto finishedRendering = false;
        auto distance = 0;

    

            while (!finishedRendering){
                finishedRendering = true;
                SDL_RenderClear(renderer);
                
                renderBackground();
                renderScore();
                renderText(std::to_string(game.numberOfSecondsLeft()), 80, 415, timeLeftLabelFont);
                
                // Render rectangle around selected cell
                auto selectedCell = cellPositionFromCoordinates(lastMouseDownX, lastMouseDownY);
                if (game.getGameBoard().isCellValid(selectedCell)) {
                    auto selectedCellRect = rectForCellPosition(selectedCell, cellTextures[CandyCrush::Blue]);
                    SDL_SetRenderDrawColor(renderer, 255, 80, 80, 1);
                    SDL_RenderDrawRect(renderer, &selectedCellRect);
                }
                
                for (auto removedCell: gameBoardChange.removedCells) {
                    auto image = cellTextures[removedCell.second];
                    auto fromDestination = rectForCellPosition(removedCell.first, image);
                    
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
                        
                        const auto& image = cellTextures[cell];
                        
                        
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
                        
                        auto animatedgameBoardRect = gameBoardRect.y-18;
                        
                        // Handle animation from over the board
                        auto cutoff = std::max(animatedgameBoardRect-fromDestination.y, 0);
                        int w, h;
                        SDL_QueryTexture(image, NULL, NULL, &w, &h);
                        SDL_Rect srcRect = {0,cutoff,w,h-cutoff};
                        
                        if (fromDestination.y < animatedgameBoardRect) {
                            fromDestination.y = animatedgameBoardRect;
                            fromDestination.h -= cutoff;
                        }
                        SDL_RenderCopy(renderer, image, &srcRect, &fromDestination);
                    }
                }
                
                
                SDL_RenderPresent(renderer);
                SDL_Delay(5);
                distance += move;
        }
        auto currentTime = std::chrono::high_resolution_clock::now();
        int numberOfMilliSecondsElapsed = (int)std::chrono::duration_cast<std::chrono::milliseconds>(currentTime-startTime).count();
        std::cout << "Rendering took: " << numberOfMilliSecondsElapsed << "ms" << std::endl;
    }
    
    void renderGameBoard() {
        auto gameBoardChange = CandyCrushGameBoardChange(game);
        renderGameBoard(gameBoardChange);
    }
    
    GameBoard::CellPosition cellPositionFromCoordinates(int x, int y) const {
        return GameBoard::CellPosition((y-gameBoardRect.y)/cellHeight, (x-gameBoardRect.x)/cellWidth);
    }
    
    void run() {
        bool quit = false;
        bool isMouseDown = false;
        SDL_Event e;
      
        auto renderCallback = [&](CandyCrushGameBoardChange gameBoardChange) {
            renderGameBoard(gameBoardChange);
        };
        
        bool hasShownGameOver = false;
        
        while( !quit ) {
                if (game.gameOver()) {
                    if (!hasShownGameOver) {
                        hasShownGameOver = true;
                        renderGameOver();
                        SDL_RenderPresent(renderer);
                    }
                } else if (isFirstGame) {
                    renderBackground();
                    renderText("Click to start", 360, 250, scoreLabelFont);
                    SDL_RenderPresent(renderer);
                } else {
                    renderGameBoard();
                }
    
            
            while( SDL_PollEvent( &e ) != 0 ) {
                if( e.type == SDL_QUIT ) {
                    quit = true;
                }
                
                // Handle clicks
                if (e.type == SDL_MOUSEBUTTONDOWN){
                    if (isFirstGame || game.gameOver()) {
                        lastMouseDownX = -1;
                        lastMouseDownY = -1;
                        isFirstGame = false;
                        game = CandyCrush();
                        hasShownGameOver = false;
                        
                        // Intro animation - all cells falls from the top in a triangular fashion
                        CandyCrushGameBoardChange triangularFallGameBoardChange(game);
                        for (auto row = 0; row < game.getGameBoard().rows; row++) {
                            for (auto column = 0; column < game.getGameBoard().columns; column++) {
                                auto pair = triangularFallGameBoardChange.gameBoardChange[{row, column}];
                                triangularFallGameBoardChange.gameBoardChange[{row, column}] = {{row-(int)game.getGameBoard().rows-(int)game.getGameBoard().columns+1+column, column}, pair.second};
                            }
                        }
                        
                        renderGameBoard(triangularFallGameBoardChange,3);
                    } else {
                        isMouseDown = true;
                        int x, y;
                        SDL_GetMouseState(&x, &y);
                        
                        auto move = GameBoard::CellSwapMove(cellPositionFromCoordinates(x, y), cellPositionFromCoordinates(lastMouseDownX, lastMouseDownY));
                        
                        std::cout << move << std::endl;
                        
                        if (game.getGameBoard().areCellsAdjacent(move.from, move.to)) {
                            lastMouseDownX = -1;
                            lastMouseDownY = -1;
                            game.play(move, renderCallback);
                            
                        } else {
                            std::cout << "Could not make the move" << std::endl;
                            lastMouseDownX = x;
                            lastMouseDownY = y;
                        }
                        renderGameBoard();
                    }
                }
                
                if (e.type == SDL_MOUSEBUTTONUP && lastMouseDownX != -1) {
                    isMouseDown = false;
                }
                
                // Handle drag event
                if (e.type == SDL_MOUSEMOTION && isMouseDown && !game.gameOver()) {
                    int x, y;
                    SDL_GetMouseState(&x, &y);
                    auto move = GameBoard::CellSwapMove(cellPositionFromCoordinates(x, y), cellPositionFromCoordinates(lastMouseDownX, lastMouseDownY));
                    if (game.getGameBoard().areCellsAdjacent(move.from, move.to)) {
                        lastMouseDownX = -1;
                        lastMouseDownY = -1;
                        game.play(move, renderCallback);
                        renderGameBoard();
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


