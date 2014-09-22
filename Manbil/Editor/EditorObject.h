#pragma once

#include <string>
#include "../Rendering/GUI/GUIElement.h"
#include "EditorMaterialSet.h"


//A (usually modifiable) piece of data to be displayed in the editor.
struct EditorObject
{
public:

    static std::string ErrorMsg;


    EditorObject(void) : activeGUIElement(0) { }
    virtual ~EditorObject(void) { }


    //Gets the currently-active GUIElement.
    GUIElementPtr GetActiveGUIElement(void) const { return activeGUIElement; }

    //Initializes the GUIElement for representing/editing this data.
    //If there was a problem, returns false and sets the static "ErrorMsg" field. Otherwise returns true.
    //Takes in the content to use for rendering.
    virtual bool InitGUIElement(EditorMaterialSet & materialSet) = 0;
    //Updates this object and its element's properties. Does NOT need to call Update() for this object's active GUI element.
    //Returns whether the active GUI element was moved or resized at all
    //   (i.e. whether the formatted panel has to rebuild its layout).
    //Default behavior: do nothing, return false.
    virtual bool Update(float elapsed, Vector2f panelRelativeMousePos) { return false; }

protected:

    GUIElementPtr activeGUIElement;
};


typedef std::shared_ptr<EditorObject> EditorObjectPtr;