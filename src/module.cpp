#include "module.h"

float genRand(float x){
    float rand01 = rand() / static_cast<float>(RAND_MAX);
    rand01 = 2*rand01 - 1;
    return rand01* x;
}

void genVertex(unsigned int* VBOAddr, unsigned int* VAOAddr, 
        float vertices[], unsigned long verticesSize){
    glGenVertexArrays(1, VAOAddr);
    glGenBuffers(1, VBOAddr);
    glBindVertexArray(*VAOAddr);
    glBindBuffer(GL_ARRAY_BUFFER, *VBOAddr);
    glBufferData(GL_ARRAY_BUFFER, verticesSize, vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
}

void enableMatrices(Shader shader, glm::mat4 model, glm::mat4 proj){
    int modelLoc = glGetUniformLocation(shader.ID, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	int projLoc = glGetUniformLocation(shader.ID, "proj");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
}

void renderShape(unsigned int VAO, unsigned int texture, 
        Shader shader, int numVertices,
        glm::mat4 model, glm::mat4 proj){
        // bind Texture
        glBindTexture(GL_TEXTURE_2D, texture);
        // enabling matrices
    	enableMatrices(shader, model, proj);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);   
}

void genTexture(unsigned int* textureAddr, const char* imagePath){
    glGenTextures(1, textureAddr);
    glBindTexture(GL_TEXTURE_2D, *textureAddr); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
    unsigned char *data = stbi_load(imagePath, &width, &height, &nrChannels, 0);
    // std::cout<<imagePath<<" "<<nrChannels<<std::endl;
    if (data)
    {
        if (nrChannels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        else
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

