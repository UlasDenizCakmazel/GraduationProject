#include "icb_gui.h"
#include "cstdlib"  // rand(), srand()
#include "ctime"    // time()
#include "cstdio" // sprintf


int FRM1;
int keypressed;
ICBYTES screen;
ICBYTES Blocks, Block;
HANDLE HMutex;

enum ScreenState {
	INTRO,
	MENU,
	CREDITS,
	GAME,
};

enum GameState {
	STARTGAME,
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
	bool spaceState;
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
	bool DropState;
	bool MoveLeftStatus;
	bool MoveRightStatus;
	bool RotateStatus;
};

ScreenState screenState = GAME;
GameState gameState = GAMEOVER;
TableSquares TetrisMatrix[20][10];
GameBlockType BlockList[2]; // BlockList[0] -> NowBlock  &  BlockList[1] -> NextBlock
TetrisGameBlock obj;
bool createdobject = false;
int RemoveLinesId[4];
int Score;

//bool nextObjectStatus;

void ICGUI_Create() {
	ICG_MWTitle("Tetris");
	ICG_MWSize(680, 655);
}

void WhenKeyPressed(int k)
{
	if (keypressed == 0)
		keypressed = k;
}

void IntegerToChar(int value, char* buffer) {

	// Geçici bir dizide sayýyý sakla
	char temp[12];                 // Max 10 basamak + '-' + '\0'
	int index = 0;

	do {
		temp[index++] = (value % 10) + '0'; // Son basamaðý al ve karaktere çevir
		value /= 10;                       // Sayýyý bir basamak küçült
	} while (value > 0);

	// Temp dizisindeki sayýlarý ters çevir ve buffer'a aktar
	while (index > 0) {
		*buffer++ = temp[--index];
	}
	*buffer = '\0'; // Sonuna null karakteri ekle
}

void CreateScoreTable() {
	FillRect(screen, 20, 100, 150, 56, 0xDDDDDD);

	FillRect(screen, 23, 103, 144, 50, 0x252F62);

	FillRect(screen, 20, 128, 150, 1, 0xDDDDDD);

	Impress12x20(screen, 30, 108, "SCORE", 0xFFFFFF);

	Impress12x20(screen, 30, 134, "0", 0xFFFFFF);

}

void PrintScore() {

	char scoreStr[20];

	IntegerToChar(Score, scoreStr);

	FillRect(screen, 30, 134, 135, 15, 0x252F62);

	Impress12x20(screen, 30, 134, scoreStr, 0xFFFFFF);

}

void CreateNextBlockTable() {
	FillRect(screen, 480, 100, 150, 150, 0xDDDDDD);

	FillRect(screen, 483, 103, 144, 144, 0x252F62);

	FillRect(screen, 480, 128, 150, 1, 0xDDDDDD);

	Impress12x20(screen, 490, 108, "Next Block", 0xFFFFFF);
}


ICBYTES Coordinates = {
	{ 485,132,140,113 },  //Table Coordinates
	{ 11,10,30,106 },  //TETROMINO_I
	{ 11,137,55,55 },  //TETROMINO_O
	{ 113,137,80,55 },  //TETROMINO_T
	{ 62,10,55,80 },  //TETROMINO_J
	{ 138,10,55,80 },  //TETROMINO_L
	{ 215,10,80,55 },  //TETROMINO_S
	{ 215,99,80,54 }  //TETROMINO_Z
};


