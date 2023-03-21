#pragma once

#include <vector>
#include "Platform.h"
#include "GLDebugger.h"

struct VertexBufferAttribute
{
	u8 location;
	u32 type;
	u8 componentCount;
	u8 offset;

	static unsigned int GetSizeOfType(unsigned int type)
	{
		switch (type)
		{
		case GL_FLOAT:			return 4;
		case GL_UNSIGNED_INT:	return 4;
		case GL_UNSIGNED_BYTE:	return 1;
		}
		ASSERT(false);
		return 0;
	}
};

struct VertexBufferLayout
{
public:
	VertexBufferLayout()
		: m_Stride(0) {}

	template<typename T>
	void Push(u8 location, u8 count)
	{
		static_assert(false);
	}

	template<>
	void Push<float>(u8 location, u8 count)
	{
		m_Attributes.push_back({ location, GL_FLOAT, count, GL_FALSE });
		m_Stride += count * VertexBufferAttribute::GetSizeOfType(GL_FLOAT);
	}

	template<>
	void Push<unsigned int>(u8 location, u8 count)
	{
		m_Attributes.push_back({ location, GL_UNSIGNED_INT, count, GL_FALSE });
		m_Stride += count * VertexBufferAttribute::GetSizeOfType(GL_UNSIGNED_INT);
	}

	template<>
	void Push<unsigned char>(u8 location, u8 count)
	{
		m_Attributes.push_back({ location, GL_UNSIGNED_BYTE, count, GL_TRUE });
		m_Stride += count * VertexBufferAttribute::GetSizeOfType(GL_UNSIGNED_BYTE);
	}
	
	inline const std::vector<VertexBufferAttribute>& GetAttributes() const { return m_Attributes; }

	inline u8 GetStride() const { return m_Stride; }

private:
	std::vector<VertexBufferAttribute> m_Attributes;
	u8 m_Stride;
};

struct VertexShaderAttribute
{
	u8 location;
	u8 componentCount;
};

struct VertexShaderLayout
{
	std::vector<VertexShaderAttribute> attributes;
};