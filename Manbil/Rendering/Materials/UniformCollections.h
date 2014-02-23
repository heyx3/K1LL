#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include "../../OpenGLIncludes.h"
#include <SFML/Graphics/Texture.hpp>
#include "../../Math/Matrix4f.h"
#include "Data Nodes/Vector.h"


#define U_UMAP(Type) (std::unordered_map<std::string, Type>)
typedef std::shared_ptr<sf::Texture> SFTexPtr;




//Represents a single float/vec2/vec3/vec4 uniform.
struct UniformValueF
{
public:
    float Value[4];
    unsigned int NData;
    std::string Name;
    UniformLocation Location;
    UniformValueF(float value, UniformLocation loc, std::string name)
        : Name(name), Location(loc), NData(1) { Value[0] = value; }
    UniformValueF(Vector2f value, UniformLocation loc, std::string name)
        : Name(name), Location(loc), NData(2) { Value[0] = value.x; Value[1] = value.y; }
    UniformValueF(Vector3f value, UniformLocation loc, std::string name)
        : Name(name), Location(loc), NData(3) { Value[0] = value.x; Value[1] = value.y; Value[2] = value.z; }
    UniformValueF(Vector4f value, UniformLocation loc, std::string name)
        : Name(name), Location(loc), NData(4) { Value[0] = value.x; Value[1] = value.y; Value[2] = value.z; Value[3] = value.w; }
    UniformValueF(const float * value = 0, unsigned int nData = 0, UniformLocation loc = -1, std::string name = "")
        : Name(name), Location(loc), NData(nData)
    {
        for (int i = 0; i < nData; ++i) Value[i] = value[i];
    }
    void SetValue(float value) { NData = 1; Value[0] = value; }
    void SetValue(Vector2f value) { NData = 2; Value[0] = value.x; Value[1] = value.y; }
    void SetValue(Vector3f value) { NData = 3; Value[0] = value.x; Value[1] = value.y; Value[2] = value.z; }
    void SetValue(Vector4f value) { NData = 4; Value[0] = value.x; Value[1] = value.y; Value[2] = value.z;  Value[3] = value.w; }
    std::string GetDeclaration(void) const { return "uniform " + VectorF(NData).GetGLSLType() + " " + Name + ";"; }
};

//Represents an array of float/vec2/vec3/vec4 uniforms.
struct UniformArrayValueF
{
public:
    float * Values;
    unsigned int NumbValues, BasicTypesPerValue;
    UniformLocation Location;
    std::string Name;
    UniformArrayValueF(const float * values = 0, unsigned int nValues = 0, unsigned int nBasicTypesPerValue = 0, UniformLocation loc = 0, std::string name = "")
        : Name(name), Location(loc), Values(0), NumbValues(nValues), BasicTypesPerValue(nBasicTypesPerValue)
    {
        if (values != 0)
        {
            unsigned int count = NumbValues * BasicTypesPerValue;
            Values = new float[count];
            for (unsigned int i = 0; i < count; ++i) Values[i] = values[i];
        }
    }
    UniformArrayValueF(const UniformArrayValueF & cpy) : UniformArrayValueF(cpy.Values, cpy.NumbValues, cpy.BasicTypesPerValue, cpy.Location, cpy.Name) { }
    ~UniformArrayValueF(void) { if (Values != 0) delete[] Values; }
    void SetData(const float * values = 0, int nValues = -1, int nBasicTypesPerValue = -1)
    {
        if (nValues > 0 && nBasicTypesPerValue > 0)
        {
            delete[] values;
            NumbValues = nValues;
            BasicTypesPerValue = nBasicTypesPerValue;
            values = new float[nValues * nBasicTypesPerValue];
        }
        for (unsigned int i = 0; i < NumbValues * BasicTypesPerValue; ++i) Values[i] = values[i];
    }
    std::string GetDeclaration(void) const { return "uniform " + VectorF(BasicTypesPerValue).GetGLSLType() + " " + Name + "[" + std::to_string(NumbValues) + "];"; }
};


