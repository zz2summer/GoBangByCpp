#include<stdio.h>
#include<graphics.h>
#include<mmsystem.h>
#include<math.h>
#include<time.h>
#include<windows.h>

#pragma comment(lib, "winmm.lib")

// 游戏常量
#define BOARD_SIZE 20
#define CELL_SIZE 25
#define TIME_LIMIT 30  // 每步限时30秒

// 游戏状态
int flag = 0;
int board[BOARD_SIZE][BOARD_SIZE] = { 0 };

// 计时相关
int timeLeft = TIME_LIMIT;
time_t lastMoveTime = 0;
int currentPlayer = 1;

// 悔棋相关 - 使用栈结构记录历史
#define MAX_MOVES 400
struct Move {
    int x;      // 像素坐标
    int y;      // 像素坐标
    int a;      // 棋盘坐标
    int b;      // 棋盘坐标
    int player; // 玩家 1或2
};
Move moveHistory[MAX_MOVES];
int moveCount = 0;

// 函数声明
void initGame();
int judge(int a, int b);
void playChess();
void drawTimer();
void updateTimer();
int checkTimeout();
void recordMove(int x, int y, int a, int b, int player);
void undoMove();
void redrawBoard();
void showGameInfo();

int main() {
    initGame();
    playChess();
    getchar();
    return 0;
}

// 初始化游戏
void initGame() {
    // 创建窗口
    initgraph(700, 550);

    // 加载背景
    loadimage(NULL, "./src/bg.jpg");

    // 播放音乐
    mciSendString("open ./src/skyCity.mp3", 0, 0, 0);

    // 绘制棋盘
    setlinecolor(BLACK);
    for (int i = 0; i <= 500; i += CELL_SIZE) {
        line(0, i, 500, i);
        line(i, 0, i, 500);
    }
    line(501, 0, 501, 500);

    // 显示游戏信息区域
    showGameInfo();

    // 初始化计时器
    lastMoveTime = time(NULL);
    timeLeft = TIME_LIMIT;
    drawTimer();
}

// 显示游戏信息
void showGameInfo() {
    // 右侧信息面板背景
    setfillcolor(RGB(240, 240, 240));
    solidrectangle(510, 0, 700, 550);

    // 标题
    settextcolor(BLACK);
    setbkmode(TRANSPARENT);
    settextstyle(20, 0, _T("微软雅黑"));
    outtextxy(520, 20, _T("五子棋游戏"));

    // 玩家信息
    settextstyle(16, 0, _T("微软雅黑"));
    outtextxy(520, 60, _T("玩家1: 黑棋"));
    outtextxy(520, 85, _T("玩家2: 白棋"));

    // 操作说明
    settextstyle(14, 0, _T("微软雅黑"));
    outtextxy(520, 350, _T("操作说明:"));
    outtextxy(520, 375, _T("左键点击下棋"));
    outtextxy(520, 400, _T("右键点击悔棋"));

    // 悔棋按钮区域
    setfillcolor(RGB(200, 200, 200));
    solidrectangle(520, 300, 620, 330);
    settextcolor(BLACK);
    settextstyle(16, 0, _T("微软雅黑"));
    outtextxy(535, 307, _T("悔棋 (右键)"));
}

// 绘制计时器
void drawTimer() {
    // 清除之前的计时器显示区域
    setfillcolor(RGB(240, 240, 240));
    solidrectangle(520, 120, 680, 280);

    // 显示当前玩家
    settextcolor(BLACK);
    settextstyle(18, 0, _T("微软雅黑"));
    if (currentPlayer == 1) {
        outtextxy(520, 130, _T("当前回合: 黑棋"));
    } else {
        outtextxy(520, 130, _T("当前回合: 白棋"));
    }

    // 显示倒计时
    settextstyle(24, 0, _T("微软雅黑"));
    char timerStr[50];
    sprintf(timerStr, "剩余时间: %d秒", timeLeft);

    // 时间少于10秒显示红色警告
    if (timeLeft <= 10) {
        settextcolor(RED);
    } else {
        settextcolor(BLACK);
    }
    outtextxy(520, 180, timerStr);

    // 显示进度条
    int barWidth = 150;
    int filledWidth = (timeLeft * barWidth) / TIME_LIMIT;

    // 进度条背景
    setfillcolor(WHITE);
    solidrectangle(520, 220, 520 + barWidth, 240);

    // 进度条填充
    if (timeLeft <= 10) {
        setfillcolor(RED);
    } else if (timeLeft <= 20) {
        setfillcolor(YELLOW);
    } else {
        setfillcolor(GREEN);
    }
    solidrectangle(520, 220, 520 + filledWidth, 240);

    // 进度条边框
    setlinecolor(BLACK);
    rectangle(520, 220, 520 + barWidth, 240);
}

// 更新计时器
void updateTimer() {
    time_t currentTime = time(NULL);
    int elapsed = (int)(currentTime - lastMoveTime);

    if (elapsed > 0) {
        timeLeft = TIME_LIMIT - elapsed;
        if (timeLeft < 0) timeLeft = 0;
        drawTimer();
    }
}

// 检查是否超时
int checkTimeout() {
    if (timeLeft <= 0) {
        return 1;
    }
    return 0;
}

// 记录落子历史
void recordMove(int x, int y, int a, int b, int player) {
    if (moveCount < MAX_MOVES) {
        moveHistory[moveCount].x = x;
        moveHistory[moveCount].y = y;
        moveHistory[moveCount].a = a;
        moveHistory[moveCount].b = b;
        moveHistory[moveCount].player = player;
        moveCount++;
    }
}

