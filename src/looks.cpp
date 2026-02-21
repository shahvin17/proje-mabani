#include "looks.h"
#include <iostream>

void show(Looks& l) {
    l.visible = true;
}

void hide(Looks& l) {
    l.visible = false;
}

void printLooksStatus(const Looks& l) {
    std::cout << "Visible: " << (l.visible ? "Yes" : "No") << " | Layer: " << l.layer << std::endl;
}