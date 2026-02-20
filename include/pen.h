#ifndef PEN_H
#define PEN_H

void penDown();
void penUp();
void setPenColor(int r, int g, int b);

bool penIsDown();
void getPenColor(int& r, int& g, int& b);

#endif