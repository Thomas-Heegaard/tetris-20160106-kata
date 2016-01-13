#include <string.h>
#include "main.h"
#include "t6963c.h"
#include "bdd.h"
#include "board.h"

#define ROM_CG_ADDRESS 0x0000

/**
 * Dessine un rectangle entre les coordonnées spécifiées.
 * Le carré est dessiné avec des caractères BORDER_*, pour
 * que le serpent ne puisse pas le traverser.
 * @param x0, y0: Coordonnées de l'angle supérieur gauche.
 * @param x1, y1: Coordonnées de l'angle inférieur droit.
 */
void BOARD_draw(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1) 
{
	int i;
    T6963C_writeAt(x0, y0, BORDER_A);
    T6963C_writeAt(x0, y1, BORDER_F);
    T6963C_writeAt(x1, y0, BORDER_C);
    T6963C_writeAt(x1, y1, BORDER_H);
    for(i = x0 + 1; i < x1; i++)
    {
        T6963C_writeAt(i, y0, BORDER_B);
        T6963C_writeAt(i, y1, BORDER_G);
    }
    for(i = y0 + 1; i < y1; i++)
    {
        T6963C_writeAt(x0, i, BORDER_D);
        T6963C_writeAt(x1, i, BORDER_E);
    }
}

/**
 * Remplit avec des espaces le rectangle défini par les coordonnées.
 * Permet de nettoyer l'intérieur du rectangle dessiné avec BOARD_draw.
 * @param x0, y0: Coordonnées de l'angle supérieur droit.
 * @param x1, y1: Coordonnées de l'angle inférieur gauche.
 */
void BOARD_clear(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1) {
	// OK
   int i;
   int j;

   for(j = x0; j <= x1; j++)
   {
      for(i = y0; i <= y1; i++)
      {
	 T6963C_writeAt(j, i, EMPTY);
      }
   }

}

/**
 * Copie la définition d'un caractère depuis la ROM vers la zone de définition
 * des caractères du T6963C.
 * @param positionInRom Position du caractère dans la ROM.
 * @param cgCode Code du caractère dans le t6963c.
 */
void BOARD_copyFromRomToCg(unsigned char positionInRom, unsigned char cgCode) {
	unsigned char *rom_cg_address = (unsigned char __xdata *) (ROM_CG_ADDRESS + positionInRom * 8);
	unsigned int video_cg_address = cgCode * 8 + T6963C_ZONE_CG;
	T6963C_autoWrite(video_cg_address, rom_cg_address, 8);
}

/**
 * Initialise les caractères utilisés pendant le jeu.
 */
void BOARD_initialize() {
	BOARD_copyFromRomToCg( 0, BORDER_A);
	BOARD_copyFromRomToCg( 1, BORDER_B);
	BOARD_copyFromRomToCg( 2, BORDER_C);
	BOARD_copyFromRomToCg( 3, BORDER_D);
	BOARD_copyFromRomToCg( 4, BORDER_E);
	BOARD_copyFromRomToCg( 5, BORDER_F);
	BOARD_copyFromRomToCg( 6, BORDER_G);
	BOARD_copyFromRomToCg( 7, BORDER_H);

	BOARD_copyFromRomToCg( 8, OBSTACLE);
	BOARD_copyFromRomToCg( 9, PIECE);

	BOARD_clear(TETRIS_LIMIT_X0 - 1, TETRIS_LIMIT_Y0 - 1, TETRIS_LIMIT_X1 + 1, TETRIS_LIMIT_Y1 + 1);
	BOARD_draw(TETRIS_LIMIT_X0 - 1, TETRIS_LIMIT_Y0 - 1, TETRIS_LIMIT_X1 + 1, TETRIS_LIMIT_Y1 + 1);
}

// Efface une ligne complète du tableau de jeu, et
// fait descendre de 1 cran tous les obstacles qui se
// trouvent au dessus.
void dropLine(unsigned char line) {

   int i, j;

   //	Décalage de la ligne 'line'
    for(j = line + TETRIS_LIMIT_Y0; j >= TETRIS_LIMIT_Y0; j--)
        for(i = TETRIS_LIMIT_X0; i <= TETRIS_LIMIT_X1; i++)
           T6963C_writeAt(i, j, T6963C_readFrom(i, j - 1));
	
   //	Efface la ligne du haut
    for(i = TETRIS_LIMIT_X0; i <= TETRIS_LIMIT_X1; i++)
        T6963C_writeAt(i, TETRIS_LIMIT_Y0, EMPTY);	
}

/**
 * Efface toutes les lignes entières et fait descendre
 * les obstacles au dessus de 1 niveau.
 */
void BOARD_clearSolidRows() {
   
   unsigned char x, y;
   unsigned char count = TETRIS_LIMIT_X0;

   
   for(y = TETRIS_LIMIT_Y0; y <= TETRIS_LIMIT_Y1; y++)
   {
      count = TETRIS_LIMIT_X0;
      
      for(x = TETRIS_LIMIT_X0; x <= TETRIS_LIMIT_X1; x++)
      {
	 if(T6963C_readFrom(x, y) != EMPTY) count++;
      }
      if(count == TETRIS_LIMIT_X1-1) dropLine(y - TETRIS_LIMIT_Y0);
   }
   
   
}

