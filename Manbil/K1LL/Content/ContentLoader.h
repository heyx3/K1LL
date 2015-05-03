#pragma once

#include <string>


//Handles initialization of all content files.
class ContentLoader
{
public:

    static void LoadContent(std::string& outErrorMsg);
    static void DestroyContent(void);

private:
    ContentLoader(void) { }
};