//Represents a single int/ivec2/ivec3/ivec4 uniform.
struct UniformValueI
{
public:
    int Value[4];
    unsigned int NData;
    std::string Name;
    UniformLocation Location;
    UniformValueI(int value, UniformLocation loc, std::string name)
        : Name(name), Location(loc), NData(1) { Value[0] = value; }
    UniformValueI(Vector2f value, UniformLocation loc, std::string name)
        : Name(name), Location(loc), NData(2) { Value[0] = value.x; Value[1] = value.y; }
    UniformValueI(Vector3f value, UniformLocation loc, std::string name)
        : Name(name), Location(loc), NData(3) { Value[0] = value.x; Value[1] = value.y; Value[2] = value.z; }
    UniformValueI(Vector4f value, UniformLocation loc, std::string name)
        : Name(name), Location(loc), NData(4) { Value[0] = value.x; Value[1] = value.y; Value[2] = value.z; Value[3] = value.w; }
    UniformValueI(const int * value = 0, unsigned int nData = 0, UniformLocation loc = -1, std::string name = "")
        : Name(name), Location(loc), NData(nData)
    {
        for (int i = 0; i < nData; ++i) Value[i] = value[i];
    }
    void SetValue(int value) { NData = 1; Value[0] = value; }
    void SetValue(Vector2i value) { NData = 2; Value[0] = value.x; Value[1] = value.y; }
    void SetValue(Vector3i value) { NData = 3; Value[0] = value.x; Value[1] = value.y; Value[2] = value.z; }
    void SetValue(Vector4i value) { NData = 4; Value[0] = value.x; Value[1] = value.y; Value[2] = value.z;  Value[3] = value.w; }
    std::string GetDeclaration(void) const { return "uniform " + VectorI(NData).GetGLSLType() + " " + Name + ";"; }
};

//Represents an array of int/ivec2/ivec3/ivec4 uniforms.
struct UniformArrayValueI
{
public:
    int * Values;
    unsigned int NumbValues, BasicTypesPerValue;
    UniformLocation Location;
    std::string Name;
    UniformArrayValueI(const int * values = 0, unsigned int nValues = 0, unsigned int nBasicTypesPerValue = 0, UniformLocation loc = 0, std::string name = "")
        : Name(name), Location(loc), Values(0), NumbValues(nValues), BasicTypesPerValue(nBasicTypesPerValue)
    {
        if (values != 0)
        {
            unsigned int count = NumbValues * BasicTypesPerValue;
            Values = new int[count];
            for (unsigned int i = 0; i < count; ++i) Values[i] = values[i];
        }
    }
    UniformArrayValueI(const UniformArrayValueI & cpy) : UniformArrayValueI(cpy.Values, cpy.NumbValues, cpy.BasicTypesPerValue, cpy.Location, cpy.Name) { }
    ~UniformArrayValueI(void) { if (Values != 0) delete[] Values; }
    void SetData(const int * values = 0, int nValues = -1, int nBasicTypesPerValue = -1)
    {
        if (nValues > 0 && nBasicTypesPerValue > 0)
        {
            delete[] values;
            NumbValues = nValues;
            BasicTypesPerValue = nBasicTypesPerValue;
            values = new int[nValues * nBasicTypesPerValue];
        }
        for (unsigned int i = 0; i < NumbValues * BasicTypesPerValue; ++i) Values[i] = values[i];
    }
    std::string GetDeclaration(void) const { return "uniform " + VectorI(BasicTypesPerValue).GetGLSLType() + " " + Name + "[" + std::to_string(NumbValues) + "];"; }
};

//Represents a uniform mat4.
struct UniformMatrixValue
{
public:
    Matrix4f Value;
    UniformLocation Location;
    std::string Name;
    UniformMatrixValue(void) { }
    UniformMatrixValue(const Matrix4f & value, UniformLocation loc, std::string name)
        : Name(name), Location(loc), Value(value) { }
    std::string GetDeclaration(void) const { return "uniform mat4 " + Name + ";"; }
};

//Represents a 2D texture sampler uniform.
struct UniformSamplerValue
{
public:
    sf::Texture * Texture;
    UniformLocation Location;
    std::string Name;
    UniformSamplerValue(sf::Texture * texture = 0, UniformLocation loc = 0, std::string name = "")
        : Name(name), Location(loc), Texture(texture) { }
    std::string GetDeclaration(void) const { return "uniform sampler2D " + Name + ";"; }
};




//Represents a list of uniforms.
struct UniformList
{
public:
    struct Uniform { public: std::string Name; UniformLocation Loc; Uniform(std::string name, UniformLocation loc) : Name(name), Loc(loc) { } };
    std::vector<Uniform> FloatUniforms, FloatArrayUniforms,
                         MatrixUniforms, TextureUniforms,
                         IntUniforms, IntArrayUniforms;
    //Returns "Uniform("", 0)" if the given name isn't found.
    static Uniform FindUniform(std::string name, const std::vector<Uniform> & toSearch)
    {
        for (int i = 0; i < toSearch.size(); ++i)
            if (toSearch[i].Name == name)
                return toSearch[i];
        return Uniform("", 0);
    }
};

//Represents a collection of uniform locations.
//TODO: Change it so that the DataNodes write to UniformLists instead of UniformDictionaries.
struct UniformDictionary
{
public:
    U_UMAP(UniformValueF) FloatUniforms;
    U_UMAP(UniformArrayValueF) FloatArrayUniforms;
    U_UMAP(UniformValueI) IntUniforms;
    U_UMAP(UniformArrayValueI) IntArrayUniforms;
    U_UMAP(UniformMatrixValue) MatrixUniforms;
    U_UMAP(UniformSamplerValue) TextureUniforms;
};