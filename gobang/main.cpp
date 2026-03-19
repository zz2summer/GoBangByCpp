#include<stdio.h>
#include<graphics.h>
#include<mmsystem.h>
#include<math.h>
#include<windows.h>
#include<time.h>

#pragma comment(lib, "winmm.lib")

int flag = 0;
int board[20][20] = { 0 };

typedef struct {
	int x, y;
	int player;
} ChessMove;

ChessMove moveHistory[400];
int historyCount = 0;

int timeLimit = 30;
int playerTime[2] = { 30, 30 };
int gameOver = 0;
HANDLE hTimerThread = NULL;

void initGame();
int judge(int a, int b);
void playChess();
void undoMove();
void updateTimeDisplay();
DWORD WINAPI timerThread(LPVOID lpParam);
void stopTimer();

int main() {

	initGame();
	playChess();

	getchar();
	return 0;
}

void initGame() {

	initgraph(600, 500);

	loadimage(NULL, "./src/bg.jpg");

	mciSendString("open ./src/skyCity.mp3", 0, 0, 0);

	for (int i = 0; i <= 500; i += 25) {
		line(0, i, 500, i);
		line(i, 0, i, 500);
	}
	line(501, 0, 501, 500);

	setfillcolor(RGB(240, 240, 240));
	solidrectangle(505, 50, 595, 450);

	setbkmode(1);
	settextcolor(BLACK);

	outtextxy(510, 60, "玩家1：黑棋");
	outtextxy(510, 90, "玩家2：白棋");
	outtextxy(510, 130, "操作说明：");
	outtextxy(510, 155, "左键：落子");
	outtextxy(510, 180, "右键：悔棋");
	outtextxy(510, 210, "计时模式：");
	outtextxy(510, 235, "每步30秒");
	outtextxy(510, 260, "超时判负");

	updateTimeDisplay();
}

DWORD WINAPI timerThread(LPVOID lpParam) {
	while (!gameOver) {
		Sleep(1000);
		if (gameOver) break;

		int currentPlayer = flag % 2;
		playerTime[currentPlayer]--;

		if (playerTime[currentPlayer] <= 0) {
			gameOver = 1;
			MessageBox(NULL, currentPlayer == 0 ? "玩家1超时判负，玩家2获胜！" : "玩家2超时判负，玩家1获胜！", "游戏结束", MB_OK);
			exit(0);
		}

		updateTimeDisplay();
	}
	return 0;
}

void updateTimeDisplay() {
	setfillcolor(RGB(240, 240, 240));
	solidrectangle(505, 290, 595, 380);

	char timeStr1[50], timeStr2[50];
	sprintf(timeStr1, "玩家1: %d秒", playerTime[0]);
	sprintf(timeStr2, "玩家2: %d秒", playerTime[1]);

	setbkmode(1);
	settextcolor(BLACK);
	outtextxy(510, 300, timeStr1);
	outtextxy(510, 330, timeStr2);
}

void stopTimer() {
	if (hTimerThread != NULL) {
		gameOver = 1;
		WaitForSingleObject(hTimerThread, 1000);
		CloseHandle(hTimerThread);
		hTimerThread = NULL;
	}
}

void undoMove() {
	if (historyCount == 0) {
		MessageBox(NULL, "暂无棋可悔！", "提示", MB_OK);
		return;
	}

	ChessMove lastMove = moveHistory[historyCount - 1];
	board[lastMove.x][lastMove.y] = 0;
	historyCount--;

	flag--;

	setfillcolor(RGB(240, 240, 240));
	solidcircle(lastMove.x * 25, lastMove.y * 25, 12);

	setlinecolor(BLACK);
	for (int i = 0; i <= 500; i += 25) {
		line(0, i, 500, i);
		line(i, 0, i, 500);
	}
	line(501, 0, 501, 500);

	playerTime[flag % 2] += 30;
	if (playerTime[flag % 2] > 60) playerTime[flag % 2] = 60;
	updateTimeDisplay();
}

void playChess() {

	MOUSEMSG m;
	int x = 0, y = 0;
	int a = 0, b = 0;

	hTimerThread = CreateThread(NULL, 0, timerThread, NULL, 0, NULL);

	while (1) {
		m = GetMouseMsg();

		for (int i = 1; i < 20; i++) {
			for (int j = 1; j < 20; j++) {
				if (abs(m.x - i * 25) < 12 && abs(m.y - j * 25) < 12) {
					x = i * 25;
					y = j * 25;
					a = i;
					b = j;
				}
			}
		}

		if (m.uMsg == WM_RBUTTONDOWN) {
			undoMove();
			continue;
		}

		if (m.uMsg == WM_LBUTTONDOWN) {
			if (board[a][b] != 0) {
				MessageBox(NULL, "该位置已有棋子，请重新选择！", "提示", MB_OK);
				continue;
			}

			if (flag % 2 == 0) {
				setfillcolor(BLACK);
				solidcircle(x, y, 10);
				board[a][b] = 1;
			}
			else {
				setfillcolor(WHITE);
				solidcircle(x, y, 10);
				board[a][b] = 2;
			}

			moveHistory[historyCount].x = a;
			moveHistory[historyCount].y = b;
			moveHistory[historyCount].player = board[a][b];
			historyCount++;

			playerTime[flag % 2] = timeLimit;
			updateTimeDisplay();

			flag++;

			if (judge(a, b)) {
				stopTimer();
				if (flag % 2 == 0) {
					MessageBox(NULL, "玩家2获胜！", "游戏结束", MB_OK);
				}
				else {
					MessageBox(NULL, "玩家1获胜！", "游戏结束", MB_OK);
				}
				exit(0);
			}
		}
	}
}


int judge(int a, int b) {
	int i, j;
	int t = 2 - flag % 2;

	for (i = a - 4, j = b; i <= a; i++) {
		if (i > 0 && i < 16 && t == board[i][j] && t == board[i + 1][j] && t == board[i + 2][j] && t == board[i + 3][j] && t == board[i + 4][j]) {
			return 1;
		}
	}

	for (i = a, j = b - 4; j <= b; j++) {
		if (j > 0 && j < 16 && t == board[i][j] && t == board[i][j + 1] && t == board[i][j + 2] && t == board[i][j + 3] && t == board[i][j + 4]) {
			return 1;
		}
	}

	for (i = a - 4, j = b - 4; i <= a, j <= b; i++, j++) {
		if (i > 0 && i < 16 && j > 0 && j < 16 && t == board[i][j] && t == board[i + 1][j + 1] && t == board[i + 2][j + 2] && t == board[i + 3][j + 3] && t == board[i + 4][j + 4]) {
			return 1;
		}
	}

	for (i = a - 4, j = b + 4; i <= a, j >= b; i++, j--) {
		if (i > 0 && i < 16 && j > 0 && j < 16 && t == board[i][j] && t == board[i + 1][j - 1] && t == board[i + 2][j - 2] && t == board[i + 3][j - 3] && t == board[i + 4][j - 4]) {
			return 1;
		}
	}

	return 0;
}
