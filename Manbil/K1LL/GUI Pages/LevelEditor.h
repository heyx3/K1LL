#pragma once

#include "Page.h"


class LevelEditor : public Page
{
public:

    LevelEditor(const std::string& levelFileName, PageManager* manager);
    ~LevelEditor(void);
};