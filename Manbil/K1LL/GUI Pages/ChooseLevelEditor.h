#pragma once

#include "Page.h"

#include "../../Rendering/GUI/GUI Elements/GUIPanel.h"
#include "../../Rendering/GUI/GUI Elements/GUISelectionBox.h"


//Lets the player choose what level to edit, or choose to create a new level.
class ChooseLevelEditor : public Page
{
public:

    //If there was an error creating this page, outputs an error into "outErrorMsg".
    ChooseLevelEditor(PageManager* manager, std::string& outErrorMsg);


    virtual void Update(Vector2i mPos, float frameSeconds) override;

    virtual void OnWindowResized(void) override;
    virtual void OnCloseWindow(void) override;

    virtual void OnWindowGainedFocus(void) override;

private:

    //The different possible states for this page.
    enum PageStates
    {
        //Just displaying the different levels like normal.
        PS_IDLE,
        //Entering a new name for the level about to be created.
        PS_ENTER_NEW_NAME,
        //Picking what to do to a selected level (edit or delete it).
        PS_LEVEL_OPTIONS,
    };

    PageStates CurrentState = PS_IDLE;

    //A panel that lets the player enter a new level name.
    GUIElementPtr enterLevelName;

    //A panel that lets the player decide whether to edit, copy, or delete a level.
    GUIElementPtr levelOptionsPanel;

    //The "dropdown" panel that displays all the levels.
    GUIElementPtr levelChoicesDropdown;
    
    
    GUIPanel* GetEnterLevelNamePanel(void) { return (GUIPanel*)enterLevelName.get(); }
    const GUIPanel* GetEnterLevelNamePanel(void) const { return (GUIPanel*)enterLevelName.get(); }
    
    GUIPanel* GetLevelOptionsPanel(void) { return (GUIPanel*)levelOptionsPanel.get(); }
    const GUIPanel* GetLevelOptionsPanel(void) const { return (GUIPanel*)levelOptionsPanel.get(); }
    
    GUISelectionBox* GetLevelsDropdown(void) { return (GUISelectionBox*)levelChoicesDropdown.get(); }
    const GUISelectionBox* GetLevelsDropdown(void) const { return (GUISelectionBox*)levelChoicesDropdown.get(); }

    void DiscoverLevelFiles(void);
    void RePositionGUI(void);
};