#pragma once

#include <memory>

#include "../../../Math/Higher Math/Gradient.h"
#include "../../Materials/UniformCollections.h"
#include "../../Texture Management/TextureManager.h"
#include "../../Materials/Data Nodes/DataNodeIncludes.h"
#include "../GPUParticleDefines.h"
#include "../../Texture Management/TextureConverters.h"


class HGPComponentManager;


//"HGP" stands for "High-level Gpu Particle".
//An HGP system has a bunch of different "components" for position, color, size, and rotation.
//The base class for HGP components is a template, so it can't store global information correctly.
//Instead, store it in this namespace.
namespace HGPGlobalData
{
    //Each component has a unique ID, just like with DataNodes.
    //This value cannot be static because HGPOutputComponent is a template, not a class.
    extern unsigned int NextHGPComponentID;

    //The exception that is thrown if an instance of this component fails a sanity check.
    extern const int EXCEPTION_CHRONOLOGICAL_HGP_COMPONENT;

    extern const DataLine ParticleIDInput, ParticleRandSeedInputs, ParticleUVs;
    extern const DataNodePtr ParticleRandSeedComponents;
    
    extern const std::string ParticleElapsedTimeUniformName;
    extern const DataLine ParticleElapsedTime;

    extern TextureManager & GetTexManager(HGPComponentManager & manager);
    extern UniformDictionary & GetParams(HGPComponentManager & manager);
    extern const DataLine & GetTimeLerp(HGPComponentManager & manager);
}


//Quality settings for a texture used in HGP particles.
struct HGPTextureQuality
{
public:

    TextureSettings::TextureFiltering FilterQuality;
    unsigned int Width;
    HGPTextureQuality(TextureSettings::TextureFiltering filterQuality, unsigned int width)
        : FilterQuality(filterQuality), Width(width)
    {

    }
};



#define HGPComponentPtr(ComponentSize) std::shared_ptr<HGPOutputComponent<ComponentSize>>
#define HGPConstComponentPtr(ComponentSize) std::shared_ptr<const HGPOutputComponent<ComponentSize>>


//The size of the component's float output (float, vec2, vec3, or vec4).
template<unsigned int ComponentSize>
//Represents some property of a particle.
class HGPOutputComponent
{
public:

    HGPComponentManager & Manager;
    TextureManager & GetTexManager(void) const { return HGPGlobalData::GetTexManager(Manager); }
    UniformDictionary & GetParams(void) const { return HGPGlobalData::GetParams(Manager); }


    HGPOutputComponent(HGPComponentManager & manager)
        : Manager(manager)
    {
        id = HGPGlobalData::NextHGPComponentID;
        HGPGlobalData::NextHGPComponentID += 1;
    }
    HGPOutputComponent(const HGPOutputComponent & cpy); //Intentionally not implemented.


    void AddParent(HGPOutputComponent<1>* parent)
    {
        if (std::find(parents1.begin(), parents1.end(), parent) == parents1.end())
            parents1.insert(parents1.end(), parent);
    }
    void AddParent(HGPOutputComponent<2>* parent)
    {
        if (std::find(parents2.begin(), parents2.end(), parent) == parents2.end())
            parents2.insert(parents2.end(), parent);
    }
    void AddParent(HGPOutputComponent<3>* parent)
    {
        if (std::find(parents3.begin(), parents3.end(), parent) == parents3.end())
            parents3.insert(parents3.end(), parent);
    }
    void AddParent(HGPOutputComponent<4>* parent)
    {
        if (std::find(parents4.begin(), parents4.end(), parent) == parents4.end())
            parents4.insert(parents4.end(), parent);
    }
    void RemoveParent(HGPOutputComponent<1>* parent)
    {
        parents1.erase(std::find(parents1.begin(), parents1.end(), parent));
    }
    void RemoveParent(HGPOutputComponent<2>* parent)
    {
        parents2.erase(std::find(parents2.begin(), parents2.end(), parent));
    }
    void RemoveParent(HGPOutputComponent<3>* parent)
    {
        parents3.erase(std::find(parents3.begin(), parents3.end(), parent));
    }
    void RemoveParent(HGPOutputComponent<4>* parent)
    {
        parents4.erase(std::find(parents4.begin(), parents4.end(), parent));
    }


