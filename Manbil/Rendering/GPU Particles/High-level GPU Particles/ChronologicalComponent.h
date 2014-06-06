#pragma once

#include <memory>

#include "../../../Math/Higher Math/Gradient.h"
#include "../../Materials/UniformCollections.h"
#include "../../Texture Management/TextureManager.h"
#include "../../Materials/Data Nodes/DataNodeIncludes.h"
#include "../GPUParticleDefines.h"
#include "../../Texture Management/TextureConverters.h"


//"HGP" stands for "High-level Gpu Particle".
//An HGP system has a bunch of different "components" for position, color, size, and rotation.
//The base class for HGP components is a template, so it can't store global information correctly.
//Instead, store it in this namespace.
namespace HGPGlobalData
{
    //Each component has a unique ID, just like with DataNodes.
    //This value cannot be static because ChronologicalHGPComponent is a template, not a class.
    unsigned int NextHGPComponentID = 1;

    //The exception that is thrown if an instance of this component fails a sanity check.
    static const int EXCEPTION_CHRONOLOGICAL_HGP_COMPONENT = 19285;


    static const ShaderInNode ParticleIDInput = ShaderInNode(2, 0, 0, 0, 0),
                              ParticleRandSeedInput = ShaderInNode(1, 1, 1, 0, 1);
    static const DataLine ParticleUVs = DataLine(DataNodePtr(new FragmentInputNode(ParticleVertex::GetAttributeData())), 2);
    
    static const std::string ParticleTimeLerpUniformName = "u_particleTime";
    static const DataLine ParticleTimeLerp = DataLine(DataNodePtr(new ParamNode(1, ParticleTimeLerpUniformName)), 0);
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



//The size of the component's float output (float, vec2, vec3, or vec4).
template<unsigned int ComponentSize>
//Represents a property of a particle that changes over time.
class ChronologicalHGPComponent
{
public:

    ChronologicalHGPComponent(TextureManager & manager, UniformDictionary & dict)
        : Manager(manager), Dict(dict)
    {
        id = NextHGPComponentID;
        NextHGPComponentID += 1;
    }


    unsigned int GetUniqueID(void) const { return id; }
    std::string GetUniqueIDStr(void) const { return std::to_string(id); }

    std::string GetError(void) const { return errorMsg; }
    bool HasError(void) const { return !errorMsg.empty(); }


    //Creates the series of DataNodes that effectively creates this component.
    //Has an output size of "ComponentSize".
    virtual DataLine GetComponentOutput(void) const = 0;
    //Initializes any OpenGL data necessary for this component and stores relevant uniform data into the given UniformDictionary.
    virtual void InitializeComponent(void) { }
    //Updates any param values necessary for this component and updates relevant uniform data in the given UniformDictionary.
    virtual void UpdateComponent(void) { }


protected:

    mutable std::string errorMsg;

    TextureManager & Manager;
    UniformDictionary & Params;

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

    unsigned int id;
};


//The size of the component's output (float, vec2, vec3, or vec4).
template<unsigned int ComponentSize>
//Represents a constant value, independent of time.
class ConstantHGPComponent : public ChronologicalHGPComponent<ComponentSize>
{
public:

    VectorF ConstantValue;

    ConstantHGPComponent(const VectorF & constantValue, TextureManager & mng, UniformDictionary & prms) : ChronologicalHGPComponent(mng, prms), ConstantValue(constantValue) { }

    virtual DataLine GetComponentOutput(void) const override
    {
        Assert(ConstantValue, "ConstantValue");
        return DataLine(ConstantValue);
    }
};


//The size of the component's output (float, vec2, vec3, or vec4).
template<unsigned int ComponentSize>
//Represents a randomly-chosen constant value (between two values).
class ConstantRangeHGPComponent : public ChronologicalHGPComponent<ComponentSize>
{
public:

    VectorF MinValue, MaxValue;

    ConstantRangeHGPComponent(const VectorF & minValue, const VectorF & maxValue, TextureManager & mng, UniformDictionary & prms) : ChronologicalHGPComponent(mng, prms), MinValue(minValue), MaxValue(maxValue) { }

    virtual DataLine GetComponentOutput(void) const override
    {
        Assert(MinValue, "MinValue");
        Assert(MaxValue, "MaxValue");
        return DataLine(DataNodePtr(new InterpolateNode(DataLine(MinValue), DataLine(MaxValue), HGPGlobalData::ParticleRandSeedInput)), 0);
    }
};


//The size of the component's output (float, vec2, vec3, or vec4).
template<unsigned int ComponentSize>
//Represents a gradient value over time.
class GradientHGPComponent : public ChronologicalHGPComponent<ComponentSize>
{
public:

    Gradient<ComponentSize> GradientValue;
    HGPTextureQuality GradientTexQuality;

    std::string GetSamplerName(void) const { return std::string("u_gradientSampler") + GetUniqueIDStr(); }
    

    GradientHGPComponent(const Gradient<ComponentSize> & gradientValue, TextureManager & mng, UniformDictionary & prms, unsigned int lookupTextureWidth = 256)
        : ChronologicalHGPComponent(mng, prms), GradientValue(gradientValue), LookupTextureWidth(lookupTextureWidth)
    {

    }


    virtual DataLine GetComponentOutput(void) const override
    {
        DataLine uvLookup(DataNodePtr(new CombineVectorNode(HGPGlobalData::ParticleTimeLerp, DataLine(VectorF(0.5f)))), 0);
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

private:

    //Given a float array of size "LookupTextureWidth", fills it with the values of the gradient.
    void GenerateTextureData(VectorF * values) const
    {
        const float toTValue = 1.0f / (float)(LookupTextureWidth - 1);
        for (unsigned int i = 0; i < LookupTextureWidth; ++i)
            GradientValue.GetValue(i * toTValue, values[i].GetValue());
    }

    unsigned int lookupTexID;
};


//TODO: GradientRangeHGPComponent.