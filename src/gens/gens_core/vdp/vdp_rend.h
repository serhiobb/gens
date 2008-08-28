#ifndef VDP_REND_H
#define VDP_REND_H

// 16-bit color
extern unsigned short MD_Screen[336 * 240];
extern unsigned short Palette[0x1000];
extern unsigned short MD_Palette[256];

// 32-bit color
extern unsigned int MD_Screen32[336 * 240];
extern unsigned int Palette32[0x1000];
extern unsigned int MD_Palette32[256];

extern unsigned long TAB336[336];

extern struct
{
	int Pos_X;
	int Pos_Y;
	unsigned int Size_X;
	unsigned int Size_Y;
	int Pos_X_Max;
	int Pos_Y_Max;
	unsigned int Num_Tile;
	int dirt;
} Sprite_Struct[256];

extern int Mode_555;
extern int Sprite_Over;

void Render_Line();
void Render_Line_32X();

#endif