    //Gets the tree of DataNodes that effectively creates this component's behavior.
    //Has an output size of "ComponentSize".
    DataLine GetComponentOutput(void) const
    {
        if (!createdComponentYet)
        {
            createdComponentYet = true;
            UpdateComponentOutput();
        }
        return componentOutput;
    }

    unsigned int GetUniqueID(void) const { return id; }
    std::string GetUniqueIDStr(void) const { return std::to_string(id); }

    std::string GetError(void) const { return errorMsg; }
    bool HasError(void) const { return !errorMsg.empty(); }


    //Searches down this component's directed graph recursively and replaces all instances of the given old component with the given new component.
    virtual void SwapOutSubComponent(HGPComponentPtr(1) oldC, HGPComponentPtr(1) newC) { }
    //Searches down this component's directed graph recursively and replaces all instances of the given old component with the given new component.
    virtual void SwapOutSubComponent(HGPComponentPtr(2) oldC, HGPComponentPtr(2) newC) { }
    //Searches down this component's directed graph recursively and replaces all instances of the given old component with the given new component.
    virtual void SwapOutSubComponent(HGPComponentPtr(3) oldC, HGPComponentPtr(3) newC) { }
    //Searches down this component's directed graph recursively and replaces all instances of the given old component with the given new component.
    virtual void SwapOutSubComponent(HGPComponentPtr(4) oldC, HGPComponentPtr(4) newC) { }


    //Initializes any OpenGL data necessary for this component and stores relevant uniform data into the given UniformDictionary.
    virtual void InitializeComponent(void) { }
    //Updates any param values necessary for this component and updates relevant uniform data in the given UniformDictionary.
    virtual void UpdateComponent(void) { }

    //Updates this component's DataLine output.
    //Must be called when a property of this component that impacts the output DataLine is changed.
    void UpdateComponentOutput(void) const
    {
        componentOutput = GenerateComponentOutput();

        for (unsigned int i = 0; i < parents1.size(); ++i)
            parents1[i]->UpdateComponentOutput();
        for (unsigned int i = 0; i < parents2.size(); ++i)
            parents2[i]->UpdateComponentOutput();
        for (unsigned int i = 0; i < parents3.size(); ++i)
            parents3[i]->UpdateComponentOutput();
        for (unsigned int i = 0; i < parents4.size(); ++i)
            parents4[i]->UpdateComponentOutput();
    }


protected:

    mutable std::string errorMsg;


    //Generates the output for this component's behavior.
    virtual DataLine GenerateComponentOutput(void) const = 0;


    //If the given test is false, sets the error message to the given message and throws EXCEPTION_CHRONOLOGICAL_HGP_COMPONENT.
    void Assert(bool test, std::string errMsg = "UNKNOWN ERROR") const
    {
        if (!test)
        {
            errorMsg = errMsg;
            throw HGPGlobalData::EXCEPTION_CHRONOLOGICAL_HGP_COMPONENT;
        }
    }
    //Asserts that the given vector has the given size.
    void Assert(const VectorF & v, std::string nameOfVector, int size = ComponentSize)
    {
        Assert(v.GetSize() == size,
               std::string() + "'" + nameOfVector + "' is size " + std::to_string(v.GetSize()) +
                   ", not size " + std::to_string(size) + "!");
    }


private:

    mutable DataLine componentOutput;
    unsigned int id;

    mutable bool createdComponentYet = false;

    std::vector<HGPOutputComponent<1>*> parents1;
    std::vector<HGPOutputComponent<2>*> parents2;
    std::vector<HGPOutputComponent<3>*> parents3;
    std::vector<HGPOutputComponent<4>*> parents4;
};



//The size of the component's output (float, vec2, vec3, or vec4).
template<unsigned int ComponentSize>
//Represents a constant value, independent of time.
class ConstantHGPComponent : public HGPOutputComponent<ComponentSize>
{
public:

    VectorF GetConstantValue(void) const { return constValue; }
    void SetConstantValue(VectorF newVal) const { constValue = newVal; UpdateComponentOutput(); }

    ConstantHGPComponent(const VectorF & constantValue, HGPComponentManager & manager) : HGPOutputComponent(manager), constValue(constantValue) { }


protected:

    virtual DataLine GenerateComponentOutput(void) const override
    {
        Assert(ConstantValue, "ConstantValue");
        return DataLine(ConstantValue);
    }

private:

    VectorF constValue;
};


//The size of the component's output (float, vec2, vec3, or vec4).
template<unsigned int ComponentSize>
//Represents a gradient value over time.
class GradientHGPComponent : public HGPOutputComponent<ComponentSize>
{
public:

