#include "RenderDataHandler.h"

#include <assert.h>
#include "ShaderHandler.h"

char RenderDataHandler::errorMsg[ERROR_MESSAGE_SIZE];
const int RenderDataHandler::EXCEPTION_ELEMENTS_OUT_OF_RANGE = 1;

bool RenderDataHandler::GetUniformLocation(BufferObjHandle shaderProgram, const Char* name, UniformLocation & out_handle)
{
	out_handle = glGetUniformLocation(shaderProgram, name);

	if (!UniformLocIsValid(out_handle))
	{
		SetErrorMsg((std::string("Shader does not contain '") + name + "' (or it was optimized out in compilation).").c_str());
		return false;
	}

	return true;
}

void RenderDataHandler::SetUniformValue(UniformLocation loc, int elements, const float * value)
{
	if (elements < 1 || elements > 4)
	{
		throw EXCEPTION_ELEMENTS_OUT_OF_RANGE;
	}

	switch (elements)
	{
		case 1:
			glUniform1f(loc, value[0]);
			break;
		case 2:
			glUniform2f(loc, value[0], value[1]);
			break;
		case 3:
			glUniform3f(loc, value[0], value[1], value[2]);
			break;
		case 4:
			glUniform4f(loc, value[0], value[1], value[2], value[3]);
			break;

		default: assert(false);
	}
}
void RenderDataHandler::SetUniformValue(UniformLocation loc, int elements, const int * value)
{
	if (elements < 1 || elements > 4)
	{
		throw EXCEPTION_ELEMENTS_OUT_OF_RANGE;
	}

	switch (elements)
	{
		case 1:
			glUniform1i(loc, value[0]);
			break;
		case 2:
			glUniform2i(loc, value[0], value[1]);
			break;
		case 3:
			glUniform3i(loc, value[0], value[1], value[2]);
			break;
		case 4:
			glUniform4i(loc, value[0], value[1], value[2], value[3]);
			break;

		default: assert(false);
	}
}
void RenderDataHandler::SetMatrixValue(UniformLocation lc, const Matrix4f & mat)
{
	glUniformMatrix4fv(lc, 1, GL_TRUE, (const GLfloat*)(&mat));
}

void RenderDataHandler::CreateTexture2D(BufferObjHandle & texObjectHandle, sf::Image & img)
{
	glGenTextures(1, &texObjectHandle);
	glBindTexture(GL_TEXTURE_2D, texObjectHandle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.getSize().x, img.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.getPixelsPtr());
}
void RenderDataHandler::CreateTexture2D(BufferObjHandle & texObjectHandle, Vector2i size)
{
	glGenTextures(1, &texObjectHandle);
	glBindTexture(GL_TEXTURE_2D, texObjectHandle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
}
void RenderDataHandler::CreateDepthTexture2D(BufferObjHandle & depthTexObjHandle, Vector2i size)
{
	glGenTextures(1, &depthTexObjHandle);
	glBindTexture(GL_TEXTURE_2D, depthTexObjHandle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, size.x, size.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
}

void RenderDataHandler::SetTexture2DDataFloats(const BufferObjHandle & texObjectHandle, Vector2i texSize, Void* pixelData)
{
	glBindTexture(GL_TEXTURE_2D, texObjectHandle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texSize.x, texSize.y, 0, GL_RGBA, GL_FLOAT, pixelData);
}
void RenderDataHandler::SetTexture2DDataUBytes(const BufferObjHandle & texObjectHandle, Vector2i texSize, Void* pixelData)
{
	glBindTexture(GL_TEXTURE_2D, texObjectHandle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texSize.x, texSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, pixelData);
}

void RenderDataHandler::SetDepthTexture2DSize(const BufferObjHandle & texObjHandle, Vector2i texSize)
{
	glBindTexture(GL_TEXTURE_2D, texObjHandle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, texSize.x, texSize.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
}

void RenderDataHandler::DeleteTexture2D(BufferObjHandle & texObjHandle)
{
	glDeleteTextures(1, &texObjHandle);
}

RenderDataHandler::FrameBufferStatus RenderDataHandler::GetFramebufferStatus(const BufferObjHandle & fbo)

{
	GLint prevBuffer;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevBuffer);


	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	glBindFramebuffer(GL_FRAMEBUFFER, prevBuffer);


	switch (result)
	{
		case GL_FRAMEBUFFER_COMPLETE: return FrameBufferStatus::EVERYTHING_IS_FINE;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: return FrameBufferStatus::BAD_ATTACHMENT;
		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT: return FrameBufferStatus::DIFFERENT_ATTACHMENT_DIMENSIONS;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: return FrameBufferStatus::NO_ATTACHMENTS;
		case GL_FRAMEBUFFER_UNSUPPORTED: return FrameBufferStatus::NOT_SUPPORTED;

		default: return FrameBufferStatus::UNKNOWN;
	}
}
const char * RenderDataHandler::GetFrameBufferStatusMessage(const BufferObjHandle & fbo)
{
	GLint prevBuffer;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevBuffer);


	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	if (strcmp(GetCurrentRenderingError(), "") != 0)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, prevBuffer);
		return "Unable to bind the frame buffer";
	}
	glBindFramebuffer(GL_FRAMEBUFFER, prevBuffer);


	switch (glCheckFramebufferStatus(GL_FRAMEBUFFER))
	{
		case GL_FRAMEBUFFER_COMPLETE: return "";
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: return "Bad texture or depth buffer attachment";
		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT: return "Texture and depth buffer are different dimensions.";
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: return "Nothing is attached";
		case GL_FRAMEBUFFER_UNSUPPORTED: return "This combination of texture and depth buffer is not supported on this platform.";

		default: assert(false);
	}
}