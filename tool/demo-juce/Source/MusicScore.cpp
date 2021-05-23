#include "MusicScore.h"
#include "MusicPage.h"

MusicScore::MusicScore() {}
MusicScore::~MusicScore() {}
belle::Page* MusicScore::NewPage() {return new MusicPage;}