    const Gradient<ComponentSize> & GetGradientValue(void) const { return gradientValue; }
    const HGPTextureQuality & GetGradientTextureQuality(void) const { return gradientTexQuality; }
    void SetGradientValue(const Gradient<ComponentSize> & newGradient) const { gradientValue = newGradient; UpdateComponentOutput(); }
    void SetGradientTextureQuality(const HGPTextureQuality & newTexQuality) const { gradientTexQuality = newTexQuality; UpdateComponentOutput(); }

    std::string GetSamplerName(void) const { return std::string("u_gradientSampler") + GetUniqueIDStr(); }
    

    GradientHGPComponent(const Gradient<ComponentSize> & gradientVal, const HGPTextureQuality & gradientTextureQuality, HGPComponentManager & manager)
        : HGPOutputComponent(manager), gradientValue(gradientVal), gradientTexQuality(gradientTextureQuality)
    {

    }


    virtual void InitializeComponent(void) override
    {
        //Create the texture.
        lookupTexID = Manager.CreateSFMLTexture();
        Assert(lookupTexID != TextureManager::UNUSED_ID, std::string() + "Texture creation failed. Texture width: " + std::to_string(GradientTexQuality.Width));

        //Generate the texture data.
        Array2D<VectorF> texOut(GradientTexQuality.Width, 1);
        GenerateTextureData(texOut.GetArray());
        sf::Image img;
        const sf::Uint8 maxUint8 = std::numeric_limits<sf::Uint8>().max();
        TextureConverters::ToImage(texOut, img, (void*)(&max),
                                   [](void* pDat, VectorF imgElement)
        {
            const sf::Uint8 max = *(sf::Uint8*)pDat;
            return sf::Color((sf::Uint8)(imgElement[0] * max),
                             (imgElement.GetSize() == 1) ? 0 : (sf::Uint8)(imgElement[1] * max),
                             (imgElement.GetSize() <= 2) ? 0 : (sf::Uint8)(imgElement[2] * max),
                             (imgElement.GetSize() <= 3) ? 0 : (sf::Uint8)(imgElement[3] * max));
        });

        ManbilTexture * tex = Manager[lookupTexID];
        Assert(tex != 0, "Texture was not found in the manager immediately after successfully creating it!");
        tex->SFMLTex->create(GradientTexQuality.Width, 1);

        //Set the texture quality.
        tex->SetFiltering(GradientTexQuality.FilterQuality);
        tex->SetWrapping(TextureSettings::TextureWrapping::TW_CLAMP);

        //Set the texture data.
        Assert(tex->SFMLTex->loadFromImage(img), std::string() + "Texture loading failed. Texture width: " + std::to_string(GradientTexQuality.Width));
        Params.TextureUniforms[GetSamplerName()].Texture = tex;

    }
    virtual void UpdateComponent(void) override
    {
        Params.TextureUniforms[GetSamplerName()].Texture = Manager[lookupTexID];
    }

protected:

    virtual DataLine GenerateComponentOutput(void) const override
    {
        DataLine uvLookup(DataNodePtr(new CombineVectorNode(HGPGlobalData::GetTimeLerp(Manager), DataLine(VectorF(0.5f)))), 0);
        DataLine textureSample(DataNodePtr(new TextureSampleNode(uvLookup, GetSamplerName())), TextureSampleNode::GetOutputIndex(ChannelsOut::CO_AllChannels));

        switch (ComponentSize)
        {
            case 1: return DataLine(DataNodePtr(new SwizzleNode(textureSample, SwizzleNode::Components::C_X)), 0);
            case 2: return DataLine(DataNodePtr(new SwizzleNode(textureSample, SwizzleNode::Components::C_X, SwizzleNode::Components::C_Y)), 0);
            case 3: return DataLine(DataNodePtr(new SwizzleNode(textureSample, SwizzleNode::Components::C_X, SwizzleNode::Components::C_Y, SwizzleNode::Components::C_Z)), 0);
            case 4: return DataLine(DataNodePtr(new SwizzleNode(textureSample, SwizzleNode::Components::C_X, SwizzleNode::Components::C_Y, SwizzleNode::Components::C_Z, SwizzleNode::Components::C_W)), 0);
            default:
                Assert(false, std::string() + "Invalid ComponentSize value of " + std::to_string(ComponentSize) + "; must be between 1-4 inclusive!");
                return DataLine(DataNodePtr(), 666);
        }
    }

private:

