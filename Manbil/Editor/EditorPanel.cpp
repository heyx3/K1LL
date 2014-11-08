#include "EditorPanel.h"


std::string EditorPanel::AddObject(EditorObjectPtr toAdd, unsigned int index)
{
    if (toAdd->InitGUIElement(MaterialSet))
    {
        editorObjects.insert(editorObjects.begin() + index, toAdd);
        panel.AddObject(GUIFormatObject(toAdd->GetActiveGUIElement(), toAdd->GetMoveHorizontally(),
                                        toAdd->GetMoveVertically(), toAdd->Offset));
        return "";
    }
    else
    {
        return EditorObject::ErrorMsg;
    }
}
std::string EditorPanel::AddObjects(const std::vector<EditorObjectPtr> & toAdds, unsigned int startIndex)
{
    std::vector<GUIFormatObject> newObjs;
    newObjs.reserve(toAdds.size());
    for (unsigned int i = 0; i < toAdds.size(); ++i)
    {
        if (toAdds[i]->InitGUIElement(MaterialSet))
        {
            newObjs.insert(newObjs.begin() + startIndex + i,
                           GUIFormatObject(toAdds[i]->GetActiveGUIElement(),
                                           toAdds[i]->GetMoveHorizontally(),
                                           toAdds[i]->GetMoveVertically(),
                                           toAdds[i]->Offset));
            editorObjects.insert(editorObjects.end(), toAdds[i]);
        }
        else
        {
            //Make sure to remove all the other elements that were added.
            editorObjects.erase(editorObjects.end() - newObjs.size(), editorObjects.end());
            return std::string("Error adding object #") + std::to_string(i) + ": " + EditorObject::ErrorMsg;
        }
    }

    return "";
}
bool EditorPanel::RemoveObject(EditorObjectPtr toRemove)
{
    bool found = false;

    for (unsigned int i = 0; i < editorObjects.size(); ++i)
    {
        if (editorObjects[i].get() == toRemove.get())
        {
            editorObjects.erase(editorObjects.begin() + i);
            found = true;
            panel.RemoveObject(i);
            --i;
        }
    }

    return found;
}

bool EditorPanel::GetDidBoundsChangeDeep(void) const
{
    return DidBoundsChange || panel.GetDidBoundsChangeDeep();
}
void EditorPanel::ClearDidBoundsChangeDeep(void)
{
    DidBoundsChange = false;
    panel.ClearDidBoundsChangeDeep();
}

void EditorPanel::CustomUpdate(float elapsedTime, Vector2f relMousePos)
{
    //Update the editor objects.
    for (unsigned int i = 0; i < editorObjects.size(); ++i)
    {
        //If the active element changed, replace it in the formatted panel.
        if (editorObjects[i]->Update(elapsedTime, relMousePos))
        {
            panel.ReplaceObject(i, GUIFormatObject(editorObjects[i]->GetActiveGUIElement(),
                                                   editorObjects[i]->GetMoveHorizontally(),
                                                   editorObjects[i]->GetMoveVertically(),
                                                   editorObjects[i]->Offset));
            DidBoundsChange = true;
        }
    }

    //Now update the GUI objects.
    panel.Update(elapsedTime, relMousePos);
}
std::string EditorPanel::Render(float elapsedTime, const RenderInfo & info)
{
    return panel.Render(elapsedTime, info);
}