void PrintNextBlock(GameBlockType type)  {
	int x, y, width, height; //Tetromino block
	//type = TETROMINO_S;
	switch (type)

	{
	case TETROMINO_I: {
		x = Coordinates.I(1, 2);
		y = Coordinates.I(2, 2);
		width = Coordinates.I(3, 2);
		height = Coordinates.I(4, 2);
		break;
	}
	case TETROMINO_O: {
		x = Coordinates.I(1, 3);
		y = Coordinates.I(2, 3);
		width = Coordinates.I(3, 3);
		height = Coordinates.I(4, 3);
		break;
	}
	case TETROMINO_T: {
		x = Coordinates.I(1, 4);
		y = Coordinates.I(2, 4);
		width = Coordinates.I(3, 4);
		height = Coordinates.I(4, 4);
		break;
	}
	case TETROMINO_J: {
		x = Coordinates.I(1, 5);
		y = Coordinates.I(2, 5);
		width = Coordinates.I(3, 5);
		height = Coordinates.I(4, 5);
		break;
	}
	case TETROMINO_L: {
		x = Coordinates.I(1, 6);
		y = Coordinates.I(2, 6);
		width = Coordinates.I(3, 6);
		height = Coordinates.I(4, 6);
		break;
	}
	case TETROMINO_S: {
		x = Coordinates.I(1, 7);
		y = Coordinates.I(2, 7);
		width = Coordinates.I(3, 7);
		height = Coordinates.I(4, 7);
		break;
	}
	case TETROMINO_Z: {
		x = Coordinates.I(1, 8);
		y = Coordinates.I(2, 8);
		width = Coordinates.I(3, 8);
		height = Coordinates.I(4, 8);
		break;
	}
	default:
		break;
	}
	FillRect(screen, Coordinates.I(1, 1), Coordinates.I(2, 1), Coordinates.I(3, 1), Coordinates.I(4, 1), 0x252F62); //Clean
	Copy(Blocks, x, y, width, height, Block);
	PasteNon0(Block, Coordinates.I(1, 1) + Coordinates.I(3, 1) / 2 - width / 2, Coordinates.I(2, 1) + Coordinates.I(4, 1) / 2 - height / 2, screen);
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

	if(!createdobject)
		for (int i = 0; i < 20; i++) {
			sqr_x = x + sqr_space;
			for (int j = 0; j < 10; j++) {
					TetrisMatrix[i][j] = { sqr_x, sqr_y, sqr_size, 0x192a60 , true };
				sqr_x = sqr_x + sqr_size + sqr_space;
			}
			sqr_y = sqr_y + sqr_size + sqr_space;
		}
}

GameBlockType SelectBlockType() {
	// Enum'un minimum ve maksimum deðerlerini belirle
	int minValue = TETROMINO_I; // 0
	int maxValue = TETROMINO_Z; // 6

	// Rastgele bir deðer seç
	GameBlockType RandomBlockType = static_cast<GameBlockType>(minValue + rand() % (maxValue - minValue + 1));
	return RandomBlockType;
}


void PrintTetrisMatrix() {
	for (int i = 0; i < 20; i++) {
		for (int j = 0; j < 10; j++) {
			FillRect(screen, TetrisMatrix[i][j].x, TetrisMatrix[i][j].y, TetrisMatrix[i][j].size, TetrisMatrix[i][j].size, TetrisMatrix[i][j].color);
		}
	}
}

void PrintTetrisBlock(TetrisGameBlock object) {
	for (int i = 0; i < 4; i++) {
		TetrisMatrix[object.GameBlockSquares[i].row][object.GameBlockSquares[i].col].color = object.color;
	}
	//PrintTetrisMatrix();
}

void DeleteTetrisBlock(TetrisGameBlock object) {
	for (int i = 0; i < 4; i++) {
		TetrisMatrix[object.GameBlockSquares[i].row][object.GameBlockSquares[i].col].color = 0x192a60;
	}
}

