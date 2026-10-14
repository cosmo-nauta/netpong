#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "raylib.h"
#include "common.h"

int main(void)
{
  /*  NET  */
  
  int sockfd, n;
  struct sockaddr_in servaddr;
  char buffer[BUFSIZE];
  char dummy = 'H';
  int player_number;
  
  // clear servaddr
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  servaddr.sin_port = htons(PORT);
  servaddr.sin_family = AF_INET;
    
  // create datagram socket
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    
  // connect to server
  if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
      printf("\n Error : Connect Failed \n");
      exit(0);
    }

  sendto(sockfd, &dummy, sizeof(dummy), 0, NULL, 0);
  recvfrom(sockfd, &player_number, sizeof(player_number), 0, NULL, NULL);

  printf("PLAYER NUMBER: %d\n", player_number);
  /* GAME */
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "netpong client - 1000x700");
  int scrollSpeed = 4;
  int framesCounter = 0;
  Vector2 boxSize = {PADDLE_W, PADDLE_H};

  fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK);
  
  GameState state;
  memset(&state, 0, sizeof(state));
  
  while(!WindowShouldClose())
    {
      char c = 0;
      if (player_number == 1) {
	if (IsKeyDown(KEY_W)) c = 'U';
	if (IsKeyDown(KEY_S)) c = 'D';
      }
      if (player_number == 2) {
	if (IsKeyDown(KEY_UP))   c = 'U';
	if (IsKeyDown(KEY_DOWN)) c = 'D';
      }
      if (c != 0) send(sockfd, &c, sizeof(c), 0);
      
      GameState tmp;
      while (recv(sockfd, &tmp, sizeof(tmp), 0) == (ssize_t)sizeof(tmp))
	state = tmp;
      
      ssize_t n = recvfrom(sockfd, &state, sizeof(state), 0, NULL, NULL);
      
      //draw
      
      
      Rectangle box1 = {
	state.paddle1Pos.x,
	state.paddle1Pos.y,
	PADDLE_W,
	PADDLE_H
      };

      Rectangle box2 = {
	state.paddle2Pos.x,
	state.paddle2Pos.y,
	PADDLE_W,
	PADDLE_H
      };

      BeginDrawing();

      ClearBackground(RAYWHITE);

      DrawCircleV(state.ballPosition, (float)state.ballRadius, BLUE);

       // DrawRectangle(30, 300, 20, 100, GREEN);
      DrawRectangleV(state.paddle1Pos, boxSize, GREEN);
	 // DrawRectangle(950, 300, 20, 100, RED);
      DrawRectangleV(state.paddle2Pos, boxSize, RED);
      
      DrawText(TextFormat("GREEN : %d", state.score1), 30, 30, 30, GREEN);
      DrawText(TextFormat("RED : %d", state.score2), 870, 30, 30, RED);
      DrawFPS(10,10);
      
      EndDrawing();
      
    }
  
  CloseWindow();

  return 0;
}

