#include "icb_gui.h"
#include "cstdlib"  // rand(), srand()
#include "ctime"    // time()

int FRM1;
int keypressed;
ICBYTES screen;

enum ScreenState {
	INTRO,
	MENU,
	CREDITS,
	GAME,
};

enum GameState {
	GAMING,
	PAUSE,
	GAMEOVER,
};

enum GameBlockType {
	TETROMINO_I, //0
	TETROMINO_O, //1
	TETROMINO_T, //2
	TETROMINO_J, //3
	TETROMINO_L, //4
	TETROMINO_S, //5
	TETROMINO_Z, //6
};

struct TableSquares {
	int x;
	int y;
	int size;
	int color;
};

struct TetrisBlockSquare {
	int row;  //TetrisMatrix[i][] 
	int col; //TetrisMatrix[][j]
	int color;
};

struct TetrisGameBlock {
	GameBlockType type;
	int color;
	TetrisBlockSquare GameBlockSquares[4];
	int RotateState;
};

ScreenState screenState = GAME;
GameState gameState = GAMEOVER;
TableSquares TetrisMatrix[20][10];
GameBlockType BlockList[2]; // BlockList[0] -> NowBlock  &  BlockList[1] -> NextBlock
TetrisGameBlock obj;
bool createdobject = false;

void ICGUI_Create() {
	ICG_MWTitle("Tetris");
	ICG_MWSize(680, 655);
}

void WhenKeyPressed(int k)
{
	keypressed = k;
}

void CreateScoreTable() {
	FillRect(screen, 20, 100, 150, 56, 0xDDDDDD);

	FillRect(screen, 23, 103, 144, 50, 0x252F62);

	FillRect(screen, 20, 128, 150, 1, 0xDDDDDD);

	Impress12x20(screen, 30, 108, "SCORE", 0xFFFFFF);

	Impress12x20(screen, 30, 134, "0", 0xFFFFFF);
}

void CreateNextBlockTable() {
	FillRect(screen, 480, 100, 150, 150, 0xDDDDDD);

	FillRect(screen, 483, 103, 144, 144, 0x252F62);

	FillRect(screen, 480, 128, 150, 1, 0xDDDDDD);

	Impress12x20(screen, 490, 108, "Next Block", 0xFFFFFF);
}

void CreateTetrisTable() {
	int x = 202;
	int y = 100;

	int sqr_size = 20; //tetris kare bloklarýnýn boyutu
	int trs_tbl_size = 5; //tetris tablosunun kenar kalýnlýðý
	int sqr_space = 3; //tetris kare bloklarý arasýndaki boþluk
	int sqr_x, sqr_y;

	FillRect(screen, x, y, sqr_size * 10 + sqr_space * 11 + trs_tbl_size * 2, sqr_size * 20 + sqr_space * 21 + trs_tbl_size * 2, 0x768CFF);

	x += trs_tbl_size;
	y += trs_tbl_size;

	FillRect(screen, x, y, sqr_size * 10 + sqr_space * 11, sqr_size * 20 + sqr_space * 21, 0x0B1A49);

	sqr_y = y + sqr_space;

	for (int i = 0; i < 20; i++) {
		sqr_x = x + sqr_space;
		for (int j = 0; j < 10; j++) {
			TetrisMatrix[i][j] = { sqr_x, sqr_y, sqr_size, 0x192a60 };
			//FillRect(screen, sqr_x, sqr_y, sqr_size, sqr_size, 0x192a60);
			sqr_x = sqr_x + sqr_size + sqr_space;
		}
		sqr_y = sqr_y + sqr_size + sqr_space;
	}
}

GameBlockType SelectBlockType() {
	// Rastgeleliði baþlatmak için seed oluþtur
	srand(static_cast<unsigned>(time(0)));
	// Enum'un minimum ve maksimum deðerlerini belirle
	int minValue = TETROMINO_I; // 0
	int maxValue = TETROMINO_Z; // 6
	// Rastgele bir deðer seç
	GameBlockType RandomBlockType = static_cast<GameBlockType>(minValue + rand() % (maxValue - minValue + 1));
	return RandomBlockType;
}

