#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <emscripten.h>
#include <emscripten/html5.h>

#define WIDTH 30
#define HEIGHT 20
#define CELL_SIZE 20
#define MAX_LENGTH 200

typedef struct {
    int x, y;
} Point;

typedef struct {
    Point body[MAX_LENGTH];
    int length;
    int dx, dy;
    int alive;
} Snake;

typedef struct {
    char name[30];
    int score;
} Player;

Snake snake;
Point fruit;
int score = 0;
int gameOver = 0;
char playerName[30] = "Player";
Player leaderboard[10];
int leaderboardCount = 0;

EM_JS(void, drawRect, (int x, int y, const char *color), {
    const ctx = Module.ctx;
    ctx.fillStyle = UTF8ToString(color);
    ctx.fillRect(x, y, Module.cellSize, Module.cellSize);
});

EM_JS(void, clearCanvas, (), {
    const ctx = Module.ctx;
    ctx.fillStyle = "black";
    ctx.fillRect(0, 0, Module.canvas.width, Module.canvas.height);
});

EM_JS(void, updateScore, (int score), {
    document.getElementById("score").innerText = "Score: " + score;
});

EM_BOOL key_callback(int eventType, const EmscriptenKeyboardEvent *e, void *userData) {
    switch (e->keyCode) {
        case 37: case 65: snake.dx = -1; snake.dy = 0; break; // left / A
        case 38: case 87: snake.dx = 0; snake.dy = -1; break; // up / W
        case 39: case 68: snake.dx = 1; snake.dy = 0; break;  // right / D
        case 40: case 83: snake.dx = 0; snake.dy = 1; break;  // down / S
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
    snake.alive = 1;
    snake.body[0].x = WIDTH / 2;
    snake.body[0].y = HEIGHT / 2;
    score = 0;
    spawnFruit();
    updateScore(score);
}

void drawGame() {
    clearCanvas();
    // Draw fruit
    drawRect(fruit.x * CELL_SIZE, fruit.y * CELL_SIZE, "red");
    // Draw snake
    for (int i = 0; i < snake.length; i++) {
        drawRect(snake.body[i].x * CELL_SIZE, snake.body[i].y * CELL_SIZE, "lime");
    }
}

void updateGame() {
    if (!snake.alive) return;

    Point newHead = snake.body[0];
    newHead.x += snake.dx;
    newHead.y += snake.dy;

    // Collisions
    if (newHead.x < 0 || newHead.x >= WIDTH || newHead.y < 0 || newHead.y >= HEIGHT) {
        snake.alive = 0;
        return;
    }
    for (int i = 0; i < snake.length; i++) {
        if (snake.body[i].x == newHead.x && snake.body[i].y == newHead.y) {
            snake.alive = 0;
            return;
        }
    }

    // Move body
    for (int i = snake.length; i > 0; i--)
        snake.body[i] = snake.body[i - 1];
    snake.body[0] = newHead;

    // Eat fruit
    if (newHead.x == fruit.x && newHead.y == fruit.y) {
        if (snake.length < MAX_LENGTH)
            snake.length++;
        score += 10;
        updateScore(score);
        spawnFruit();
    }
}

void addToLeaderboard() {
    if (leaderboardCount < 10) {
        strcpy(leaderboard[leaderboardCount].name, playerName);
        leaderboard[leaderboardCount].score = score;
        leaderboardCount++;
    }

    // sort descending
    for (int i = 0; i < leaderboardCount - 1; i++) {
        for (int j = i + 1; j < leaderboardCount; j++) {
            if (leaderboard[j].score > leaderboard[i].score) {
                Player tmp = leaderboard[i];
                leaderboard[i] = leaderboard[j];
                leaderboard[j] = tmp;
            }
        }
    }

    printf("\n=== Leaderboard ===\n");
    for (int i = 0; i < leaderboardCount; i++) {
        printf("%d. %s - %d\n", i + 1, leaderboard[i].name, leaderboard[i].score);
    }
}

void loop() {
    if (snake.alive) {
        updateGame();
        drawGame();
    } else {
        printf("Game Over! Final Score: %d\n", score);
        addToLeaderboard();
        emscripten_cancel_main_loop();
    }
}

int main() {
    srand(time(NULL));
    printf("Enter your name: ");
    scanf("%s", playerName);

    EM_ASM({
        Module.canvas = document.getElementById("canvas");
        Module.ctx = Module.canvas.getContext("2d");
        Module.cellSize = $0;
    }, CELL_SIZE);

    resetGame();
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, key_callback);
    emscripten_set_main_loop(loop, 10, 1);
    return 0;
}
