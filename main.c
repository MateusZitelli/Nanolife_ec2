//Nanolife - Simple artificial life simulator

//Copyright 2011 Mateus Zitelli <zitellimateus@gmail.com>

//This program is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
//MA 02110-1301, USA.

#include <stdio.h>
#include <stdlib.h>
#define SX 200
#define SY 200
#define WIDTH 200
#define HEIGHT 200

#define MEM_SIZE 128
#define MAX_AGE 600.0

int last;
struct bot *bots;

/*
0x0 - ptr++
0x1 - ptr--
0x2 - memory[b->ptr]++
0x3 - memory[b->ptr]--
0x4 - while(memory[b->ptr]){
0x5 - }
0x6 - memory[b->ptr] = 1 if have a compatible bot in front or 0 if not
0x7 - memory[b->ptr] = dir
0x8 - Act;
*/

/*Actions dict:
0x0 - Do nothing
0x1 - Rotate clockwise - dir++
0x2 - Rotate anticlockwise - dir--
0x3 - Move foward
0x4 - Move Back
0x5 - Reproduce
0x6 - Atack
0x7 - Divide energy
*/

struct bot {
	int p;
	float energy;
	short gcode[MEM_SIZE];
	short memory[MEM_SIZE];
	unsigned short nl;
	unsigned short loops[MEM_SIZE];
	unsigned short loops_ptr[MEM_SIZE];
	short ptr;
	short pos;
	unsigned short dir;
	short r;
	short g;
	short b;
	int age;
	int generation;
};

    /* The Itoa code is in the puiblic domain */
char *itoa(int value, char *str, int radix)
{
	static char dig[] = "0123456789" "abcdefghijklmnopqrstuvwxyz";
	int n = 0, neg = 0;
	unsigned int v;
	char *p, *q;
	char c;

	if (radix == 10 && value < 0) {
		value = -value;
		neg = 1;
	}
	v = value;
	do {
		str[n++] = dig[v % radix];
		v /= radix;
	} while (v);
	if (neg)
		str[n++] = '-';
	str[n] = '\0';

	for (p = str, q = p + (n - 1); p < q; ++p, --q)
		c = *p, *p = *q, *q = c;
	return str;
}

void
set_bot(struct bot *b, int p, float e, unsigned short *g, struct bot **lb,
	short gen)
{
	short i;
	int cr = 0, cg = 0, cb = 0;
	if (e <= 0)
		return;
	b->p = p;
	b->energy = e;
	for (i = 0; i < MEM_SIZE; i++) {
		b->gcode[i] = g[i];
		b->memory[i] = 0;
	}
	b->nl = 0;
	b->ptr = 0;
	b->pos = 0;
	b->dir = rand() % 4;
	for (i = 0; i < MEM_SIZE / 3; i++) {
		cr += g[i];
	}
	for (i = MEM_SIZE / 3; i < MEM_SIZE / 3 * 2; i++) {
		cg += g[i];
	}
	for (i = MEM_SIZE / 3 * 2; i < MEM_SIZE; i++) {
		cb += g[i];
	}
	b->r = ((int)((float)cr / (MEM_SIZE / 3 * 8) * 256) % 256 - 64) * 2;
	b->g = ((int)((float)cg / (MEM_SIZE / 3 * 8) * 256) % 256 - 64) * 2;
	b->b = ((int)((float)cb / (MEM_SIZE / 3 * 8) * 256) % 256 - 64) * 2;
	b->age = MAX_AGE;
	b->generation = gen + 1;
}

char compatible(struct bot *b1, struct bot *b2)
{
	int pos, i;
	for (i = 0; i < 4; i++) {
		pos = rand() % MEM_SIZE;
		if (b1->gcode[pos] != b2->gcode[pos])
			return (0);
	}
	return (1);
}

float compatibility(short *gcode, struct bot *b)
{
	int i;
	float val = 0;
	for (i = 0; i < MEM_SIZE; i++) {
		if (gcode[i] == b->gcode[i])
			val += 1;
	}
	return (val/MEM_SIZE);
}