TetrisGameBlock CreateNewBlock() {
	//nextObjectStatus = true;
	GameBlockType type = BlockList[0];
	//BlockList[1] = SelectBlockType();
	//type = TETROMINO_Z;
	//BlockList[0] = BlockList[1];
	switch (type)
	{
	case TETROMINO_I: {
		//obj = { type, 0x008bac, { {-1,3,0xFF0000}, {-1,4,0x008bac}, {-1,6,0xFFFF00}, {-1,5,0xFFFFFF}}, 400 };
		obj = { type, 0x52e5fa, { {-1,3}, {-1,4}, {-1,6}, {-1,5}}, 400, true, true, true, false };
		break;
	}
	case TETROMINO_O: {
		//obj = { type, 0x008bac, { {-2,4,0xFF0000}, {-1,4,0x008bac}, {-2,5,0xFFFF00}, {-1,5,0xFFFFFF}}, 400 };
		obj = { type, 0xffff00 , { {-2,4}, {-1,4}, {-2,5}, {-1,5} }, 400, true, true, true, false };
		break;
	}
	case TETROMINO_T: {
		//obj = { type, 0x008bac, { {-2,3,0xFF0000}, {-2,4,0x008bac}, {-2,5,0xFFFF00}, {-1,4,0xFFFFFF}}, 400 };
		obj = { type, 0x9696ff , { {-2,3}, {-2,4}, {-2,5}, {-1,4} }, 400, true, true, true, false }; //þuan hazýr deðil
		break;
	}
	case TETROMINO_J: {
		//obj = { type, 0x008bac, { {-2,3,0xFF0000}, {-1,3,0x008bac}, {-1,5,0xFFFF00}, {-1,4,0xFFFFFF}}, 400 };
		obj = { type, 0x0000ff , { {-2,3}, {-1,3}, {-1,5}, {-1,4} }, 400, true, true, true, false }; //þuan hazýr deðil
		break;
	}
	case TETROMINO_L: {
		//obj = { type, 0x008bac, { {-1,3,0xFF0000}, {-1,4,0x008bac}, {-2,5,0xFFFF00}, {-1,5,0xFFFFFF}}, 400 };
		obj = { type, 0xdfb54e , { {-1,3}, {-1,4}, {-2,5}, {-1,5} }, 400, true, true, true, false }; //þuan hazýr deðil
		break;
	}
	case TETROMINO_S: {
		//obj = { type, 0x008bac, { {-1,3,0xFF0000}, {-2,4,0x008bac}, {-2,5,0xFFFF00}, {-1,4,0xFFFFFF}}, 400 };
		obj = { type, 0x3dc747 , {  {-1,3 }, {-2,4}, {-2,5},{-1,4} }, 400, true, true, true, false }; //þuan hazýr deðil
		break;
	}
	case TETROMINO_Z: {
		//obj = { type, 0x008bac, { {-2,3,0xFF0000}, {-2,4,0x008bac}, {-1,5,0xFFFF00}, {-1,4,0xFFFFFF}}, 400 };
		obj = { type, 0xff5757 , { {-2,3}, {-2,4}, {-1,5}, {-1,4} }, 400, true, true, true, false }; //þuan hazýr deðil
		break;
	}
	default:
		break;
	}
	createdobject = true;
	return obj;
}



int TurnOtherBlockControl() {
	for (int i = 0; i < 4; i++) {
		if (obj.GameBlockSquares[i].row >= 0) {
			if (!TetrisMatrix[obj.GameBlockSquares[i].row][obj.GameBlockSquares[i].col].spaceState)
				return -1;
		}
	}
	return 1;
}

bool RemoveBlockAnimation(int LineCount, int color) {
	for (int i = 0; i < LineCount; i++) {
		for (int j = 0; j < 10; j++) {
			TetrisMatrix[RemoveLinesId[i]][j].color = color;
		}
	}

	return true;

}

void GameOverControl() {
	for (int i = 0; i < 4; i++) {
		if (obj.GameBlockSquares[i].row < 0) {
			gameState = GAMEOVER;
			break;
		}
	}
}

void ScoreCalculate(int c) {
	switch (c){
	case 1: { Score += 100; break; }
	case 2: { Score += 300; break; }
	case 3: { Score += 500; break; }
	case 4: { Score += 800; break; }
	default: break;
	}
}

void RemoveLinesSquares() {
	int count = 0;
	bool remove;
	bool allSquareSpace;
	int allSpaceLineId;

	if (createdobject ) {
		for (int i = 19; i >= 0; i--) {
			remove = false;
			for (int j = 0; j < 10; j++) {
				if (!TetrisMatrix[i][j].spaceState) {
					remove = true;
				}
				else {
					remove = false;
					break;
				}
			}
			if (remove) {
				RemoveLinesId[count] = i;
				count++;
			}
			if (count == 4)
				break;
		}

		if (count != 0) {
			// Remove Animation
			int k = 0;
			while (k < 8) {
				Sleep(200);
				if (gameState != PAUSE) {
					if (k % 2 == 0)
						RemoveBlockAnimation(count, 0xFFFFFF);
					else
						RemoveBlockAnimation(count, 0x192a60);
					k++;
				}
			}
			Sleep(200);
			// drop the upper frames down
			for (int i = RemoveLinesId[0]; i >= count; i--) {
				for (int j = 0; j < 10; j++) {
					TetrisMatrix[i][j].color = TetrisMatrix[i - count][j].color;
					TetrisMatrix[i][j].spaceState = TetrisMatrix[i - count][j].spaceState;
				}
			}
			ScoreCalculate(count);
			PrintScore();
		}
	}
}