void PrintTetrisBlock(TetrisGameBlock object) {
	for (int i = 0; i < 4; i++) {
		TetrisMatrix[object.GameBlockSquares[i].row][object.GameBlockSquares[i].col].color = object.GameBlockSquares[i].color;
	}
}

void DeleteTetrisBlock(TetrisGameBlock object) {
	for (int i = 0; i < 4; i++) {
		TetrisMatrix[object.GameBlockSquares[i].row][object.GameBlockSquares[i].col].color = 0x192a60;
	}
}


TetrisGameBlock CreateNewBlock() {
	GameBlockType type = BlockList[0];
	type = TETROMINO_S;
	//BlockList[0] = BlockList[1];
	switch (type)
	{
	case TETROMINO_I: {
		obj = { type, 0x008bac, { {-1,3,0xFF0000}, {-1,4,0x008bac}, {-1,6,0xFFFF00}, {-1,5,0xFFFFFF}}, 400 };
		//obj = { type, 0x008bac, { {-1,3}, {-1,4}, {-1,6}, {-1,5}}, 0 };
		break;
	}
	case TETROMINO_O: {
		obj = { type, 0x008bac, { {-2,4,0xFF0000}, {-1,4,0x008bac}, {-1,5,0xFFFF00}, {-2,5,0xFFFFFF}}, 400 };
		//obj = { type, 0xFFFFFF , { {-2,4}, {-1,4}, {-1,5}, {-2,5} }, 0 };
		break;
	}
	case TETROMINO_T: {
		obj = { type, 0x008bac, { {-2,3,0xFF0000}, {-2,4,0x008bac}, {-2,5,0xFFFF00}, {-1,4,0xFFFFFF}}, 400 };
		//obj = { type, 0xdf9300 , { {-2,3}, {-2,4}, {-2,5}, {-1,4} }, 0 }; //þuan hazýr deðil
		break;
	}
	case TETROMINO_J: {
		obj = { type, 0x008bac, { {-2,3,0xFF0000}, {-1,3,0x008bac}, {-1,5,0xFFFF00}, {-1,4,0xFFFFFF}}, 400 };
		//obj = { type, 0xdf9300 , { {-2,3}, {-1,3}, {-1,5}, {-1,4} }, 0 }; //þuan hazýr deðil
		break;
	}
	case TETROMINO_L: {
		 obj = { type, 0x008bac, { {-1,3,0xFF0000}, {-1,4,0x008bac}, {-2,5,0xFFFF00}, {-1,5,0xFFFFFF}}, 400 };
		//obj = { type, 0x0000FF , { {-1,3}, {-1,4}, {-1,5}, {-2,5} }, 0 }; //þuan hazýr deðil
		break;
	}
	case TETROMINO_S: {
		obj = { type, 0x008bac, { {-1,3,0xFF0000}, {-2,4,0x008bac}, {-2,5,0xFFFF00}, {-1,4,0xFFFFFF}}, 400 };
		//obj = { type, 0x0000FF , {  {-1,3 }, {-2,4}, {-2,5},{-1,4} }, 0 }; //þuan hazýr deðil
		break;
	}
	case TETROMINO_Z: {
		obj = { type, 0x008bac, { {-2,3,0xFF0000}, {-2,4,0x008bac}, {-1,5,0xFFFF00}, {-1,4,0xFFFFFF}}, 400 };
		//obj = { type, 0x0000FF , { {-2,3}, {-2,4}, {-1,4}, {-1,5} }, 0 }; //þuan hazýr deðil
		break;
	}
	default:
		break;
	}
	createdobject = true;
	return obj;
}

