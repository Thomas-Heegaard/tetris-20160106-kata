#include <stdio.h>
#include "stdio-t6963c.h"
#include "piece.h"
#include "bdd.h"
#include "t6963c.h"
#include "test.h"
#include "board.h"
#include "keyboard.h"

#define ALSO_EMPTY 0x0E

void calculateScreenPosition(Piece *piece, Position *positionInPiece, Position *screenPosition) {
	unsigned char orientation = (piece->orientation % 4);

	switch(orientation) {
		// Rotation 0: ABC
		//             DEF
		case 0:
			screenPosition->x = positionInPiece->x;
			screenPosition->y = positionInPiece->y;
			break;

		// Rotation 1: DA
		//             EB
		//             FC
		case 1:
			screenPosition->x = piece->block.height - positionInPiece->y - 1;
			screenPosition->y = positionInPiece->x;
			break;

		// Rotation 2: FED
		//             CBA
		//
		case 2:
			screenPosition->x = piece->block.width - positionInPiece->x - 1;
			screenPosition->y = piece->block.height - positionInPiece->y - 1;
			break;

		// Rotation 3: CF
		//             BE
		//             AD
		case 3:
		default:
			screenPosition->x = positionInPiece->y;
			screenPosition->y = piece->block.width - positionInPiece->x - 1;
			break;
	}

	screenPosition->x += piece->position.x;
	screenPosition->y += piece->position.y;
}

unsigned char readPieceContent(Piece *piece, Position *positionInPiece) // Devairs complier mais il nay a pas de test unitaire 
{	
   unsigned char index; 
 
   index = piece->position.x + (piece->position.y * piece->block.width);  // ram point of view

   return piece->block.block_data[index]; // Retour du carat�re situer dans l'adresse m�moire  
}

/**
 * Initialise une pi�ce � la position et rotation voulue, avec le tetromino indiqu�.
 * @param piece La pi�ce � initialiser.
 * @param type Le type de pi�ce (tetromino).
 * @param x Position de la pi�ce.
 * @param y Position de la pi�ce.
 * @param orientation Orientation de la pi�ce.
 */
void PIECE_initialize(Piece *piece, TetrominoType type, unsigned char x, unsigned char y, unsigned char orientation) {
	unsigned char __xdata tetrominoIData[] = {
		PIECE, PIECE, PIECE, PIECE
	};

	unsigned char __xdata tetrominoOData[] = {
		PIECE, PIECE,
		PIECE, PIECE
	};
	unsigned char __xdata tetrominoTData[] = {
		0, PIECE, 0,
		PIECE, PIECE, PIECE
	};
	unsigned char __xdata tetrominoSData[] = {
		0, PIECE, PIECE,
		PIECE, PIECE, 0
	};
	unsigned char __xdata tetrominoZData[] = {
		PIECE, PIECE, 0,
		0, PIECE, PIECE
	};
	unsigned char __xdata tetrominoJData[] = {
		PIECE, 0, 0,
		PIECE, PIECE, PIECE
	};
	unsigned char __xdata tetrominoLData[] = {
		0, 0, PIECE,
		PIECE, PIECE, PIECE	
	};

#ifdef TEST
	unsigned char __xdata testABCFShapeData[] = {
		'A',  'B',  'C',
		EMPTY,EMPTY,'F'
	};
	unsigned char testBlockingShapeData[] = {
		OBSTACLE,OBSTACLE,
		OBSTACLE,OBSTACLE
	};
#endif

	Block __xdata tetromino[] = {
		{4, 1, tetrominoIData},
		{2, 2, tetrominoOData},
		{3, 2, tetrominoTData},
		{3, 2, tetrominoSData},
		{3, 2, tetrominoZData},
		{3, 2, tetrominoJData},
		{3, 2, tetrominoLData},
#ifdef TEST
		{3, 2, testABCFShapeData},
		{2, 2, testBlockingShapeData}
#endif
	};

	piece->block.width = tetromino[type].width;
	piece->block.height = tetromino[type].height;
	piece->block.block_data = tetromino[type].block_data;
	piece->position.x = x;
	piece->position.y = y;
	piece->orientation = orientation;
}

