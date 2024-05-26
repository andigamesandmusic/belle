#pragma once

#include "Belle.h"
#include "MusicScore.h"

class MusicPage : public belle::Page
{
    public:
    void drawText(belle::Painter& Painter, belle::Portfolio& Portfolio)
    {
        MusicScore& ScoreRef = dynamic_cast<MusicScore&>(Portfolio);
        belle::ScopedAffine a(Painter, belle::Affine::Translate(belle::Vector(1.f, 10.5f)));
        Painter.Draw(
            "Music engraving with JUCE!",
            ScoreRef.GetFont(),
            12.f,
            belle::Font::Bold,
            belle::Text::Justifications::Left,
            0.f
        );
    }

    virtual void Paint(belle::Painter& Painter, belle::Portfolio& Portfolio)
    {
        drawText(Painter, Portfolio);
        Page::Paint(Painter, Portfolio);
    }

    virtual ~MusicPage();
};
