#pragma once

#include "../GUIElement.h"
#include "GUILabel.h"
#include "GUITexture.h"



//A dropdown menu of string items to choose from.
//Note that instances currently have a fixed number of items, set in the constuctor
//    (although items can be hidden with "SetDrawEmptyItems(false)").
class GUISelectionBox : public GUIElement
{
public:

    //"MainBox" is the background when the dropdown menu isn't active.
    //"SelectionBackground" is the background when the dropdown menu is active.
    GUITexture MainBox, SelectionBackground, Highlight;

    TextRenderer* TextRender;
    Vector4f TextColor;
    FreeTypeHandler::FontID FontID;

    //Whether this box can be clicked on.
    bool IsClickable = true;

    //Raised when an option is selected.
    void(*OnOptionSelected)(GUISelectionBox* selector, const std::string& item,
                            unsigned int itemIndex, void* pData) = 0;
    //Extra data to be passed to the "OnOptionSelected" event.
    void* OnOptionSelected_pData = 0;

    //Raised when the dropdown box is turned on or off.
    //IS NOT called when a new item is selected, even though
    //    the dropdown box is toggled off when this happens.
    void(*OnDropdownToggled)(GUISelectionBox* selector, void* pData) = 0;
    //Extra data to be passed to the "OnDropdownToggled" event.
    void* OnDropdownToggled_pData = 0;


    //Probably the most terrifying constructor I've ever made. Just take it one line at a time.
    //The "outError" string will be set to an error message, or left unchanged if everything went fine.
    //Passing an invalid GUITexture will just cause that gui element to not be drawn;
    //    it is not considered an error.
    //TODO: Add param for separation of items in the dropdown menu.
    //TODO: Add param for horizontal alignment of text.
    GUISelectionBox(TextRenderer* textRenderer, FreeTypeHandler::FontID font, Vector4f textColor,
                    bool mipmappedText, FilteringTypes textFilterQuality,
                    Vector2f _textScale, float textSpacing,
                    Material* textRenderMat, const UniformDictionary& textRenderParams,
                    const GUITexture& mainBackground, const GUITexture& highlight,
                    const GUITexture& selectionBackground,
                    bool _extendAbove, std::string& outError,
                    const std::vector<std::string>& _items,
                    void(*onOptionSelected)(GUISelectionBox* selector, const std::string& item,
                                            unsigned int itemIndex, void* pData),
                    void(*onDropdownToggled)(GUISelectionBox* selector, void* pData),
                    unsigned int textRenderHeight = 0,
                    void* onOptionSelected_pData = 0, void* onDropdownToggled_pData = 0,
                    float textAnimSpeed = 1.0f);
    virtual ~GUISelectionBox(void);


    //Gets the number of visible items (items with an empty string are not visible
    //    if this instance is set to not draw empty strings in the dropdown box).
    unsigned int GetNVisibleItems(void) const { return nVisibleItems; }

    //Gets the index of the current item.
    unsigned int GetSelectedObject(void) const { return currentItem; }
    //Sets the index of the current item. If "raiseEvent" is true, calls "OnOptionSelected".
    void SetSelectedObject(unsigned int newIndex, bool raiseEvent);

    //Gets the index of the moused-over object. Returns a negative value if none are moused-over.
    int GetMousedOverObject(void) const { return mousedOverItem; }
    //Sets the index of the moused-over object. Pass a negative value if none should be moused-over.
    void SetMousedOverObject(int newIndex) { mousedOverItem = newIndex; }

    //Gets all the items.
    const std::vector<std::string>& GetItems(void) const { return items; }

    //Changes the text value of an item. Returns the old text value.
    //Returns an empty string if the given item index didn't exist.
    std::string SetItem(unsigned int index, std::string newVal);


    //Gets whether the dropdown menu extends above or below this element.
    bool GetExtendsAbove(void) const { return extendAbove; }
    //Sets whether the dropdown menu extends above or below this element.
    void SetExtendsAbove(bool newVal);
    
    //Gets whether the dropdown menu is currently open.
    bool GetIsExtended(void) const { return isExtended; }
    //Sets whether the dropdown menu is currently open. If it is set to open,
    //     the SelectionBackground's bounds are re-calculated.
    //If "raiseEvent" is true, the "OnDropdownToggled" even is raised.
    void SetIsExtended(bool _isExtended, bool raiseEvent = true);

    //Gets whether to draw empty text items.
    bool GetDrawEmptyItems(void) const { return drawEmptyItems; }
    //Sets whether to draw empty text items.
    void SetDrawEmptyItems(bool shouldDraw);


    virtual bool GetDidBoundsChangeDeep(void) const override;
    virtual void ClearDidBoundsChangeDeep(void) override;
    virtual Box2D GetBounds(void) const override;
    virtual void ScaleBy(Vector2f scaleAmount) override;
    virtual void SetScale(Vector2f newScale) override;

    virtual void Render(float elapsedTime, const RenderInfo& info) override;

    virtual void OnMouseClick(Vector2f relativeMousePos) override;


protected:

    virtual void CustomUpdate(float elapsed, Vector2f relativeMousePos) override;


private:

    //Whether to display the options above or below this box.
    bool extendAbove;
    //Whether this box is currently open.
    bool isExtended;
    //Whether to ignore any items that don't have any text.
    //This can be used to "delete" an item from the collection.
    bool drawEmptyItems = false;

    //The scale of the text.
    Vector2f textScale;
    //The vertical space between items in the drop-down menu.
    float textSpacing;

    //The index of the currently-selected item.
    unsigned int currentItem = 0;
    //The index of the item currently moused-over. A value below 0 means nothing is moused over.
    int mousedOverItem = -1;

    //The number of currently-visible items.
    unsigned int nVisibleItems;

    //The text that can be chosen.
    std::vector<std::string> items;
    //The labels for the text options.
    std::vector<GUILabel> itemElements;

    //The font being used to render all the items.
    FreeTypeHandler::FontID itemFontID;


    //Sets up the selection background's position/size based on the main box.
    void PositionSelectionBackground(void);
};