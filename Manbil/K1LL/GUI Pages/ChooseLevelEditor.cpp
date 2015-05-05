#include "ChooseLevelEditor.h"

#include "../../DebugAssist.h"
#include "../tinydir.h"

#include "PageManager.h"
#include "../Level Info/LevelInfo.h"


ChooseLevelEditor::ChooseLevelEditor(PageManager* manager, std::string& err)
    : Page(manager)
{
    //TODO: Build out GUI stuff. Make sure level choices dropdown is always open.

    DiscoverLevelFiles();
}

void ChooseLevelEditor::Update(Vector2i mPos, float frameSeconds)
{
    Page::Update(mPos, frameSeconds);

    GetLevelsDropdown()->SetIsExtended(true, false);
}

void ChooseLevelEditor::OnWindowResized(void)
{

}
void ChooseLevelEditor::OnCloseWindow(void)
{
    Manager->EndWorld();
}

void ChooseLevelEditor::OnWindowGainedFocus(void)
{
    Page::OnWindowGainedFocus();
    DiscoverLevelFiles();
}
void ChooseLevelEditor::DiscoverLevelFiles(void)
{
    //Find the available levels.
    std::vector<std::string> items;

    tinydir_dir dir;
    tinydir_open(&dir, LevelInfo::LevelFilesPath.c_str());
    std::cout << "------------------------------\n";

    while (dir.has_next)
    {
        tinydir_file file;
        tinydir_readfile(&dir, &file);
        std::cout << file.name << "\n";

        //Remove the extension from the name and add it to the list.
        std::string levelName = file.name;
        if (levelName.substr(levelName.size() - 4, 4) == ".lvl")
        {
            items.push_back(levelName.substr(0, levelName.size() - 4));
        }

        tinydir_next(&dir);
    }

    tinydir_close(&dir);
    std::cout << "-----------------------------\n\n\n";


    //Now reset the dropdown box.
    std::string err;
    GetLevelsDropdown()->ResetItems(items, err);
    if (!Assert(err.empty(), "Error creating new item boxes", err))
    {
        return;
    }
}