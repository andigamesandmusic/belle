#pragma once

#include "Belle.h"

class MusicScore : public belle::Score
{
    public:
    MusicScore();
    virtual belle::Page* NewPage();
    virtual ~MusicScore();
};