    //Given a float array of size "LookupTextureWidth", fills it with the values of the gradient.
    void GenerateTextureData(VectorF * values) const
    {
        const float toTValue = 1.0f / (float)(LookupTextureWidth - 1);
        for (unsigned int i = 0; i < LookupTextureWidth; ++i)
            GradientValue.GetValue(i * toTValue, values[i].GetValue());
    }

    Gradient<ComponentSize> gradientValue;
    HGPTextureQuality gradientTexQuality;

    unsigned int lookupTexID;
};


//The size of the component's output (float, vec2, vec3, or vec4).
template<unsigned int ComponentSize>
//Represents a value that is a certain randomized interpolation between two given values.
class RandomizedHGPComponent : public HGPOutputComponent<ComponentSize>
{
public:

    HGPComponentPtr(ComponentSize) GetMin(void) const { return min; }
    HGPComponentPtr(ComponentSize) GetMax(void) const { return max; }
    void SetMin(HGPComponentPtr(ComponentSize) newMin)
    {
        min->RemoveParent(this);
        min = newMin;
        min->AddParent(this);
        UpdateComponentOutput();
    }
    void Setmax(HGPComponentPtr(ComponentSize) newMax)
    {
        max->RemoveParent(this);
        max = newMax;
        max->AddParent(this);
        UpdateComponentOutput();
    }

    unsigned int GetRandSeedIndex(void) const { return randSeedIndex; }
    void SetRandSeedIndex(unsigned int newVal) const { randSeedIndex = newVal; UpdateComponentOutput(); }

    RandomizedHGPComponent(HGPComponentManager & manager,
                           HGPComponentPtr(ComponentSize) _min, HGPComponentPtr(ComponentSize) _max, unsigned int _randSeedIndex = 0)
        : HGPOutputComponent(manager), min(_min), max(_max), randSeedIndex(_randSeedIndex)
    {
        min->AddParent(this);
        max->AddParent(this);
    }

    virtual void InitializeComponent(void) override
    {
        Min->InitializeComponent();
        Max->InitializeComponent();
    }
    virtual void UpdateComponent(void) override
    {
        Min->UpdateComponent();
        Max->UpdateComponent();
    }

    virtual void SwapOutSubComponent(HGPComponentPtr(1) oldC, HGPComponentPtr(1) newC) override
    {
        if ((void*)oldC.get() == (void*)min.get())
            SetMin(newC);
        else min->SwapOutSubComponent(oldC, newC);

        if ((void*)oldC.get() == (void*)max.get())
            SetMax(newC);
        else max->SwapOutSubComponent(oldC, newC);
    }
    virtual void SwapOutSubComponent(HGPComponentPtr(2) oldC, HGPComponentPtr(2) newC) override
    {
        if ((void*)oldC.get() == (void*)min.get())
            SetMin(newC);
        else min->SwapOutSubComponent(oldC, newC);

        if ((void*)oldC.get() == (void*)max.get())
            SetMax(newC);
        else max->SwapOutSubComponent(oldC, newC);
    }
    virtual void SwapOutSubComponent(HGPComponentPtr(3) oldC, HGPComponentPtr(3) newC) override
    {
        if ((void*)oldC.get() == (void*)min.get())
            SetMin(newC);
        else min->SwapOutSubComponent(oldC, newC);

        if ((void*)oldC.get() == (void*)max.get())
            SetMax(newC);
        else max->SwapOutSubComponent(oldC, newC);
    }
    virtual void SwapOutSubComponent(HGPComponentPtr(4) oldC, HGPComponentPtr(4) newC) override
    {
        if ((void*)oldC.get() == (void*)min.get())
            SetMin(newC);
        else min->SwapOutSubComponent(oldC, newC);

        if ((void*)oldC.get() == (void*)max.get())
            SetMax(newC);
        else max->SwapOutSubComponent(oldC, newC);
    }

protected:
    
    virtual DataLine GenerateComponentOutput(void) const override
    {
        return DataLine(DataNodePtr(new InterpolateNode(Min->GetComponentOutput(), Max->GetComponentOutput(),
                                                        DataLine(HGPGlobalData::ParticleRandSeedComponents, randSeedIndex),
                                                        DataLine(1.0f))), 0);
    }

private:

    unsigned int randSeedIndex;
    HGPComponentPtr(ComponentSize) min, max;
};