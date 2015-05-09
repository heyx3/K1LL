#pragma once

#include "Page.h"

#include "../../Rendering/GUI/GUI Elements/GUIPanel.h"
#include "../../Rendering/GUI/GUI Elements/GUIFormattedPanel.h"
#include "../../Rendering/GUI/GUI Elements/GUISelectionBox.h"
#include "../../Rendering/GUI/GUI Elements/GUITextBox.h"


//Lets the player choose what level to edit, or choose to create a new level.
class ChooseLevelEditor : public Page
{
public:
    
    //The different possible states for this page.
    enum PageStates
    {
        //Just displaying the different levels like normal.
        PS_IDLE,
        //Entering a new name for the level about to be created.
        PS_ENTER_NEW_NAME,
        //Picking what to do to a selected level (edit or delete it).
        PS_LEVEL_OPTIONS,
        //Confirming whether to delete the selected level.
        PS_CONFIRM_DELETE,
    };


    //If there was an error creating this page, outputs an error into "outErrorMsg".
    ChooseLevelEditor(PageManager* manager, std::string& outErrorMsg);
    ~ChooseLevelEditor(void);


    PageStates GetState(void) const { return currentState; }
    void ChangeState(PageStates newState);

    //Creates a new level file with the name set in the "Enter level name" panel.
    //Returns the name of the created level (without the extension),
    //    or an empty string if a level name is invalid.
    std::string GenerateLevel(void);
    //Deletes the currently-selected level.
    void DeleteLevel(void);


    virtual void Update(Vector2i mPos, float frameSeconds) override;

    virtual void OnWindowResized(void) override;
    virtual void OnCloseWindow(void) override;

    virtual void OnWindowGainedFocus(void) override;


private:

    PageStates currentState = PS_IDLE;

    //A panel that lets the player enter a new level name.
    GUIElementPtr enterLevelName;
    GUITextBox* enterLevelTextBox;

    //A panel that lets the player decide whether to edit or delete a level.
    GUIElementPtr levelOptionsPanel, confirmDelete;

    //The "dropdown" panel that displays all the levels.
    GUIElementPtr idlePanel;
    GUISelectionBox* levelChoices;
    GUITexture *mainBackButton, *mainNewButton;
    
    
    GUIFormattedPanel* GetEnterLevelNamePanel(void) { return (GUIFormattedPanel*)enterLevelName.get(); }
    const GUIFormattedPanel* GetEnterLevelNamePanel(void) const { return (GUIFormattedPanel*)enterLevelName.get(); }
    
    GUIPanel* GetLevelOptionsPanel(void) { return (GUIPanel*)levelOptionsPanel.get(); }
    const GUIPanel* GetLevelOptionsPanel(void) const { return (GUIPanel*)levelOptionsPanel.get(); }
    
    GUIPanel* GetConfirmDeletePanel(void) { return (GUIPanel*)confirmDelete.get(); }
    const GUIPanel* GetConfirmDeletePanel(void) const { return (GUIPanel*)confirmDelete.get(); }

    GUIPanel* GetIdlePanel(void) { return (GUIPanel*)idlePanel.get(); }
    const GUIPanel* GetIdlePanel(void) const { return (GUIPanel*)idlePanel.get(); }

    void DiscoverLevelFiles(void);
    void RePositionGUI(void);
};