void run(struct bot *b, struct bot **lb)
{
	--b->energy;
	//printf("%i -> %i\n", b->ptr, b->memory[b->ptr]);
	unsigned short ngcode[MEM_SIZE];
	int mean, p, index = MEM_SIZE / 2, i;
	switch (b->memory[b->ptr]) {
	case 1:
		b->dir = (b->dir + 1) % 4;
		break;
	case 2:
		b->dir = (b->dir - 1) % 4;
		break;
	case 3:
		switch (b->dir) {
		case 0:
			if (b->p + 1 < SX * SY && lb[b->p + 1] == NULL) {
				lb[b->p] = NULL;
				lb[b->p + 1] = b;
				b->p = b->p + 1;
			}
			break;
		case 1:
			if (b->p - SX >= 0 && lb[b->p - SX] == NULL) {
				lb[b->p] = NULL;
				lb[b->p - SX] = b;
				b->p = b->p - SX;
			}
			break;
		case 2:
			if (b->p - 1 >= 0 && lb[b->p - 1] == NULL) {
				lb[b->p] = NULL;
				lb[b->p - 1] = b;
				b->p = b->p - 1;
			}
			break;
		case 3:
			if (b->p + SX < SX * SY && lb[b->p + SX] == NULL) {
				lb[b->p] = NULL;
				lb[b->p + SX] = b;
				b->p = b->p + SX;
			}
			break;
		}
		break;
	case 4:
		switch (b->dir) {
		case 2:
			if (b->p + 1 < SX * SY && lb[b->p + 1] == NULL) {
				lb[b->p] = NULL;
				lb[b->p + 1] = b;
				b->p = b->p + 1;
			}
			break;
		case 3:
			if (b->p - SX >= 0 && lb[b->p - SX] == NULL) {
				lb[b->p] = NULL;
				lb[b->p - SX] = b;
				b->p = b->p - SX;
			}
			break;
		case 0:
			if (b->p - 1 >= 0 && lb[b->p - 1] == NULL) {
				lb[b->p] = NULL;
				lb[b->p - 1] = b;
				b->p = b->p - 1;
			}
			break;
		case 1:
			if (b->p + SX < SX * SY && lb[b->p + SX] == NULL) {
				lb[b->p] = NULL;
				lb[b->p + SX] = b;
				b->p = b->p + SX;
			}
			break;
		}
		break;
	case 5:
		for (i = 0; i < MEM_SIZE; i++) {
			ngcode[i] = b->gcode[i];
		}
		for(i = 0; i < 100; i++){
			if (rand() % 1000 > 400)
				ngcode[rand() % MEM_SIZE] = rand() % 9;
			else
				break;
		}
		switch (b->dir) {
		case 0:
			if (b->p + 1 < SX * SY && lb[b->p + 1] == NULL) {
				lb[b->p + 1] = &bots[last];
				set_bot(&bots[last++], b->p + 1,
					b->energy / 5.0, ngcode, lb,
					b->generation);
				b->energy -= b->energy / 5.0;
			}
			break;
		case 1:
			if (b->p - SX >= 0 && lb[b->p - SX] == NULL) {
				lb[b->p - SX] = &bots[last];
				set_bot(&bots[last++], b->p - SX,
					b->energy / 5.0, ngcode, lb,
					b->generation);
				b->energy -= b->energy / 5.0;
			}
			break;
		case 2:
			if (b->p - 1 >= 0 && lb[b->p - 1] == NULL) {
				lb[b->p - 1] = &bots[last];
				set_bot(&bots[last++], b->p - 1,
					b->energy / 5.0, ngcode, lb,
					b->generation);
				b->energy -= b->energy / 5.0;
			}
			break;
		case 3:
			if (b->p + SX < SX * SY && lb[b->p + SX] == NULL) {
				lb[b->p + SX] = &bots[last];
				set_bot(&bots[last++], b->p + SX,
					b->energy / 5.0, ngcode, lb,
					b->generation);
				b->energy -= b->energy / 5.0;
			}
			break;
		}
		break;
	case 6:
		switch (b->dir) {
		case 0:
			if (b->p + 1 < SX * SY && lb[b->p + 1] != NULL) {
				b->energy += lb[b->p + 1]->energy / 2.0;
				lb[b->p + 1]->energy /= 2.0;
			}
			break;
		case 1:
			if (b->p - SX >= 0 && lb[b->p - SX] != NULL) {
				b->energy += lb[b->p - SX]->energy / 2.0;
				lb[b->p - SX]->energy /= 2.0;
			}
			break;
		case 2:
			if (b->p - 1 >= 0 && lb[b->p - 1] != NULL) {
				b->energy += lb[b->p - 1]->energy / 2.0;
				lb[b->p - 1]->energy /= 2.0;
			}
			break;
		case 3:
			if (b->p + SX < SX * SY && lb[b->p + SX] != NULL) {
				b->energy += lb[b->p + SX]->energy / 2.0;
				lb[b->p + SX]->energy /= 2.0;
			}
			break;
		}
		break;
	case 7:
		switch (b->dir) {
		case 0:
			if (b->p + 1 < SX * SY && lb[b->p + 1] != NULL
			    && compatible(lb[b->p + 1], b)) {
				mean = (b->energy + lb[b->p + 1]->energy) / 2.0;
				b->energy = mean;
				lb[b->p + 1]->energy = mean;
			}
			break;
		case 1:
			if (b->p - SX >= 0 && lb[b->p - SX] != NULL
			    && compatible(lb[b->p - SX], b)) {
				mean =
				    (b->energy + lb[b->p - SX]->energy) / 2.0;
				b->energy = mean;
				lb[b->p - SX]->energy = mean;
			}
			break;
		case 2:
			if (b->p - 1 >= 0 && lb[b->p - 1] != NULL
			    && compatible(lb[b->p - 1], b)) {
				mean = (b->energy + lb[b->p - 1]->energy) / 2.0;
				b->energy = mean;
				lb[b->p - 1]->energy = mean;
			}
			break;
		case 3:
			if (b->p + SX < SX * SY && lb[b->p + SX] != NULL
			    && compatible(lb[b->p + SX], b)) {
				mean =
				    (b->energy + lb[b->p + SX]->energy) / 2.0;
				b->energy = mean;
				lb[b->p + SX]->energy = mean;
			}
			break;
		}
		break;
  case 8:
		switch (b->dir) {
		index = rand() % MEM_SIZE;
		case 0:
			if (b->p + 1 < SX * SY && b->p + SX < SX * SY && lb[b->p + 1] != NULL && lb[b->p + SX] == NULL){
			    //&& compatible(lb[b->p + 1], b)) {
				for(i = 0; i < index; i++){
					ngcode[i] = b->gcode[i];
				}
				for(i = index; i < MEM_SIZE; i++){
				  ngcode[i] = lb[b->p + 1]->gcode[i];
				}
				set_bot(&bots[last++], b->p + SX,
					b->energy / 5.0 + lb[b->p + 1]->energy / 5.0, ngcode, lb,
					b->generation > lb[b->p + 1]->generation ? b->generation:lb[b->p + 1]->generation);
				b->energy -= b->energy / 5.0;
				lb[b->p + 1]->energy -= lb[b->p + 1]->energy/5.0;
			}
			break;
		case 1:
			if (b->p - SX >= 0 && b->p + 1 < SX * SY && lb[b->p - SX] != NULL && lb[b->p + 1] == NULL){
			    //&& compatible(lb[b->p - SX], b)) {
				for(i = 0; i < index; i++){
					ngcode[i] = b->gcode[i];
				}
				for(i = index; i < MEM_SIZE; i++){
				  ngcode[i] = lb[b->p - SX]->gcode[i];
				}
				set_bot(&bots[last++], b->p + 1,
					b->energy / 5.0 + lb[b->p - SX]->energy / 5.0, ngcode, lb,
					b->generation > lb[b->p - SX]->generation ? b->generation:lb[b->p - SX]->generation);
				b->energy -= b->energy / 5.0;
				lb[b->p - SX]->energy -= lb[b->p - SX]->energy/5.0;
			}
			break;
		case 2:
			if (b->p - 1 >= 0 && b->p - SX >= 0 && lb[b->p - 1] != NULL && lb[b->p - SX] == NULL){
			    //&& compatible(lb[b->p - 1], b)) {
				for(i = 0; i < index; i++){
					ngcode[i] = b->gcode[i];
				}
				for(i = index; i < MEM_SIZE; i++){
				  ngcode[i] = lb[b->p - 1]->gcode[i];
				}
				set_bot(&bots[last++], b->p - SX,
					b->energy / 5.0 + lb[b->p - 1]->energy / 5.0, ngcode, lb,
					b->generation > lb[b->p - 1]->generation ? b->generation:lb[b->p - 1]->generation);
				b->energy -= b->energy / 5.0;
				lb[b->p - 1]->energy -= lb[b->p - 1]->energy/5.0;
			}
			break;
		case 3:
			if (b->p + SX < SX * SY && b->p - 1 >= 0 && lb[b->p + SX] != NULL && lb[b->p - 1] == NULL){
			    //&& compatible(lb[b->p + SX], b)) {
				for(i = 0; i < index; i++){
					ngcode[i] = b->gcode[i];
				}
				for(i = index; i < MEM_SIZE; i++){
				  ngcode[i] = lb[b->p + SX]->gcode[i];
				}
				set_bot(&bots[last++], b->p - 1,
					b->energy / 5.0 + lb[b->p + SX]->energy / 5.0, ngcode, lb,
					b->generation > lb[b->p + SX]->generation ? b->generation:lb[b->p + SX]->generation);
				b->energy -= b->energy / 5.0;
				lb[b->p + SX]->energy -= lb[b->p + SX]->energy/5.0;
			}
			break;
		}
		break;
	}
}

