//
// Created by benyamin on 2/12/2026.
//

#ifndef PROJE_MABANI_LOGIC_H
#define PROJE_MABANI_LOGIC_H


    spr->x += steps * sin(DEG_TO_RAD(spr->direction));
    spr->y -= steps * cos(DEG_TO_RAD(spr->direction));
}

void normalizeDirection(Sprite* spr) {
    spr->direction = fmod(spr->direction, 360.0);
    if (spr->direction < 0) spr->direction += 360.0;
}

void turnRight(Sprite* spr, double degrees) {
    spr->direction += degrees;
    normalizeDirection(spr);
}

void turnLeft(Sprite* spr, double degrees) {
    spr->direction -= degrees;
    normalizeDirection(spr);
}

void goToXY(Sprite* spr, double newX, double newY) {
    spr->x = newX;
    spr->y = newY;
}
void changeX(Sprite* spr, double amount) {
    spr->x += amount;
}

void setX(Sprite* spr, double newX) {
    spr->x = newX;
}

void changeY(Sprite* spr, double amount) {
    spr->y -= amount;
}

void pointInDirection(Sprite* spr, double newDirection) {
    spr->direction = newDirection;
    normalizeDirection(spr);
}

class logic {

};


#endif //PROJE_MABANI_LOGIC_H