void BlockReturnil(int k){
	DeleteTetrisBlock(obj);
	switch (obj.type)
	{
	case TETROMINO_I: {
		if (obj.RotateState % 2 == 0) {
			obj.GameBlockSquares[0].row += 2 * k;
			obj.GameBlockSquares[0].col -= 2 * k;
			obj.GameBlockSquares[1].row += 1 * k;
			obj.GameBlockSquares[1].col -= 1 * k;
			obj.GameBlockSquares[2].col += 1 * k;
			obj.GameBlockSquares[3].row -= 1 * k;
		}
		else { //obj.RotateState % 2 == 1
			obj.GameBlockSquares[0].row -= 2 * k;
			obj.GameBlockSquares[0].col += 2 * k;
			obj.GameBlockSquares[1].row -= 1 * k;
			obj.GameBlockSquares[1].col += 1 * k;
			obj.GameBlockSquares[2].col -= 1 * k;
			obj.GameBlockSquares[3].row += 1 * k;
			
		}
		break;
	}
	case TETROMINO_T: {
		if (obj.RotateState % 4 == 0) {
			obj.GameBlockSquares[0].col -= 1 * k;
			obj.GameBlockSquares[1].row += 1 * k;
		}
		else if (obj.RotateState % 4 == 1) {
			obj.GameBlockSquares[1].row -= 1 * k;
			obj.GameBlockSquares[2].col -= 1 * k;
		}
		else if (obj.RotateState % 4 == 2) {
			obj.GameBlockSquares[2].col += 1 * k;
			obj.GameBlockSquares[3].row -= 1 * k;
		}
		else { //obj.RotateState % 2 == 3
			obj.GameBlockSquares[0].col += 1 * k;
			obj.GameBlockSquares[3].row += 1 * k;
		}
		break;
	}
	case TETROMINO_J: {
		if (obj.RotateState % 4 == 0) {
			obj.GameBlockSquares[0].row -= 2 * k;
			obj.GameBlockSquares[1].row += 1 * k;
			obj.GameBlockSquares[1].col -= 1 * k;
			obj.GameBlockSquares[2].col += 1 * k;
			obj.GameBlockSquares[3].row -= 1 * k;
		}
		else if (obj.RotateState % 4 == 1) {
			obj.GameBlockSquares[0].col += 1 * k;
			obj.GameBlockSquares[1].col += 1 * k;
			obj.GameBlockSquares[2].row -= 1 * k;
			obj.GameBlockSquares[3].row += 1 * k;
		}
		else if (obj.RotateState % 4 == 2) {
			obj.GameBlockSquares[0].col -= 1 * k;
			obj.GameBlockSquares[0].row += 1 * k;
			obj.GameBlockSquares[2].row += 1 * k;
			obj.GameBlockSquares[3].col += 1 * k;
			
		}
		else { //obj.RotateState % 2 == 3
			obj.GameBlockSquares[0].row += 1 * k;
			obj.GameBlockSquares[1].row -= 1 * k;
			obj.GameBlockSquares[2].col -= 1 * k;
			obj.GameBlockSquares[3].col -= 1 * k;
		}
		break;
	}
	case TETROMINO_L: {
		if (obj.RotateState % 4 == 0) {
			obj.GameBlockSquares[0].row += 1 * k;
			obj.GameBlockSquares[2].col += 1 * k;
			obj.GameBlockSquares[3].row -= 1 * k;
			obj.GameBlockSquares[3].col += 1 * k;
		}
		else if (obj.RotateState % 4 == 1) {
			obj.GameBlockSquares[0].row -= 1 * k;
			obj.GameBlockSquares[0].col += 1 * k;
			obj.GameBlockSquares[2].row += 2 * k;
			obj.GameBlockSquares[3].col -= 1 * k;
			obj.GameBlockSquares[3].row += 1 * k;
		}
		else if (obj.RotateState % 4 == 2) {
			obj.GameBlockSquares[0].row += 1 * k;
			obj.GameBlockSquares[0].col -= 1 * k;
			obj.GameBlockSquares[2].row -= 1 * k;
			obj.GameBlockSquares[3].col -= 1 * k;
		}
		else { //obj.RotateState % 2 == 3
			obj.GameBlockSquares[0].row -= 1 * k;
			obj.GameBlockSquares[2].row -= 1 * k;
			obj.GameBlockSquares[2].col -= 1 * k;
			obj.GameBlockSquares[3].col += 1 * k;
		}
		break;
	}
	case TETROMINO_S: {
		if (obj.RotateState % 2 == 0) {
			obj.GameBlockSquares[0].row += 2 * k;
			obj.GameBlockSquares[0].col -= 1 * k;
			obj.GameBlockSquares[3].col -= 1 * k;

		}
		else { //obj.RotateState % 2 == 1
			obj.GameBlockSquares[0].row -= 2 * k;
			obj.GameBlockSquares[0].col += 1 * k;
			obj.GameBlockSquares[3].col += 1 * k;
		}
		break;
	}
	case TETROMINO_Z: {
		if (obj.RotateState % 2 == 0) {
			obj.GameBlockSquares[2].row += 2 * k;
			obj.GameBlockSquares[2].col += 1 * k;
			obj.GameBlockSquares[3].col += 1 * k;

		}
		else { //obj.RotateState % 2 == 1
			obj.GameBlockSquares[2].row -= 2 * k;
			obj.GameBlockSquares[2].col -= 1 * k;
			obj.GameBlockSquares[3].col -= 1 * k;
		}
		break;
	}
	default:
		break;
	}
	PrintTetrisBlock(obj);
}

