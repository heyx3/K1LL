#pragma once

#include <unordered_map>

#include "UniformValueVector.h"
#include "UniformValueVectorArray.h"
#include "UniformValueMatrix4f.h"
#include "UniformValueSamplers.h"
#include "UniformValueSubroutine.h"


//"Uniforms" are customizable parameters in OpenGL shaders.
//This file provides a way to store collections of them.


enum UniformTypes
{
    UT_VALUE_F,
    UT_VALUE_I,
    UT_VALUE_F_ARRAY,
    UT_VALUE_I_ARRAY,
    UT_VALUE_MAT4,
    UT_VALUE_SAMPLER2D,
    UT_VALUE_SAMPLER3D,
    UT_VALUE_SAMPLERCUBE,
    UT_VALUE_SUBROUTINE,
};


//A collection of uniforms --just their names and locations in a GLSL shader program.
//PRIORITY: Store a single list, and in the "Uniform" struct add a "UniformTypes" entry.
struct UniformList
{
public:

    struct Uniform
    {
    public:
        std::string Name;
        UniformLocation Loc;
        Uniform(std::string name, UniformLocation loc = -1) : Name(name), Loc(loc) { }
    };

    std::vector<Uniform> Floats, Ints, FloatArrays, IntArrays, Matrices, Subroutines,
                         Texture2Ds, Texture3Ds, TextureCubemaps;

    //Returns "Uniform("", -1)" if the given name isn't found.
    Uniform FindUniform(std::string name) const;
};


//Maps each uniform to its current value and location in a GLSL shader program.
//PRIORITY: Use a union of the different uniform types and store them all in one list/dictionary.
//TODO: Change it so that the DataNodes write to UniformLists instead of UniformDictionaries.
struct UniformDictionary
{
public:

#define UMAP(Type) std::unordered_map<std::string, Type>
    UMAP(UniformValueF)              Floats;
    UMAP(UniformValueArrayF)         FloatArrays;
    UMAP(UniformValueI)              Ints;
    UMAP(UniformValueArrayI)         IntArrays;
    UMAP(UniformValueMatrix4f)       Matrices;
    UMAP(UniformValueSampler2D)      Texture2Ds;
    UMAP(UniformValueSampler3D)      Texture3Ds;
    UMAP(UniformValueSamplerCubemap) TextureCubemaps;
    UMAP(UniformValueSubroutine)     Subroutines;
#undef UMAP


    //Adds the given uniforms to this collection, optionally overwriting duplicates.
    void AddUniforms(const UniformDictionary& other, bool overwriteDuplicates);
    //Clears all uniforms from this collection.
    void ClearUniforms(void);

    //Counts the total number of uniforms in this collection.
    unsigned int GetNumbUniforms(void) const;
};