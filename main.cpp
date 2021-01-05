#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cmath>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "util.hpp"

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

const int CENTER_WIDTH = 5;

const float MAX_ANGLE = 3.142 / 5.0; // 72 degrees

const int FONT_SIZE = 40;
const int PLAYER_SPEED = 10;

const float BALL_MAXSPEED = 8.0f;
const float BALL_ACCELERATE = 1.05f;
const float BALL_INIT_SPEED = 4.0f;
const int BALL_WIDTH = 10;
const int BALL_HEIGHT = 10;

typedef struct {
    SDL_Rect pos;
    int speed;
    int score;
} Player;

typedef struct {
    float x;
    float y;
    float vx;
    float vy;
    float speed;
} Ball;

float calc_angle(float y1, float y2, int height) {
    float rely = y1 + height/2 - y2;
    rely /= height/2.0;   // Normalize
    return rely * MAX_ANGLE;
}

int main(int argc, char* argv[]) {
    std::cout << "Starting SDL Application..." << std::endl;
    SDL_Event e;
    SDL_Renderer *ren = nullptr;
    SDL_Window *win = nullptr;

    initialize(&ren, &win, SCREEN_WIDTH, SCREEN_HEIGHT);

    int board_width, board_height;
    SDL_Texture *squareTex = IMG_LoadTexture(ren, "img/pong_board.png");
    SDL_QueryTexture(squareTex, NULL, NULL, &board_width, &board_height);

    SDL_Color whiteColor = {255, 255, 255};

    // Define players
    Player p1, p2;

    // x, y, w, h
    p1.pos = p2.pos = SDL_Rect{
      0,
      0,
      board_width,
      board_height,
    };
   
    p1.speed = p2.speed = PLAYER_SPEED;
    p1.score = p2.score = 0;

    p1.pos.x = board_width/2 + 10;
    p2.pos.x = SCREEN_WIDTH - p2.pos.w- 10 - p2.pos.w/2;

    p1.pos.y = SCREEN_HEIGHT/2 - p1.pos.h/2;
    p2.pos.y = SCREEN_HEIGHT/2 - p2.pos.h/2;
    
    // ------------------------------------
    // TODO: Define Ball
    Ball b;
    b.x = 0;
    b.y = 100;
    b.vx = 5;
    b.vy = 0;
    b.speed = 5;
    // ------------------------------------

    std::cout << "Starting Game Loop" << std::endl;

    bool quit = false;
    char buffer[512];
    const Uint8 *keystates = SDL_GetKeyboardState(NULL);

    unsigned int frame = 0;
    while(!quit) {
      while(SDL_PollEvent(&e)) {
        if(e.type == SDL_QUIT)
          quit = true;
        if(e.type == SDL_KEYDOWN) {
          switch(e.key.keysym.scancode) {
            case SDL_SCANCODE_ESCAPE:
              quit = true;
              break;
          }
        }
      }

      // Player Movement
      if(keystates[SDL_SCANCODE_UP])
        p1.pos.y -= p1.speed;
      if(keystates[SDL_SCANCODE_DOWN])
        p1.pos.y += p1.speed;

      // ---------------------------------------
      // TODO: Player 2 controlled by human
      // Basic AI
      if(b.y < p2.pos.y + p2.pos.h/2)
        p2.pos.y -= p2.speed;

      if(b.y > p2.pos.y + p2.pos.h/2)
        p2.pos.y += p2.speed;
        // if(keystates[SDL_SCANCODE_W])
        	// p1.pos.y -= p1.speed;
      	// if(keystates[SDL_SCANCODE_S])
        	// p1.pos.y += p1.speed;
      // ---------------------------------------


      // Limit Ball Max Speed
      if(b.vx > BALL_MAXSPEED)
        b.vx = BALL_MAXSPEED;

      if(b.vy > BALL_MAXSPEED)
        b.vy = BALL_MAXSPEED;

      // Update Ball coordinates
      b.x += b.vx;
      b.y += b.vy;


      // ---------------------------------------
      // TODO: Boundary Collision of Ball
      // Y axis
      if (b.y < 0)
      {
        float angle = calc_angle(p1.pos.y, b.y, p1.pos.h);
        b.vx = b.speed * cos(angle);
        b.vy = ((b.vy>0)? -1 : 1) * b.speed * sin(angle);
      }
      else if (b.y + BALL_HEIGHT >= SCREEN_HEIGHT)
      {
        b.vx = -b.vx;
        // Update ball's position and speed
      }
      // X axis
      if (b.x < 0)
      {
        // Update ball's position and speed, and the player's score
        b.x = 0;
      }
      else if (b.x + BALL_WIDTH >= SCREEN_WIDTH)
      {
        // Update ball's position and speed, and the player's score
        b.x = SCREEN_HEIGHT - BALL_HEIGHT;
      }
      // ---------------------------------------

      // Boundary Collision of Players
      if(p1.pos.y < 0) p1.pos.y = 0;
      if(p1.pos.y + p1.pos.h > SCREEN_HEIGHT) p1.pos.y = SCREEN_HEIGHT - p1.pos.h;
      if(p2.pos.y < 0) p2.pos.y = 0;
      if(p2.pos.y + p2.pos.h > SCREEN_HEIGHT) p2.pos.y = SCREEN_HEIGHT - p2.pos.h;

      // Create a dummpy b_rect structure to detect collision
      SDL_Rect b_rect  = {
        (int) b.x,
        (int) b.y,
        BALL_WIDTH,
        BALL_HEIGHT,
      };

      // Player Collision
      if(SDL_HasIntersection(&p1.pos, &b_rect)) {
        b.x = p1.pos.x + p1.pos.w;
        b.speed = b.speed * BALL_ACCELERATE;
        float angle = calc_angle(p1.pos.y, b.y, p1.pos.h);
        b.vx = b.speed * cos(angle);
        b.vy = ((b.vy>0)? -1 : 1) * b.speed * sin(angle);
      } else if(SDL_HasIntersection(&p2.pos, &b_rect)) {
        b.x = p2.pos.x - BALL_WIDTH;
        b.speed = b.speed * BALL_ACCELERATE;
        float angle = calc_angle(p2.pos.y, b.y, p2.pos.h);
        b.vx = -1 * b.speed * cos(angle);
        b.vy = ((b.vy>0)? -1 : 1) * b.speed * sin(angle);
      }

      SDL_RenderClear(ren);

      SDL_RenderCopy(ren, squareTex, NULL, &p1.pos);
      SDL_RenderCopy(ren, squareTex, NULL, &p2.pos);

      // Draw the center line
      renderTexture(squareTex, ren, SCREEN_WIDTH/2 - CENTER_WIDTH/2, 0, CENTER_WIDTH, SCREEN_HEIGHT);
     
      // Draw the Ball
      renderTexture(squareTex, ren, b.x, b.y, BALL_WIDTH, BALL_HEIGHT);

      // Display the score
      sprintf(buffer, "%d", p1.score);
      SDL_Texture *p1score = renderText(buffer, "fonts/sample.ttf", whiteColor, FONT_SIZE, ren);
      sprintf(buffer, "%d", p2.score);
      SDL_Texture *p2score = renderText(buffer, "fonts/sample.ttf", whiteColor, FONT_SIZE, ren);

      int width;
      SDL_QueryTexture(p1score, NULL, NULL, &width, NULL);

      renderTexture(p1score, ren, SCREEN_WIDTH/2 - width - 10, 10);
      renderTexture(p2score, ren, SCREEN_WIDTH/2 + 10, 10);

      SDL_DestroyTexture(p1score);
      SDL_DestroyTexture(p2score);

      
      SDL_RenderPresent(ren);

      frame++;
    }


    SDL_DestroyTexture(squareTex);
    Cleanup(&ren, &win);
    return 0;
}