void PlaceObjectOnMatrix() {
	while (gameState == PAUSE) {

	}
	if (createdobject) {
		for (int i = 0; i < 4; i++) {
			TetrisMatrix[obj.GameBlockSquares[i].row][obj.GameBlockSquares[i].col].spaceState = false;
		}
	
		//PrintTetrisBlock(obj);

		RemoveLinesSquares();

		GameOverControl();

		if(gameState != GAMEOVER){

			createdobject = false;

			BlockList[0] = BlockList[1];

			BlockList[1] = SelectBlockType();

			PrintNextBlock(BlockList[1]);

			CreateNewBlock();
		}
		
	}
}

int DropControl() {
	if (createdobject && gameState != PAUSE) {
		if (obj.GameBlockSquares[3].row == 19) {
			obj.DropState = false;
			PlaceObjectOnMatrix();
			return 0;
		}
		else {
			for (int k = 0; k < 2; k++) {
				for (int i = 3; i >= 0; i--) {
					if (obj.GameBlockSquares[i].row + 1 >= 0) {
						if (!(TetrisMatrix[obj.GameBlockSquares[i].row + 1][obj.GameBlockSquares[i].col].spaceState)) {
							obj.DropState = false;
							break;
						}
						else {
							obj.DropState = true;
						}
					}
				}
			}
		}

		if (!obj.DropState) {
			PlaceObjectOnMatrix();
		}
	}


	return 0;
	
}

void BlockDrop() {
	DropControl();
	if (obj.DropState) {
		DeleteTetrisBlock(obj);
		for (int i = 0; i < 4; i++) {
			obj.GameBlockSquares[i].row++;		
		}
	}
	PrintTetrisBlock(obj);
}

void AutoBlockDrop() {
	Sleep(1500);
	while (1) {	
		while (gameState != PAUSE ) {
			if (createdobject) {
				if (keypressed == 0 ) {
					BlockDrop();
					Sleep(1000);
				}
			}
		}
	}
}

int Move_Left_Rigth_Controler(int c){
	if (obj.GameBlockSquares[0].col + c < 0 || obj.GameBlockSquares[2].col + c > 9 ) {
		c < 0 ? obj.MoveLeftStatus = false : obj.MoveRightStatus = false;
		return 0;
	}

	for (int i = 3; i >= 0; i--) {
		if (obj.GameBlockSquares[i].row >= 0) {
			if (!(TetrisMatrix[obj.GameBlockSquares[i].row][obj.GameBlockSquares[i].col + c].spaceState)) {
				c < 0 ? obj.MoveLeftStatus = false : obj.MoveRightStatus = false;
				return 0;
			}
			else {
				c < 0 ? obj.MoveLeftStatus = true : obj.MoveRightStatus = true;
			}
		}
	}

	return 0;

}

void BlockReturnil(int k) {
	int a = 1;
	int temp;
	DeleteTetrisBlock(obj);
	do {
		temp = a;
		k *= a;
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
		a = TurnOtherBlockControl();
	} while (a == -1);

	if (temp == -1) {
		k < 0 ? obj.RotateState++ : obj.RotateState++;
	}

	if (obj.GameBlockSquares[0].col < 0) {
		obj.GameBlockSquares[3].col += obj.GameBlockSquares[0].col * (-1);
		obj.GameBlockSquares[2].col += obj.GameBlockSquares[0].col * (-1);
		obj.GameBlockSquares[1].col += obj.GameBlockSquares[0].col * (-1);
		obj.GameBlockSquares[0].col += obj.GameBlockSquares[0].col * (-1);
	}
	if (obj.GameBlockSquares[2].col > 9) {
		obj.GameBlockSquares[0].col -= obj.GameBlockSquares[2].col - 9;
		obj.GameBlockSquares[1].col -= obj.GameBlockSquares[2].col - 9;
		obj.GameBlockSquares[3].col -= obj.GameBlockSquares[2].col - 9;
		obj.GameBlockSquares[2].col -= obj.GameBlockSquares[2].col - 9;
	}

	PrintTetrisBlock(obj);

	Move_Left_Rigth_Controler(-1);
	Move_Left_Rigth_Controler(1);
	DropControl();

	keypressed = 0;

}