// 重绘棋盘
void redrawBoard() {
    // 重新加载背景并绘制棋盘
    loadimage(NULL, "./src/bg.jpg");

    // 绘制棋盘线
    setlinecolor(BLACK);
    for (int i = 0; i <= 500; i += CELL_SIZE) {
        line(0, i, 500, i);
        line(i, 0, i, 500);
    }
    line(501, 0, 501, 500);

    // 重绘所有棋子
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] != 0) {
                int px = i * CELL_SIZE;
                int py = j * CELL_SIZE;
                if (board[i][j] == 1) {
                    setfillcolor(BLACK);
                } else {
                    setfillcolor(WHITE);
                }
                solidcircle(px, py, 10);
            }
        }
    }

    // 重绘信息面板
    showGameInfo();
    drawTimer();
}

// 悔棋功能
void undoMove() {
    if (moveCount == 0) {
        MessageBox(NULL, _T("还没有落子，无法悔棋！"), _T("提示"), MB_OK);
        return;
    }

    // 获取上一步
    moveCount--;
    Move lastMove = moveHistory[moveCount];

    // 清除棋盘上的记录
    board[lastMove.a][lastMove.b] = 0;

    // 回退回合
    flag--;
    currentPlayer = (flag % 2 == 0) ? 1 : 2;

    // 重置计时器
    lastMoveTime = time(NULL);
    timeLeft = TIME_LIMIT;

    // 重绘棋盘
    redrawBoard();

    // 显示悔棋成功提示
    settextcolor(BLUE);
    settextstyle(16, 0, _T("微软雅黑"));
    outtextxy(520, 430, _T("悔棋成功！"));
}

// 主游戏循环
void playChess() {
    MOUSEMSG m;
    int x = 0, y = 0;
    int a = 0, b = 0;

    while (1) {
        // 更新计时器
        updateTimer();

        // 检查超时
        if (checkTimeout()) {
            char msg[100];
            if (currentPlayer == 1) {
                sprintf(msg, "黑棋超时！白棋获胜！");
            } else {
                sprintf(msg, "白棋超时！黑棋获胜！");
            }
            MessageBox(NULL, msg, _T("游戏结束"), MB_OK);
            exit(0);
        }

        // 非阻塞检查鼠标消息
        if (MouseHit()) {
            m = GetMouseMsg();

            // 获取最近的棋盘交点
            for (int i = 1; i < BOARD_SIZE; i++) {
                for (int j = 1; j < BOARD_SIZE; j++) {
                    if (abs(m.x - i * CELL_SIZE) < 12 && abs(m.y - j * CELL_SIZE) < 12) {
                        x = i * CELL_SIZE;
                        y = j * CELL_SIZE;
                        a = i;
                        b = j;
                    }
                }
            }

            // 左键下棋
            if (m.uMsg == WM_LBUTTONDOWN) {
                // 检查是否重复落子
                if (board[a][b] != 0) {
                    MessageBox(NULL, _T("该位置已经有棋子了，请重新选择！"), _T("提示"), MB_OK);
                    continue;
                }

                // 检查是否在有效区域内
                if (a <= 0 || a >= BOARD_SIZE || b <= 0 || b >= BOARD_SIZE) {
                    continue;
                }

                // 判断黑白棋
                int player = (flag % 2 == 0) ? 1 : 2;
                if (player == 1) {
                    setfillcolor(BLACK);
                    solidcircle(x, y, 10);
                    board[a][b] = 1;
                    currentPlayer = 2;
                } else {
                    setfillcolor(WHITE);
                    solidcircle(x, y, 10);
                    board[a][b] = 2;
                    currentPlayer = 1;
                }

                // 记录落子历史
                recordMove(x, y, a, b, player);

                // 重置计时器
                lastMoveTime = time(NULL);
                timeLeft = TIME_LIMIT;
                drawTimer();

                flag++;

                // 检查胜负
                if (judge(a, b)) {
                    char msg[50];
                    if (player == 1) {
                        sprintf(msg, "黑棋获胜！");
                    } else {
                        sprintf(msg, "白棋获胜！");
                    }
                    MessageBox(NULL, msg, _T("游戏结束"), MB_OK);
                    exit(0);
                }
            }

            // 右键悔棋
            if (m.uMsg == WM_RBUTTONDOWN) {
                undoMove();
            }
        }

        // 短暂休眠以避免CPU占用过高
        Sleep(100);
    }
}

// 判断胜负
int judge(int a, int b) {
    int i, j;
    int t = 2 - flag % 2;

    // 水平方向
    for (i = a - 4, j = b; i <= a; i++) {
        if (i > 0 && i < 16 && t == board[i][j] && t == board[i + 1][j] && t == board[i + 2][j] && t == board[i + 3][j] && t == board[i + 4][j]) {
            return 1;
        }
    }

    // 垂直方向
    for (i = a, j = b - 4; j <= b; j++) {
        if (j > 0 && j < 16 && t == board[i][j] && t == board[i][j + 1] && t == board[i][j + 2] && t == board[i][j + 3] && t == board[i][j + 4]) {
            return 1;
        }
    }

    // 对角线方向（左上到右下）
    for (i = a - 4, j = b - 4; i <= a && j <= b; i++, j++) {
        if (i > 0 && i < 16 && j > 0 && j < 16 && t == board[i][j] && t == board[i + 1][j + 1] && t == board[i + 2][j + 2] && t == board[i + 3][j + 3] && t == board[i + 4][j + 4]) {
            return 1;
        }
    }

    // 对角线方向（右上到左下）
    for (i = a - 4, j = b + 4; i <= a && j >= b; i++, j--) {
        if (i > 0 && i < 16 && j > 0 && j < 16 && t == board[i][j] && t == board[i + 1][j - 1] && t == board[i + 2][j - 2] && t == board[i + 3][j - 3] && t == board[i + 4][j - 4]) {
            return 1;
        }
    }

    return 0;
}
