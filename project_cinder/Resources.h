#pragma once

#include "cinder/CinderResources.h"

// https://github.com/solarus-games/solarus-alttp-pack
#define HERO_WALKING_IMG CINDER_RESOURCE( resources/tilesets/, walking.png, 101, IMAGE )
#define HERO_WALKING_DAT CINDER_RESOURCE( resources/animations/, walking.json, 102, TEXT )
#define VERTEX_SHADER CINDER_RESOURCE( resources/shaders/, instanced.vert, 103, TEXT )
#define FRAMGENT_SHADER CINDER_RESOURCE( resources/shaders/, instanced.frag, 104, TEXT )