void compute(struct bot *b, struct bot **lb)
{
	--b->age;
	if (b->pos > MEM_SIZE || b->ptr > MEM_SIZE || b->ptr < 0
	    || b->nl > MEM_SIZE)
		return;
	switch (b->gcode[b->pos++]) {
	case 0:
		b->ptr++;
		break;
	case 1:
		b->ptr--;
		break;
	case 2:
		b->memory[b->ptr]++;
		break;
	case 3:
		b->memory[b->ptr]--;
		break;
	case 4:
		if (b->memory[b->ptr])
			b->loops[b->nl] = b->pos;
		b->loops_ptr[b->nl++] = b->ptr;
		break;
	case 5:
		if (b->nl && b->memory[b->loops_ptr[b->nl - 1]]) {
			b->pos = b->loops[b->nl - 1];
		} else {
			--b->nl;
		}
		break;
	case 6:
		switch (b->dir) {
		case 0:
			if (b->p + 1 < SX * SY && lb[b->p + 1] != NULL
			    && compatible(lb[b->p + 1], b)) {
				b->memory[b->ptr] = 1;
			} else {
				b->memory[b->ptr] = 0;
			}
			break;
		case 1:
			if (b->p - SX >= 0 && lb[b->p - SX] != NULL
			    && compatible(lb[b->p - SX], b)) {
				b->memory[b->ptr] = 1;
			} else {
				b->memory[b->ptr] = 0;
			}
			break;
		case 2:
			if (b->p - 1 >= 0 && lb[b->p - 1] != NULL
			    && compatible(lb[b->p - 1], b)) {
				b->memory[b->ptr] = 1;
			} else {
				b->memory[b->ptr] = 0;
			}
			break;
		case 3:
			if (b->p + SX < SX * SY && lb[b->p + SX] != NULL
			    && compatible(lb[b->p + SX], b)) {
				b->memory[b->ptr] = 1;
			} else {
				b->memory[b->ptr] = 0;
			}
			break;
		}
		break;
	case 7:
		b->memory[b->ptr] = b->dir;
		break;
	case 8:
		run(b, lb);
		break;
	}
	//b->memory[b->ptr] = b->memory[b->ptr] % 9;
}