void BlockDrop() {
	if (obj.GameBlockSquares[3].row < 19) {
		
		DeleteTetrisBlock(obj);
		for (int i = 0; i < 4; i++) {
			obj.GameBlockSquares[i].row++;		
		}
		PrintTetrisBlock(obj);
	}
	else {
		
	}
}

void AutoBlockDrop() {
	while (1) {		
		if (createdobject){
			Sleep(1000);
			BlockDrop();
		}
		else {
			Sleep(100);
		}
	}
}

void BlockMove_Left_Right(int move) {
	DeleteTetrisBlock(obj);
	for (int i = 0; i < 4; i++) {
		obj.GameBlockSquares[i].col += move; 
	}
	PrintTetrisBlock(obj);
}

void PrintTetrisMatrix() {
	for (int i = 0; i < 20; i++) {
		for (int j = 0; j < 10; j++) {
			FillRect(screen, TetrisMatrix[i][j].x, TetrisMatrix[i][j].y, TetrisMatrix[i][j].size, TetrisMatrix[i][j].size, TetrisMatrix[i][j].color);
				
		}
	}
}

void StartGame() {
	CreateTetrisTable();
	CreateScoreTable();
	CreateNextBlockTable();
	PrintTetrisMatrix();
	BlockList[0] = SelectBlockType();
	BlockList[1] = SelectBlockType();
	CreateNewBlock();

	while (1) {
		PrintTetrisMatrix();
		Sleep(100);
	}

}

void KeyPressedControl() {
	while (1) {
		if (keypressed == 67) { // C = 67 turn left
			BlockReturnil(-1);
			obj.RotateState--;
			keypressed = 0;
		}
		else if (keypressed == 86) { // V = 86 (turn right)
			obj.RotateState++;
			BlockReturnil(1);
			keypressed = 0;
		}
		else if (keypressed == 37) // left_arrow = 37 (move left)
		{
			BlockMove_Left_Right(-1);
			keypressed = 0;
		}
		else if (keypressed == 39) //right_arrow = 39 (move right)
		{
			BlockMove_Left_Right(1);
			keypressed = 0;
		}
		else if (keypressed == 40) //right_arrow = 39 (move right)
		{
			BlockDrop();
			keypressed = 0;
		}
		Sleep(300);
	}
}

void DrawThread() {
	screen = 0;
	while (1) {
		switch (screenState)
		{
		case INTRO: {

		}
		case MENU: {

		}
		case CREDITS: {

		}
		case GAME: {
			if (gameState==GAMEOVER){
				gameState = GAMING;
				CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StartGame, NULL, 0, NULL);
				CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AutoBlockDrop, NULL, 0, NULL);
				CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)KeyPressedControl, NULL, 0, NULL);
			}
		}
		default:
			break;
		}
		DisplayImage(FRM1, screen);
		Sleep(30);
	}
}

void ICGUI_main() {
	FRM1 = ICG_FrameMedium(0, 0, 650, 600);
	ICG_SetOnKeyPressed(WhenKeyPressed);
	CreateImage(screen, 650, 600, ICB_UINT);
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)DrawThread, NULL, 0, NULL);

}