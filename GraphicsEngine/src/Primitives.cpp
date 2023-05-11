#include "Primitives.h"

u32 CreateQuad()
{
    float vertices[] =
        {
        -1.0f, -1.0f,
        0.0f,  0.0f,

        1.0f,  -1.0f,
        1.0f,  0.0f,

        1.0f,  1.0f,
        1.0f,  1.0f,

        -1.0f, 1.0f,
        0.0f,  1.0f
        };

    u32 indices[] =
        {
            0, 1, 2,
            0, 2, 3
        };

    u32 VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return VAO;
}

Model* CreatePrimitive(App* app, PrimitiveType type, Material& material, u32 xNumSegments, u32 yNumSegments)
{
    Model* model = new Model();
    app->models.push_back(model);

    model->materialIDs.push_back((u32)app->materials.size());
    app->materials.push_back(material);

    Mesh mesh = {};
    switch (type)
    {
    case PrimitiveType::PLANE:
    {
        mesh.vertices.insert(mesh.vertices.end(),
            {
            -0.5f, -0.5f,  0.0f,
            0.0f,  0.0f,  1.0f,
            0.0f,  0.0f,

            0.5f, -0.5f,  0.0f,
            0.0f,  0.0f,  1.0f,
            1.0f,  0.0f,

            0.5f,  0.5f,  0.0f,
            0.0f,  0.0f,  1.0f,
            1.0f,  1.0f,

            -0.5f,  0.5f,  0.0f,
            0.0f,  0.0f,  1.0f,
            0.0f,  1.0f
            });

        mesh.indices.insert(mesh.indices.end(),
            {
                0, 1, 2,
                0, 2, 3
            });
    }
    break;
    case PrimitiveType::CUBE:
    {
        mesh.vertices.insert(mesh.vertices.end(),
            {
                /* POSITION             NORMALS               TEXCOORD */
                // Back
                -0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,    0.0f, 0.0f,
                 0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,    1.0f, 0.0f,
                 0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,    1.0f, 1.0f,
                -0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,    0.0f, 1.0f,

                // Front
                -0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,    0.0f, 0.0f,
                 0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,    1.0f, 0.0f,
                 0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,    1.0f, 1.0f,
                -0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,    0.0f, 1.0f,

                // Left
                -0.5f,  0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,    0.0f, 0.0f,
                -0.5f, -0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,    1.0f, 0.0f,
                -0.5f, -0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,    1.0f, 1.0f,
                -0.5f,  0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,    0.0f, 1.0f,

                // Right
                 0.5f, -0.5f, -0.5f,    1.0f,  0.0f,  0.0f,    0.0f, 0.0f,
                 0.5f,  0.5f, -0.5f,    1.0f,  0.0f,  0.0f,    1.0f, 0.0f,
                 0.5f,  0.5f,  0.5f,    1.0f,  0.0f,  0.0f,    1.0f, 1.0f,
                 0.5f, -0.5f,  0.5f,    1.0f,  0.0f,  0.0f,    0.0f, 1.0f,

                 // Bottom
                -0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,    0.0f, 0.0f,
                 0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,    1.0f, 0.0f,
                 0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,    1.0f, 1.0f,
                -0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,    0.0f, 1.0f,

                // Top
                 0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,    0.0f, 0.0f,
                -0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,    1.0f, 0.0f,
                -0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,    1.0f, 1.0f,
                 0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,    0.0f, 1.0f,
            });

        mesh.indices.insert(mesh.indices.end(),
            {
                // Front and Back
                0, 3, 2,
                2, 1, 0,
                4, 5, 6,
                6, 7 ,4,

                // Left and Right
                11, 8, 9,
                9, 10, 11,
                12, 13, 14,
                14, 15, 12,

                // Bottom and Top
                16, 17, 18,
                18, 19, 16,
                20, 21, 22,
                22, 23, 20
            });
    }
    break;
    case PrimitiveType::SPHERE:
    {
        for (u32 y = 0; y <= yNumSegments; ++y)
        {
            for (u32 x = 0; x <= xNumSegments; ++x)
            {
                float xSegment = (float)x / (float)xNumSegments;
                float ySegment = (float)y / (float)yNumSegments;

                float xPos = std::cos(xSegment * TAU) * std::sin(ySegment * PI);
                float yPos = std::cos(ySegment * PI);
                float zPos = std::sin(xSegment * TAU) * std::sin(ySegment * PI);

                // Inserts: Position - Normal - TexCoord
                mesh.vertices.insert(mesh.vertices.end(), { xPos, yPos, zPos, xPos, yPos, zPos, xSegment, ySegment });
            }
        }

        bool oddRow = false;
        for (u32 y = 0; y < yNumSegments; ++y)
        {
            for (u32 x = 0; x < xNumSegments; ++x)
            {
                mesh.indices.push_back((y + 1) * (xNumSegments + 1) + x);
                mesh.indices.push_back(y * (xNumSegments + 1) + x);
                mesh.indices.push_back(y * (xNumSegments + 1) + x + 1);

                mesh.indices.push_back((y + 1) * (xNumSegments + 1) + x);
                mesh.indices.push_back(y * (xNumSegments + 1) + x + 1);
                mesh.indices.push_back((y + 1) * (xNumSegments + 1) + x + 1);
            }
        }
    }
    break;
    }

    // Create the vertex format
    mesh.VBLayout.attributes.push_back(VertexBufferAttribute{ 0, 3, 0 });
    mesh.VBLayout.attributes.push_back(VertexBufferAttribute{ 1, 3, 3 * sizeof(float) });
    mesh.VBLayout.stride = 6 * sizeof(float);

    mesh.VBLayout.attributes.push_back(VertexBufferAttribute{ 2, 2, mesh.VBLayout.stride });
    mesh.VBLayout.stride += 2 * sizeof(float);

    mesh.vertexOffset = 0;
    mesh.indexOffset = 0;

    glGenBuffers(1, &model->VBHandle);
    glBindBuffer(GL_ARRAY_BUFFER, model->VBHandle);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(float), mesh.vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &model->EBHandle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->EBHandle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(u32), mesh.indices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    model->meshes.push_back(mesh);

    return model;
}