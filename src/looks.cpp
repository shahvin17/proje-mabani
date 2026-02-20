#include "looks.h"
#include "render.h"
#include <iostream>
using namespace std;

void looks_say(Runtime* rt, Block* block) {
    if (!rt || !block) return;

    if (block->inputs.empty()) return;

    std::string text = std::to_string(block->inputs[0]);

    std::cout << "[looks] say block executed: " << text << std::endl;

    renderSayText(text, 100, 100);
}