void BlockMove_Left_Right(int move) {
	bool control;
	Move_Left_Rigth_Controler(move);
	move < 0 ? control = obj.MoveLeftStatus : control = obj.MoveRightStatus;
	if (control) {
		DeleteTetrisBlock(obj);
		for (int i = 0; i < 4; i++) {
			obj.GameBlockSquares[i].col += move;
		}
		move < 0 ? obj.MoveRightStatus=true : control = obj.MoveLeftStatus=true;

		PrintTetrisBlock(obj);
	}

	keypressed = 0;

}

void MusicControllerThread() {
	//PlaySound("sound/Intro.wav", NULL, SND_SYNC);
	while (true)
		PlaySound("Sound/Music.wav", NULL, SND_SYNC);
}

void KeyPressedControl() {
	//bool mutexTaken = true;
	while (1) {
		
		
	}
	
				
}

void StartGame() {
	// Rastgeleliði baþlatmak için seed oluþtur
	srand(static_cast<unsigned>(time(0)));  // Seed baþlatýlýr
	CreateTetrisTable();
	CreateScoreTable();
	CreateNextBlockTable();
	PrintTetrisMatrix();
	BlockList[0] = SelectBlockType();
	CreateNewBlock();
	//Sleep(600);
	BlockList[1] = SelectBlockType();
	PrintNextBlock(BlockList[1]);
	Score = 0;

	while (1) {
		while (gameState != GAMEOVER) {
			if (keypressed == 67 && obj.DropState && gameState != PAUSE) { // c = 67 turn left
				BlockReturnil(-1);
				obj.RotateState--;

			}
			else if (keypressed == 32 && obj.DropState && gameState != PAUSE) { // V = 86 (turn right)
				obj.RotateState++;
				BlockReturnil(1);

			}
			else if (keypressed == 37 && obj.MoveLeftStatus && gameState != PAUSE) // left_arrow = 37 (move left)
			{
				BlockMove_Left_Right(-1);

			}
			else if (keypressed == 39 && obj.MoveRightStatus && gameState != PAUSE) // right_arrow = 39 (move right)
			{
				BlockMove_Left_Right(1);

			}
			else if (keypressed == 40 && obj.DropState && gameState != PAUSE) // down_arrow = 39 (move down)
			{
				BlockDrop();
				keypressed = 0;
			}
			else if (keypressed == 80) // P = 80 (Pause)
			{
				if (gameState != PAUSE) {
					gameState = PAUSE;
				}
				else {

					gameState = GAMING;
				}
				keypressed = 0;
			}
			else {
				keypressed = 0;
			}
			Sleep(50);
		}
		Sleep(1000);
		while (gameState == GAMEOVER) {
			if (keypressed != 0) {

			}
			Sleep(60);
		}
	}
}

void DrawThread() {
	screen = 0;
	gameState = STARTGAME;
	GameState prev = GAMING;
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

			if (gameState== STARTGAME){
				gameState = GAMING;
				//nextObjectStatus = false;
				CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StartGame, NULL, 0, NULL);
				CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MusicControllerThread, NULL, 0, NULL);
				
				CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AutoBlockDrop, NULL, 0, NULL);
				CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)KeyPressedControl, NULL, 0, NULL);
				
			}
			else if (gameState == GAMEOVER) {
				Impress12x20(screen, 267, 295, "GAME OVER", 0xFFFFFF);
			}
			else {
				if (gameState == PAUSE) {
					PrintTetrisMatrix();
					Impress12x20(screen, 285, 295, "PAUSED", 0xFFFFFF);
				}
				else if (gameState != PAUSE && prev == PAUSE) {
					CreateTetrisTable();
					PrintTetrisMatrix();
				}
				else {
					PrintTetrisMatrix();
				}
			}

			prev = gameState;


		}
		default:
			break;
		}
		
		Sleep(60);

		DisplayImage(FRM1, screen);
	}
}

void ICGUI_main() {
	FRM1 = ICG_FrameMedium(0, 0, 650, 600);
	ICG_SetOnKeyPressed(WhenKeyPressed);
	ReadImage("Assets/blocks.bmp", Blocks);
	CreateImage(screen, 650, 600, ICB_UINT);
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)DrawThread, NULL, 0, NULL);
	
}