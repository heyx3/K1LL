#pragma once


#include "../../Rendering/Rendering.hpp"



//Handles post-processing of rendered world data.
class PostProcessing
{
public:

    static PostProcessing Instance;



    //Returns whether initialization succeeded.
    bool Initialize(std::string& outError);
    void Destroy(void);


    //Runs post-processing and returns the final color texture.
    RenderTarget* Apply(const MTexture2D& worldColor,
                      const MTexture2D& worldDepth);


private:

    static std::string errMsg;


    std::unique_ptr<RenderTarget> rendTarg;

    MTexture2D outTex;

    //The various textures used. Changes based on quality settings.
    std::vector<MTexture2D> colorTargets;
    //The various passes that are run. Changes based on quality settings.
    std::vector<std::unique_ptr<Material>> passes;
    std::vector<UniformDictionary> passParams;

    
    PostProcessing(void);
};