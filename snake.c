#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <emscripten.h>
#include <emscripten/html5.h>

#define WIDTH 40
#define HEIGHT 20
#define MAX_LENGTH 100

typedef struct {
    int x, y;
} Point;

typedef struct {
    Point body[MAX_LENGTH];
    int length;
    int dx, dy;
} Snake;

Snake snake;
Point fruit;
int score = 0;
int gameOver = 0;
char playerName[30];

typedef struct {
    char name[30];
    int score;
} Player;

Player leaderboard[10];
int leaderboardCount = 0;

EM_BOOL key_callback(int eventType, const EmscriptenKeyboardEvent *e, void *userData) {
    switch (e->keyCode) {
        case 37: case 65: snake.dx = -1; snake.dy = 0; break; // left / A
        case 38: case 87: snake.dx = 0; snake.dy = -1; break; // up / W
        case 39: case 68: snake.dx = 1; snake.dy = 0; break; // right / D
        case 40: case 83: snake.dx = 0; snake.dy = 1; break; // down / S
    }
    return 0;
}

void spawnFruit() {
    fruit.x = rand() % WIDTH;
    fruit.y = rand() % HEIGHT;
}

void resetGame() {
    snake.length = 3;
    snake.dx = 1;
    snake.dy = 0;
    snake.body[0].x = WIDTH / 2;
    snake.body[0].y = HEIGHT / 2;
    score = 0;
    gameOver = 0;
    spawnFruit();
}

void draw() {
    printf("\033[H\033[J"); // clear terminal
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            int isBody = 0;
            for (int i = 0; i < snake.length; i++) {
                if (snake.body[i].x == x && snake.body[i].y == y) {
                    printf("O");
                    isBody = 1;
                    break;
                }
            }
            if (!isBody) {
                if (fruit.x == x && fruit.y == y)
                    printf("F");
                else
                    printf(".");
            }
        }
        printf("\n");
    }
    printf("\nPlayer: %s | Score: %d\n", playerName, score);
}

void update() {
    if (gameOver) return;

    Point newHead = snake.body[0];
    newHead.x += snake.dx;
    newHead.y += snake.dy;

    if (newHead.x < 0 || newHead.x >= WIDTH || newHead.y < 0 || newHead.y >= HEIGHT) {
        gameOver = 1;
        return;
    }

    for (int i = 0; i < snake.length; i++) {
        if (snake.body[i].x == newHead.x && snake.body[i].y == newHead.y) {
            gameOver = 1;
            return;
        }
    }

    for (int i = snake.length - 1; i > 0; i--)
        snake.body[i] = snake.body[i - 1];
    snake.body[0] = newHead;

    if (newHead.x == fruit.x && newHead.y == fruit.y) {
        if (snake.length < MAX_LENGTH)
            snake.length++;
        score += 10;
        spawnFruit();
    }
}

void addToLeaderboard() {
    if (leaderboardCount < 10) {
        strcpy(leaderboard[leaderboardCount].name, playerName);
        leaderboard[leaderboardCount].score = score;
        leaderboardCount++;
    }

    for (int i = 0; i < leaderboardCount - 1; i++) {
        for (int j = i + 1; j < leaderboardCount; j++) {
            if (leaderboard[j].score > leaderboard[i].score) {
                Player temp = leaderboard[i];
                leaderboard[i] = leaderboard[j];
                leaderboard[j] = temp;
            }
        }
    }
}

void showLeaderboard() {
    printf("\n===== Leaderboard =====\n");
    for (int i = 0; i < leaderboardCount; i++) {
        printf("%d. %s - %d\n", i + 1, leaderboard[i].name, leaderboard[i].score);
    }
}

void gameLoop() {
    if (!gameOver) {
        update();
        draw();
    } else {
        printf("\nGame Over! Final Score: %d\n", score);
        addToLeaderboard();
        showLeaderboard();
        emscripten_cancel_main_loop();
    }
}

int main() {
    srand(time(NULL));
    printf("Enter your name: ");
    scanf("%s", playerName);
    resetGame();
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, key_callback);
    emscripten_set_main_loop(gameLoop, 5, 1); // 5 FPS
    return 0;
}