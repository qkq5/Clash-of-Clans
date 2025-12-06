#include "Building.h"

void Building::takeDamage(int damage){ if(damage<=0) return; hp = std::max(0, hp - damage); }
bool Building::isDestroyed() const{ return hp <= 0; }