/**
 * Indique si la pi�ce indiqu�e, dans sa rotation et position actuelle,
 * peut �tre plac�e sur le tableau de jeu.
 * Pour ceci aucun des caract�res de la pi�ce ne doit �tre positionn� 
 * sur un obstacle ou d�passer les limites du terrain de jeu.
 * @param piece La description de la pi�ce.
 * @return 1 si la pi�ce peut �tre plac�e � cet endroit. 0 autrement.
 */
char PIECE_canPlace(Piece *piece) {
    int i, j;
    if(piece->position.x < TETRIS_LIMIT_X0 ||
        piece->position.y < TETRIS_LIMIT_Y0 ||
        piece->position.x + piece->block.width > TETRIS_LIMIT_X1 ||
        piece->position.y + piece->block.height > TETRIS_LIMIT_Y1)
        return 0;
    for(i = 0; i < piece->block.width; i++)
        for(j = 0; j < piece->block.height; j++)
            if(piece->block.block_data[j*piece->block.width + i] == PIECE)
                if(T6963C_readFrom(piece->position.x + TETRIS_LIMIT_X0 + i, piece->position.y + TETRIS_LIMIT_Y0 + j) != EMPTY)
                    return 0;
    return 1;
}

/**
 * Dessine la pi�ce indiqu�e.
 * @param piece La description de la pi�ce.
 */
void PIECE_place(Piece *piece) {
}

/**
 * Efface (remplace par des espaces) la pi�ce indiqu�e.
 */
void PIECE_clear(Piece *piece) {
	// � faire
}

/**
 * Remplace par des obstacles la pi�ce indiqu�e.
 * @param piece La pi�ce.
 */
void PIECE_freeze(Piece *piece) {
	// � faire
}

#ifdef TEST

Piece *obtainPieceLShapeAt(char x, char y, unsigned int orientation) {
	static Piece testABCFShape;
	PIECE_initialize(&testABCFShape, ABCF, BDD_SCREEN_X + x, BDD_SCREEN_Y + y, orientation);
	return &testABCFShape;
}

Piece *obtainPieceBlockingAt(char x, char y) {
	static Piece testBlockingShape;
	PIECE_initialize(&testBlockingShape, BLOCKING, BDD_SCREEN_X + x, BDD_SCREEN_Y + y, 0);	
	return &testBlockingShape;
}

int bddClearPieceWithOrientation0() {
	int testsInError = 0;
	BddContent c = {
		"   .......",
		".. .......",
		"..........",
		"..........",
		".........."
	};

	BDD_clear();
	PIECE_clear(obtainPieceLShapeAt(0, 0, 0));
	testsInError += BDD_assert(c, "PCL-0");

	PIECE_clear(obtainPieceLShapeAt(0, 0, 4));
	testsInError += BDD_assert(c, "PCL-4");

	return testsInError;
}

int bddClearPieceWithOrientation1() {
	int testsInError = 0;
	BddContent c = {
		". ........",
		". ........",
		"  ........",
		"..........",
		".........."
	};

	BDD_clear();
	PIECE_clear(obtainPieceLShapeAt(0, 0, 1));
	testsInError += BDD_assert(c, "PCL-1");

	BDD_clear();
	PIECE_clear(obtainPieceLShapeAt(0, 0, 5));
	testsInError += BDD_assert(c, "PCL-5");

	return testsInError;
}

int bddClearPieceWithOrientation2() {
	int testsInError = 0;
	BddContent c = {
		" .........",
		"   .......",
		"..........",
		"..........",
		"..........",
	};

	BDD_clear();
	PIECE_clear(obtainPieceLShapeAt(0, 0, 2));
	testsInError += BDD_assert(c, "PCL-2");

	BDD_clear();
	PIECE_clear(obtainPieceLShapeAt(0, 0, 6));
	testsInError += BDD_assert(c, "PCL-6");

	return testsInError;
}

int bddClearPieceWithOrientation3() {
	int testsInError = 0;
	BddContent c = {
		"  ........",
		" .........",
		" .........",
		"..........",
		"..........",
	};

	BDD_clear();
	PIECE_clear(obtainPieceLShapeAt(0, 0, 3));
	testsInError += BDD_assert(c, "PCL-3");

	BDD_clear();
	PIECE_clear(obtainPieceLShapeAt(0, 0, 7));
	testsInError += BDD_assert(c, "PCL-7");

	return testsInError;
}

