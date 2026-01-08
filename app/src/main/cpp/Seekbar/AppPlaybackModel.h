#pragma once

#include "PlayerState.h"
#include "SeekbarState.h"
#include "Chapters.h"

struct AppPlaybackModel {
    PlayerState player;
    SeekbarState seek;
    std::vector<Chapter> chapters;
};