/**
 * Affiche un texte entouré d'un rectangle.
 * La largeur du rectangle est définie par la longueur du texte.
 * La fonction ne vérifie pas si le texte dépasse les bords de l'écran.
 * @param x0, y0 L'angle supérieur droit du rectangle.
 * @param text Le texte à afficher.
 */
void BOARD_display(unsigned char x0, unsigned char y0, char *text) {
	int NbCaract = 0;
	int x0start = x0;
	// calcul du nombre de caractere
	while (text[NbCaract] != '\0')
	{
		NbCaract ++;
	}
	// ecriture du cadre superieur
	T6963C_writeAt(x0,y0,BORDER_A);
	x0++;
	T6963C_autoRepeat(T6963C_calculateAddress(x0,y0),BORDER_B, NbCaract);
	x0 += NbCaract;
	T6963C_writeAt(x0,y0,BORDER_C);
	
	// ecriture de la ligne de text avec cadre
	x0 = x0start;
	y0++;
	T6963C_writeAt(x0,y0,BORDER_D);
	x0++;
	T6963C_autoWrite(T6963C_calculateAddress(x0,y0), text, NbCaract);
	x0 += NbCaract;
	T6963C_writeAt(x0,y0,BORDER_E);
	
	// ecriture du cadre inferrieur
	x0 = x0start;
	y0++;
	T6963C_writeAt(x0,y0,BORDER_F);
	x0++;
	T6963C_autoRepeat(T6963C_calculateAddress(x0,y0),BORDER_G, NbCaract);
	x0 += NbCaract;
	T6963C_writeAt(x0,y0,BORDER_H);
}

#ifdef TEST
int bddBoardDraw() {
	BddContent c = {
		"ABBBBBBBBC",
		"D........E",
		"D........E",
		"D........E",
		"FGGGGGGGGH"
	};

	BDD_clear();
	BOARD_draw(BDD_SCREEN_X, BDD_SCREEN_Y, BDD_SCREEN_X + BDD_SCREEN_WIDTH - 1, BDD_SCREEN_Y + BDD_SCREEN_HEIGHT - 1);
	return BDD_assert(c, "BOD");
}

int bddBoardClear() {
	BddContent c = {
		"..........",
		".        .",
		".        .",
		".        .",
		".........."
	};

	BDD_clear();
	BOARD_clear(BDD_SCREEN_X + 1, BDD_SCREEN_Y + 1, BDD_SCREEN_X + BDD_SCREEN_WIDTH - 2, BDD_SCREEN_Y + BDD_SCREEN_HEIGHT - 2);
	return BDD_assert(c, "BOC");
}

int bddBoardDisplay() {
	BddContent c = {
		"..........",
		".ABBBBBC..",
		".D TXT E..",
		".FGGGGGH..",
		".........."
	};

	BDD_clear();
	BOARD_display(BDD_SCREEN_X + 1, BDD_SCREEN_Y + 1, " TXT ");
	return BDD_assert(c, "BOD");
}

int bddClearSolidRows1() {
	BddContent initial = {
		"%%%%%%%%%%",
		"%%%%%%%%% ",
		"%%%% %%%%%",
		" %%%%%%%%%",
		"%%%%%%%%%%"
	};
	BddContent expected = {
		"          ",
		"          ",
		"%%%%%%%%% ",
		"%%%% %%%%%",
		" %%%%%%%%%"
	};

	BDD_initialize(initial);
	BOARD_clearSolidRows();
	return BDD_assert(expected, "BOCLSR1");
}

int bddClearSolidRows2() {
	BddContent initial = {
		"%% % %  %%",
		" %% %%%%% ",
		"%%%%%%%%%%",
		" %    %%%%",
		"%%%%%%%%%%"
	};
	BddContent expected = {
		"          ",
		"          ",
		"%% % %  %%",
		" %% %%%%% ",
		" %    %%%%"
	};

	BDD_initialize(initial);
	BOARD_clearSolidRows();
	return BDD_assert(expected, "BOCLSR2");
}


int bddClearClearLine1() {
	BddContent initial = {
		"%% % %  %%",
		" %% %%%%% ",
		"%%%%%%%%%%",
		" %    %%%%",
		"%%%%%%%%%%"
	};
	BddContent expected = {
		"          ",
		"%% % %  %%",
		" %% %%%%% ",
		" %    %%%%",
		"%%%%%%%%%%"
	};

	BDD_initialize(initial);
	dropLine(2);
	return BDD_assert(expected, "BODRPLN1");
}

int testBoard() {
	int testsInError = 0;

	//testsInError += bddBoardDraw();
	//testsInError += bddBoardClear();
	//testsInError += bddBoardDisplay();
	testsInError += bddClearClearLine1();
	testsInError += bddClearSolidRows1();
	testsInError += bddClearSolidRows2();
	return testsInError;
}
#endif