int bddPlacePieceWithOrientation0() {
	int testsInError = 0;
	BddContent c = {
		"abc.......",
		"..f.......",
		"..........",
		"..........",
		".........."
	};

	BDD_clear();

	PIECE_place(obtainPieceLShapeAt(0, 0, 0));
	testsInError += BDD_assert(c, "PROT-0");

	PIECE_place(obtainPieceLShapeAt(0, 0, 4));
	testsInError += BDD_assert(c, "PROT-4");

	return testsInError;
}

int bddPlacePieceWithOrientation1() {
	int testsInError = 0;
	BddContent c = {
		".a........",
		".b........",
		"fc........",
		"..........",
		".........."
	};

	BDD_clear();

	PIECE_place(obtainPieceLShapeAt(0, 0, 1));
	testsInError += BDD_assert(c, "PROT-1");

	PIECE_place(obtainPieceLShapeAt(0, 0, 5));
	testsInError += BDD_assert(c, "PROT-5");

	return testsInError;
}

int bddPlacePieceWithOrientation2() {
	int testsInError = 0;
	BddContent c = {
		"f.........",
		"cba.......",
		"..........",
		"..........",
		"..........",
	};

	BDD_clear();

	PIECE_place(obtainPieceLShapeAt(0, 0, 2));
	testsInError += BDD_assert(c, "PROT-2");

	PIECE_place(obtainPieceLShapeAt(0, 0, 6));
	testsInError += BDD_assert(c, "PROT-6");

	return testsInError;
}

int bddPlacePieceWithOrientation3() {
	int testsInError = 0;
	BddContent c = {
		"cf........",
		"b.........",
		"a.........",
		"..........",
		"..........",
	};

	BDD_clear();

	PIECE_place(obtainPieceLShapeAt(0, 0, 3));
	testsInError += BDD_assert(c, "PROT-3");

	PIECE_place(obtainPieceLShapeAt(0, 0, 7));
	testsInError += BDD_assert(c, "PROT-7");

	return testsInError;
}

int bddCanPlacePieceBesidesObstacle() {
	int testsInError = 0;
	BddContent c = {
		"..........",
		".%%a......",
		".%%b......",
		"..fc......",
		"..........",
	};

	BDD_clear();
	PIECE_place(obtainPieceBlockingAt(1, 1));

	testsInError += assertEquals(PIECE_canPlace(obtainPieceLShapeAt(0, 0, 3)), 1, "PCPO0");
	testsInError += assertEquals(PIECE_canPlace(obtainPieceLShapeAt(1, 0, 0)), 1, "PCPO1");
	testsInError += assertEquals(PIECE_canPlace(obtainPieceLShapeAt(0, 2, 2)), 1, "PCPO2");
	testsInError += assertEquals(PIECE_canPlace(obtainPieceLShapeAt(2, 1, 1)), 1, "PCPO3");

	PIECE_place(obtainPieceLShapeAt(2, 1, 1));
	testsInError += BDD_assert(c, "PCCPO");

	return testsInError;
}
int bddCanPlacePieceBesidesBorderOrBottom() {
	int testsInError = 0;
	BddContent c = {
		".......abc",
		"....a....f",
		"....b.....",
		"f..fc.....",
		"cba......."
	};
	Piece *pieceLShape;

	BDD_clear();

	pieceLShape = obtainPieceLShapeAt(7, 0, 0);
	testsInError += assertEquals(PIECE_canPlace(pieceLShape), 1, "PCPB1");
	PIECE_place(pieceLShape);

	pieceLShape = obtainPieceLShapeAt(3, 1, 1);
	testsInError += assertEquals(PIECE_canPlace(pieceLShape), 1, "PCPB2");
	PIECE_place(pieceLShape);

	pieceLShape = obtainPieceLShapeAt(0, 3, 2);
	testsInError += assertEquals(PIECE_canPlace(pieceLShape), 1, "PCPB3");
	PIECE_place(pieceLShape);

	testsInError += BDD_assert(c, "PCPB");

	return testsInError;
}

