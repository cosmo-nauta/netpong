// server program for udp connection
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include<netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#define RAYMATH_IMPLEMENTATION
#include "common.h"

typedef struct {
    struct sockaddr_in addr;
    socklen_t addrlen;
    int connesso; // 0 = free slot, 1 = taken
} Client;

static long long now_ms(void)
{
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec * 1000LL + ts.tv_nsec / 1000000;
}

int identify_player(Client players[2], struct sockaddr_in *cliaddr,
                                  socklen_t clilen, int listenfd)
{
    for (int i = 0; i < 2; i++) {
        if (players[i].connesso &&
            players[i].addr.sin_addr.s_addr == cliaddr->sin_addr.s_addr &&
            players[i].addr.sin_port == cliaddr->sin_port) {
            return i;
        }
    }

    for (int i = 0; i < 2; i++) {
        if (!players[i].connesso) {
            players[i].addr = *cliaddr;
            players[i].addrlen = clilen;
            players[i].connesso = 1;

            int player_number = i + 1;
            sendto(listenfd, &player_number, sizeof(player_number), 0,
                   (struct sockaddr*)&players[i].addr, players[i].addrlen);

            return i;
        }
    }

    return -1;
}

void update_ball(GameState *state)
{
   
  state->ballPosition.x += state->ballSpeed.x;
  state->ballPosition.y += state->ballSpeed.y;
      Rect box1 = { state->paddle1Pos.x, state->paddle1Pos.y, PADDLE_W, PADDLE_H };
    Rect box2 = { state->paddle2Pos.x, state->paddle2Pos.y, PADDLE_W, PADDLE_H };

  if (check_collision_circle_rect(state->ballPosition,
			      state->ballRadius, box1)) {
    state->ballSpeed.x *= -1;
  }
  if (check_collision_circle_rect(state->ballPosition,
			      state->ballRadius, box2)) {
    state->ballSpeed.x *= -1;
  }

  //Left Wall
  //Green
  if (state->ballPosition.x <= state->ballRadius) {
    state->ballPosition.x = state->ballRadius;
    state->ballSpeed.x *= -1.0f;
    state->score1++;
    }

  //Right wall
  //Red
  if (state->ballPosition.x >= SCREEN_WIDTH - state->ballRadius)
    {
        state->ballPosition.x = SCREEN_WIDTH - state->ballRadius;
        state->ballSpeed.x *= -1.0f;
	state->score2++;
    }

    // Ceiling
    if (state->ballPosition.y <= state->ballRadius)
    {
        state->ballPosition.y = state->ballRadius;
        state->ballSpeed.y *= -1.0f;
    }

    // Floor 
    if (state->ballPosition.y >= SCREEN_HEIGHT - state->ballRadius)
    {
        state->ballPosition.y = SCREEN_HEIGHT - state->ballRadius;
        state->ballSpeed.y *= -1.0f;
    }

}

void move_paddles(GameState *state ,int idx, char c)
{
  Vector2 *p = (idx == 0 ) ? &state->paddle1Pos : &state->paddle2Pos;

    //left paddle
  if(c == 'U') p->y -= state->paddleSpeed;
  if(c == 'D') p->y += state->paddleSpeed;
    if (p->y < 0) p->y = 0;
    if (p->y > SCREEN_HEIGHT - PADDLE_H) p->y = SCREEN_HEIGHT - PADDLE_H;
}

int main()
{
  /*  NET  */

  int listenfd,len;
  struct sockaddr_in servaddr, cliaddr;
  char buffer[64];
  socklen_t clilen;
  char c;
  bzero(&servaddr, sizeof(servaddr));
  int clientindex = 1; // you need this to know which client you need to assign 

  //create a udp socket
  listenfd = socket(AF_INET, SOCK_DGRAM, 0);        
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(PORT);
  servaddr.sin_family = AF_INET; 

  //bind server address to socket descriptor
  if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  int flags = fcntl(listenfd, F_GETFL, 0);
  fcntl(listenfd, F_SETFL, flags | O_NONBLOCK);

  Client players[2] = {0};
  
  /*  GAME  */

  GameState state = {
    .ballPosition = {SCREEN_WIDTH/2.0f, SCREEN_HEIGHT/2.0f},
    .ballSpeed = {7.0f, 6.0f},
    .ballRadius = 20,
    .paddle1Pos = {30.0f, 300.0f},
    .paddle2Pos = {950.0f, 300.0f},
    .paddleSpeed = 4.0f,
    .score1 = 0,
    .score2 = 0
    };

  long long next_tick = now_ms();
  
  while(1){
    while (1) {
        clilen = sizeof(cliaddr);
        ssize_t n = recvfrom(listenfd, buffer, BUFSIZE - 1, 0,
                             (struct sockaddr*)&cliaddr, &clilen);
        if (n < 0) break;               // EAGAIN: nothing to read

        int idx = identify_player(players, &cliaddr, clilen, listenfd);
        if (idx < 0) continue;          // server full
        if (n > 0) move_paddles(&state, idx, buffer[0]);
    }

    // 2. tick di gioco a ~60 Hz
    long long t = now_ms();
    if (t >= next_tick) {
        next_tick = t + 16;

        if (players[0].connesso && players[1].connesso)
            update_ball(&state);

        for (int i = 0; i < 2; i++) {
            if (players[i].connesso)
                sendto(listenfd, &state, sizeof(state), 0,
                       (struct sockaddr*)&players[i].addr, players[i].addrlen);
        }
    }

    usleep(1000);   
}
}
