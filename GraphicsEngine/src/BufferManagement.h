#pragma once

typedef unsigned int u32;
typedef unsigned int GLenum;
typedef unsigned int GLuint;

struct Buffer
{
	GLuint handle;
	GLenum type;
	u32 size;
	u32 head;
	void* data; // Mapped data
};

Buffer CreateBuffer(u32 size, GLenum type, GLenum usage);

void BindBuffer(const Buffer& buffer);

void MapBuffer(Buffer& buffer, GLenum access);

void UnmapBuffer(Buffer& buffer);

bool IsPowerOf2(u32 value);

u32 Align(u32 value, u32 alignment);

void AlignHead(Buffer& buffer, u32 alignment);

void PushAlignedData(Buffer& buffer, const void* data, u32 size, u32 alignment);

#define CreateConstantBuffer(size) CreateBuffer(size, GL_UNIFORM_BUFFER, GL_STREAM_DRAW)
#define CreateStaticVertexBuffer(size) CreateBuffer(size, GL_ARRAY_BUFFER, GL_STATIC_DRAW)
#define CreateStaticIndexBuffer(size) CreateBuffer(size, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW)

#define PushData(buffer, data, size) PushAlignedData(buffer, data, size, 1)
#define PushUInt(buffer, value) { u32 v = value; PushAlignedData(buffer, &v, sizeof(v), 4); }
#define PushVec3(buffer, value) PushAlignedData(buffer, value_ptr(value), sizeof(value), sizeof(vec4))
#define PushVec4(buffer, value) PushAlignedData(buffer, value_ptr(value), sizeof(value), sizeof(vec4))
#define PushMat3(buffer, value) PushAlignedData(buffer, value_ptr(value), sizeof(value), sizeof(vec4))
#define PushMat4(buffer, value) PushAlignedData(buffer, value_ptr(value), sizeof(value), sizeof(vec4))