int bddCannotPlacePieceBecauseObstacle() {
	int testsInError = 0;
	BddContent c = {
		"..........",
		".%%.......",
		".%%.......",
		"..........",
		"..........",
	};

	BDD_clear();
	PIECE_place(obtainPieceBlockingAt(1, 1));

	testsInError += assertEquals(PIECE_canPlace(obtainPieceLShapeAt(0, 0, 0)), 0, "PCNPO1");
	testsInError += assertEquals(PIECE_canPlace(obtainPieceLShapeAt(0, 1, 3)), 0, "PCNPO2");
	testsInError += assertEquals(PIECE_canPlace(obtainPieceLShapeAt(1, 1, 2)), 0, "PCNPO3");
	testsInError += assertEquals(PIECE_canPlace(obtainPieceLShapeAt(0, 2, 1)), 0, "PCNPO4");

//	PIECE_place(obtainPieceLShapeAt(0, 2, 1));
	testsInError += BDD_assert(c, "PCNPO");

	return testsInError;
}

int bddCannotPlacePieceBecauseLeftBorder() {
	int testsInError = 0;

	BDD_clear();

	testsInError += assertEquals(PIECE_canPlace(obtainPieceLShapeAt(-1, 0, 0)), 0, "PCLB10");
	testsInError += assertEquals(PIECE_canPlace(obtainPieceLShapeAt(-1, 0, 1)), 0, "PCLB20");
	testsInError += assertEquals(PIECE_canPlace(obtainPieceLShapeAt(-1, 0, 2)), 0, "PCLB30");
	testsInError += assertEquals(PIECE_canPlace(obtainPieceLShapeAt(-1, 0, 3)), 0, "PCLB40");

	return testsInError;
}

int bddCannotPlacePieceBecauseRightBorder() {
	int testsInError = 0;

	BDD_clear();

	testsInError += assertEquals(PIECE_canPlace(obtainPieceLShapeAt(10, 0, 0)), 0, "PCRB10");
	testsInError += assertEquals(PIECE_canPlace(obtainPieceLShapeAt(10, 0, 1)), 0, "PCRB20");
	testsInError += assertEquals(PIECE_canPlace(obtainPieceLShapeAt(10, 0, 2)), 0, "PCRB30");
	testsInError += assertEquals(PIECE_canPlace(obtainPieceLShapeAt(10, 0, 3)), 0, "PCRB40");

	return testsInError;
}

int bddCannotPlacePieceBecauseBottomBorder() {
	int testsInError = 0;
	BddContent c = {
		"..........",
		"..........",
		"..........",
		"cf........",
		"b........."
	};

	BDD_clear();

	testsInError += assertEquals(PIECE_canPlace(obtainPieceLShapeAt(0, 3, 3)), 0, "PCBB10");
	testsInError += assertEquals(PIECE_canPlace(obtainPieceLShapeAt(0, 4, 0)), 0, "PCBB20");

	PIECE_place(obtainPieceLShapeAt(0, 3, 3));
	testsInError += BDD_assert(c, "PCBB");

	return testsInError;
}

int bddPieceFreezes() {
	int testsInError = 0;
	BddContent c = {
		"..........",
		"...%%%....",
		".....%....",
		"..........",
		".........."
	};
	BDD_clear();

	PIECE_freeze(obtainPieceLShapeAt(3, 1, 0));
	testsInError += BDD_assert(c, "PCFR");

	return testsInError;
}

int testPiece() {
	int testsInError = 0;

	//testsInError += bddPlacePieceWithOrientation0();
	//testsInError += bddPlacePieceWithOrientation1();
	//testsInError += bddPlacePieceWithOrientation2();
	//testsInError += bddPlacePieceWithOrientation3();

	//testsInError += bddClearPieceWithOrientation0();
	//testsInError += bddClearPieceWithOrientation1();
	//testsInError += bddClearPieceWithOrientation2();
	//testsInError += bddClearPieceWithOrientation3();

	testsInError += bddCanPlacePieceBesidesObstacle();
	//testsInError += bddCanPlacePieceBesidesBorderOrBottom();
	//testsInError += bddCannotPlacePieceBecauseObstacle();
	//testsInError += bddCannotPlacePieceBecauseLeftBorder();
	//testsInError += bddCannotPlacePieceBecauseRightBorder();
	//testsInError += bddCannotPlacePieceBecauseBottomBorder();

	//testsInError += bddPieceFreezes();

	return testsInError;
}

#endif
