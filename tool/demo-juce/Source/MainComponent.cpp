#include "Belle.h"
#include "MainComponent.h"
#include "BinaryData.h"
#include "MusicScore.h"
#include "MusicPage.h"

MainComponent::MainComponent()
{
    setSize (600, 776);
    score = new MusicScore();
    score->SetSpaceHeight(belle::RastralSize::Inches(6));
    score->SetSystemWidth(6.5f);
    score->SetSystemLeft(1.f);
    belle::Font NotationFont = Helper::ImportNotationFont();
    score->InitializeFont(NotationFont);
    belle::Pointer<belle::Music> Music;
    belle::String InputData = belle::String(BinaryData::bachinvention_xml);
    Music.New()->ImportXML(belle::ConvertToXML(InputData));
    score->AddSystem(Music);
    score->Wrap(score->GetSystemWidth());
    score->Engrave();
    score->Layout();
}

MainComponent::~MainComponent()
{
    delete score;
}

void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::white);
    belle::JUCE::Properties JUCESpecificProperties;
    JUCESpecificProperties.GraphicsContext = &g;
    JUCESpecificProperties.ComponentContext = this;
    JUCESpecificProperties.IndexOfCanvas = 0;
    JUCESpecificProperties.PageDimensions = belle::Inches(8.5f, 11.f);
    JUCESpecificProperties.PageVisibility = belle::BoxInt(0, 0, getWidth(), getHeight());
    JUCESpecificProperties.PageArea = belle::BoxInt(0, 0, getWidth(), getHeight());
    score->Create<belle::JUCE>(JUCESpecificProperties);
}

void MainComponent::resized()
{
}