int main(void)
{
	srand(time(0));
	FILE *file;
	file = fopen("file.txt","w");
	last = 0;
	int keypress = 0,h = 0, k = 0,i, px, py, j, food = 50;
	long long b = 0, v = 0, a = MAX_AGE;
	unsigned short g[MEM_SIZE];
	short selected[MEM_SIZE];
	bots = (struct bot *)malloc(sizeof(struct bot) * SX * SY);
	struct bot **lb = (struct bot **)malloc(sizeof(struct bot *) * SX * SY);
	for (i = 0; i < SX * SY; i++) {
		lb[i] = NULL;
	}
	for (j = 0; j < 5000; j++) {
		px = rand() % WIDTH;
		py = rand() % HEIGHT;
		for (i = 0; i < MEM_SIZE; i++) {
			g[i] = rand() % 9;
		}
		set_bot(&bots[last++], py * SX + px, 10000, g, lb, 0);
	}
	short get = 0, view = 0;
	float comp;
	char buf[20];
	while (!keypress) {
		++k;
		for (i = 0; i < last; i++) {
			compute(&bots[i], lb);
		}
		for (i = 0; i < SX * SY; i++) {
			lb[i] = NULL;
		}
		for (i = 0; i < last; i++) {
			if (bots[i].energy > 0 && bots[i].age > 0) {
				lb[bots[i].p] = &bots[i];
			} else {
				bots[i] = bots[--last];
			}
			if (k % 1000 == 0) {
				if (bots[i].energy > v
				    && bots[i].generation > 20) {
					b = i;
					v = bots[i].energy;
				}
			}
		}
		if (k % 1000 == 0) {
			for (i = 0; i < MEM_SIZE - 1; i++) {
				fprintf(file, "%i, ", bots[b].gcode[i]);
			}
			fprintf(file, "%i\n", bots[b].gcode[i]);
			v = 0;
			printf("%i %i\n", k / 1000, last);
		}
		for (j = 0; rand() % 100 < food; j++) {
			px = rand() % WIDTH;
			py = rand() % HEIGHT;
			for (i = 0; i < MEM_SIZE; i++) {
				g[i] = rand() % 9;
			}
			if (lb[py * SX + px] == NULL)
				set_bot(&bots[last++], py * SX + px, 10000, g,
					lb, 0);
		}
	}
	return (0);
}

