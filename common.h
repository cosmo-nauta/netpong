#ifndef COMMON_H
#define COMMON_H

#include "raymath.h"

#define PORT 5000
#define BUFSIZE 64

#define SCREEN_WIDTH 1000.0f
#define SCREEN_HEIGHT 700.0f

#define PADDLE_W 20.0f
#define PADDLE_H 100.0f

typedef struct {
    Vector2 ballPosition;
    Vector2 ballSpeed;
    int ballRadius;
    Vector2 paddle1Pos;
    Vector2 paddle2Pos;
    float paddleSpeed;
    unsigned int score1;
    unsigned int score2;
} GameState;

typedef struct {
    float x, y, width, height;
} Rect;

static inline int check_collision_circle_rect(Vector2 center, float radius, Rect rec) {
    float closestX = center.x < rec.x ? rec.x :
                      (center.x > rec.x + rec.width ? rec.x + rec.width : center.x);
    float closestY = center.y < rec.y ? rec.y :
                      (center.y > rec.y + rec.height ? rec.y + rec.height : center.y);
    float dx = center.x - closestX;
    float dy = center.y - closestY;
    return (dx*dx + dy*dy) <= (radius*radius);
}

void update_ball(GameState *state);

#endif
