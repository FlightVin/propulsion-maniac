#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "module.h"
#include "transformations.h"

#include <iostream>
#include <map>
#include <string>

#include <ft2build.h>
#include FT_FREETYPE_H

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window, Player& Player);
void RenderText(Shader &shader, std::string text, float x, float y, float scale, glm::vec3 color);

// settings
const unsigned int SCR_WIDTH = 2500;
const unsigned int SCR_HEIGHT = 1500;
const float SCR_RATIO = static_cast<float>(SCR_WIDTH)/static_cast<float>(SCR_HEIGHT);

// some variable
const char* backgroundImagePath = "../src/textures/background.png";
glm::mat4 model = glm::mat4(1.0f);
glm::mat4 proj = glm::mat4(1.0f);
const float playerInitx = -0.7f;
const float playerInity = -0.7f;
float backgroundShiftSpeed;

/// Holds all state information relevant to a character as loaded using FreeType
struct Character {
    unsigned int TextureID; // ID handle of the glyph texture
    glm::ivec2   Size;      // Size of glyph
    glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
    unsigned int Advance;   // Horizontal offset to advance to next glyph
};

std::map <GLchar, Character> Characters;
unsigned int VAO, textVBO;

int main()
{

    srand(time(NULL));
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    // OpenGL state
    // ------------
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // some settings
    stbi_set_flip_vertically_on_load(true); 
    
    // TEXT RENDERING
    /************************************************************/

    // compile and setup the shader
    // ----------------------------
    Shader textShader("../src/fortext/text.vs", "../src/fortext/text.fs");
    glm::mat4 projection = glm::mat4(1.0f);
    textShader.use();
    glUniformMatrix4fv(glGetUniformLocation(textShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // FreeType
    // --------
    FT_Library ft;
    // All functions return a value different than 0 whenever an error occurred
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return -1;
    }

	// find path to font
    std::string font_name = "/usr/share/fonts/truetype/ubuntu/Ubuntu-B.ttf";
    if (font_name.empty())
    {
        std::cout << "ERROR::FREETYPE: Failed to load font_name" << std::endl;
        return -1;
    }
	
	// load font as face
    FT_Face face;
    if (FT_New_Face(ft, font_name.c_str(), 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return -1;
    }
    else {
        // set size to load glyphs as
        FT_Set_Pixel_Sizes(face, 0, 48);

        // disable byte-alignment restriction
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // load first 128 characters of ASCII set
        for (unsigned char c = 0; c < 128; c++)
        {
            // Load character glyph 
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                continue;
            }
            // generate texture
            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
            );
            // set texture options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // now store character for later use
            Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<unsigned int>(face->glyph->advance.x)
            };
            Characters.insert(std::pair<char, Character>(c, character));
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    // destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    
    // configure VAO/VBO for texture quads
    // -----------------------------------
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    /************************************************************/

    // GAME RENDERING
    /************************************************************/

    // build and compile our shader zprogram
    // ------------------------------------
    Shader ourShader("../src/shaders/texture", "../src/shaders/fragment");
    ourShader.use();
    enableMatrices(ourShader, glm::mat4(1.0f), glm::mat4(1.0));

    // background image
    float backgroundVertices[] = {
    // positions          // colors           // texture coords
     1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
     1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
    -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left

     1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
    -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
    -1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left
    };

    unsigned int VBO;
    unsigned int backgroundVAO;
    genVertex(&VBO, &backgroundVAO, backgroundVertices, sizeof(backgroundVertices));
    unsigned int backgroundTexture;
    genTexture(&backgroundTexture, backgroundImagePath);

    // player vertices
    float playerSizef = 0.075f;
    float playerVertices[] = {
    // positions          // colors           // texture coords
     playerSizef,  playerSizef*SCR_RATIO, 0.0f,   1.0f, 0.83f, 0.0f,   1.0f, 1.0f,   // top right
     playerSizef, -playerSizef*SCR_RATIO, 0.0f,   1.0f, 0.83f, 0.0f,   1.0f, 0.0f,   // bottom right
    -playerSizef, -playerSizef*SCR_RATIO, 0.0f,   1.0f, 0.83f, 0.0f,   0.0f, 0.0f,   // bottom left

     playerSizef,  playerSizef*SCR_RATIO, 0.0f,   1.0f, 0.83f, 0.0f,   1.0f, 1.0f,   // top right
    -playerSizef, -playerSizef*SCR_RATIO, 0.0f,   1.0f, 0.83f, 0.0f,   0.0f, 0.0f,   // bottom left
    -playerSizef,  playerSizef*SCR_RATIO, 0.0f,   1.0f, 0.83f, 0.0f,   0.0f, 1.0f    // top left
    };
    unsigned int playerVAO;
    genVertex(&VBO, &playerVAO, playerVertices, sizeof(playerVertices));
    unsigned int playerTexture[3];
    genTexture(&playerTexture[0], "../src/textures/player/playerRun1.png");
    genTexture(&playerTexture[1], "../src/textures/player/playerRun2.png");
    genTexture(&playerTexture[2], "../src/textures/player/playerRun3.png");

    // zapper vertices
    float zapperSize = 0.075f;
    float diagonalZapperSize = 0.2f;
    float zapperRation = 5.5f;
    float zapperVertices[4][48] = {
    { // 0
    // positions          // colors           // texture coords
     zapperSize,  zapperSize*zapperRation, 0.0f,   1.0f, 0.83f, 0.0f,   1.0f, 1.0f,   // top right
     zapperSize, -zapperSize*zapperRation, 0.0f,   1.0f, 0.83f, 0.0f,   1.0f, 0.0f,   // bottom right
    -zapperSize, -zapperSize*zapperRation, 0.0f,   1.0f, 0.83f, 0.0f,   0.0f, 0.0f,   // bottom left

     zapperSize,  zapperSize*zapperRation, 0.0f,   1.0f, 0.83f, 0.0f,   1.0f, 1.0f,   // top right
    -zapperSize, -zapperSize*zapperRation, 0.0f,   1.0f, 0.83f, 0.0f,   0.0f, 0.0f,   // bottom left
    -zapperSize,  zapperSize*zapperRation, 0.0f,   1.0f, 0.83f, 0.0f,   0.0f, 1.0f    // top left
    },
    { // 1
    // positions          // colors           // texture coords
     zapperSize,  zapperSize*zapperRation, 0.0f,   1.0f, 0.83f, 0.0f,   1.0f, 1.0f,   // top right
     zapperSize, -zapperSize*zapperRation, 0.0f,   1.0f, 0.83f, 0.0f,   1.0f, 0.0f,   // bottom right
    -zapperSize, -zapperSize*zapperRation, 0.0f,   1.0f, 0.83f, 0.0f,   0.0f, 0.0f,   // bottom left

     zapperSize,  zapperSize*zapperRation, 0.0f,   1.0f, 0.83f, 0.0f,   1.0f, 1.0f,   // top right
    -zapperSize, -zapperSize*zapperRation, 0.0f,   1.0f, 0.83f, 0.0f,   0.0f, 0.0f,   // bottom left
    -zapperSize,  zapperSize*zapperRation, 0.0f,   1.0f, 0.83f, 0.0f,   0.0f, 1.0f    // top left
    },
    { // 2
    // positions          // colors           // texture coords
     zapperSize*zapperRation/SCR_RATIO,  zapperSize*SCR_RATIO, 0.0f,   1.0f, 0.83f, 0.0f,   1.0f, 0.0f,   // top right
     zapperSize*zapperRation/SCR_RATIO, -zapperSize*SCR_RATIO, 0.0f,   1.0f, 0.83f, 0.0f,   0.0f, 0.0f,   // bottom right
    -zapperSize*zapperRation/SCR_RATIO, -zapperSize*SCR_RATIO, 0.0f,   1.0f, 0.83f, 0.0f,   0.0f, 1.0f,   // bottom left

     zapperSize*zapperRation/SCR_RATIO,  zapperSize*SCR_RATIO, 0.0f,   1.0f, 0.83f, 0.0f,   1.0f, 0.0f,   // top right
    -zapperSize*zapperRation/SCR_RATIO, -zapperSize*SCR_RATIO, 0.0f,   1.0f, 0.83f, 0.0f,   0.0f, 1.0f,   // bottom left
    -zapperSize*zapperRation/SCR_RATIO,  zapperSize*SCR_RATIO, 0.0f,   1.0f, 0.83f, 0.0f,   1.0f, 1.0f    // top left
    },
    { // 3
    // positions          // colors           // texture coords
     diagonalZapperSize,  diagonalZapperSize*SCR_RATIO, 0.0f,   1.0f, 0.83f, 0.0f,   1.0f, 1.0f,   // top right
     diagonalZapperSize, -diagonalZapperSize*SCR_RATIO, 0.0f,   1.0f, 0.83f, 0.0f,   1.0f, 0.0f,   // bottom right
    -diagonalZapperSize, -diagonalZapperSize*SCR_RATIO, 0.0f,   1.0f, 0.83f, 0.0f,   0.0f, 0.0f,   // bottom left

     diagonalZapperSize,  diagonalZapperSize*SCR_RATIO, 0.0f,   1.0f, 0.83f, 0.0f,   1.0f, 1.0f,   // top right
    -diagonalZapperSize, -diagonalZapperSize*SCR_RATIO, 0.0f,   1.0f, 0.83f, 0.0f,   0.0f, 0.0f,   // bottom left
    -diagonalZapperSize,  diagonalZapperSize*SCR_RATIO, 0.0f,   1.0f, 0.83f, 0.0f,   0.0f, 1.0f    // top left
    }
    }
    ;
    unsigned int zapperVAO[4];
    for (int i = 0; i<4; i++)
        genVertex(&VBO, &zapperVAO[i], zapperVertices[i], sizeof(zapperVertices[i]));
    unsigned int zapperTexture[4];
    for (int i = 0; i<3; i++)
        genTexture(&zapperTexture[i], "../src/textures/zapper.png");
    // specific to only diaganol
    genTexture(&zapperTexture[3], "../src/textures/diagonalZapper.png");
 

    // for coins
    float coinSize = 0.075f;
    float coinVertices[] = {
    // positions          // colors           // texture coords
     coinSize,  coinSize*SCR_RATIO, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
     coinSize, -coinSize*SCR_RATIO, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
    -coinSize, -coinSize*SCR_RATIO, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left

     coinSize,  coinSize*SCR_RATIO, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
    -coinSize, -coinSize*SCR_RATIO, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
    -coinSize,  coinSize*SCR_RATIO, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left
    };
    float blankSize = 0.0f;
    float blankVertices[] = {
    // positions          // colors           // texture coords
     blankSize,  blankSize*SCR_RATIO, 1.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
     blankSize, -blankSize*SCR_RATIO, 1.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
    -blankSize, -blankSize*SCR_RATIO, 1.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left

     blankSize,  blankSize*SCR_RATIO, 1.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
    -blankSize, -blankSize*SCR_RATIO, 1.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
    -blankSize,  blankSize*SCR_RATIO, 1.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left
    };
    unsigned int coinVAO[2];
    genVertex(&VBO, &coinVAO[1], coinVertices, sizeof(coinVertices));
    genVertex(&VBO, &coinVAO[0], blankVertices, sizeof(blankVertices));
    unsigned int coinTexture;
    genTexture(&coinTexture, "../src/textures/coin.png");

    // for pillars
    float pillarWidth = 0.15f, pillarHeight = 0.5f;
    float pilarVertices[] = {
    // positions          // colors           // texture coords
     pillarWidth,  pillarHeight, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
     pillarWidth, -pillarHeight, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
    -pillarWidth, -pillarHeight, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left

     pillarWidth,  pillarHeight, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
    -pillarWidth, -pillarHeight, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
    -pillarWidth,  pillarHeight, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left
    };
    unsigned int pillarVAO;
    genVertex(&VBO, &pillarVAO, pilarVertices, sizeof(pilarVertices));
    unsigned int pillarTexture;
    genTexture(&pillarTexture, "../src/textures/pillar.png");

    // objects and other things
    Game Jetpack("Vineeth");
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    Sprite Background1(glm::vec3(0.0f, 0.0f, 0.0f));
    Sprite Background2(glm::vec3(2.0f, 0.0f, 0.0f));
    Player Player(glm::vec3(playerInitx, playerInity, 0.0f), 0.9f);

    levelChanger Level(glm::vec3(1.0f, -0.4f, 0.0f));

    Zapper Zapper1(glm::vec3(1.6f, 0.0f, 0.0f));
    Zapper Zapper2(glm::vec3(2.4f, 0.0f, 0.0f));
    Zapper Zapper3(glm::vec3(3.2f, 0.0f, 0.0f));

    Coin Coin1(glm::vec3(2.0f, 0.0f, 0.0f));
    Coin Coin2(glm::vec3(2.8f, 0.0f, 0.0f));
    Coin Coin3(glm::vec3(3.6f, 0.0f, 0.0f));


    /************************************************************/

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        backgroundShiftSpeed = -Jetpack.frameSpeeds[Jetpack.level];

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;       
        // input
        // -----
        processInput(window, Player);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // render container
        ourShader.use();            


        // rendering backgrounds
        /*****************************************/
        Background1.setModel(model, deltaTime*backgroundShiftSpeed, 0, 0);
        glUniform1f(glGetUniformLocation(ourShader.ID, "enableSmoothstep"), 
            Background1.enableSmoothstep);
        renderShape(backgroundVAO, backgroundTexture, ourShader, 6, model, proj);
        identify(model);  
        if (Background1.currentCoordinates.x <= -2.0f){
            Background1.SpriteTranslate(4.0f, 0.0f, 0.0f);
        }

        Background2.setModel(model, deltaTime*backgroundShiftSpeed, 0, 0);
        glUniform1f(glGetUniformLocation(ourShader.ID, "enableSmoothstep"), 
            Background2.enableSmoothstep);
        renderShape(backgroundVAO, backgroundTexture, ourShader, 6, model, proj);
        identify(model);
        if (Background2.currentCoordinates.x <= -2.0f){
            Background2.SpriteTranslate(4.0f, 0.0f, 0.0f);
        }
        /*****************************************/

    
        
        // rendering levelChanger
        /*****************************************/
        Level.setModel(model, deltaTime*backgroundShiftSpeed, 0, 0);
        glUniform1f(glGetUniformLocation(ourShader.ID, "enableSmoothstep"), 
            Level.enableSmoothstep);
        renderShape(pillarVAO, pillarTexture, ourShader, 6, model, proj);
        identify(model);  
        Level.check(Jetpack, Player);
        /*****************************************/



        // rendering player
        /*****************************************/
        Player.setModel(model, 0, 0, 0);
        glUniform1f(glGetUniformLocation(ourShader.ID, "enableSmoothstep"), 
            Player.enableSmoothstep);
        Player.activateDrop(model);
        if (glfwGetTime() - Player.lastTime > Player.textureChangeTime){
            Player.lastTime = glfwGetTime();
            Player.playerRunningIndex++;
            Player.playerRunningIndex%=3;
        }

        if (!Player.isFlying){
            Player.enableSmoothstep = 0.0;
            renderShape(playerVAO, playerTexture[Player.playerRunningIndex], ourShader, 6, model, proj);
        }else{
            renderShape(playerVAO, playerTexture[Player.playerFlyingIndex], ourShader, 6, model, proj);
        identify(model);
        }
        Player.playerAcceleration = Player.gravityAcceleration;
        /*****************************************/



        // rendering obstacles
        /*****************************************/
        Zapper1.setModel(model, deltaTime*backgroundShiftSpeed, 0, 0);
        glUniform1f(glGetUniformLocation(ourShader.ID, "enableSmoothstep"), 
            Zapper1.enableSmoothstep);
        renderShape(zapperVAO[Zapper1.textureStyle], zapperTexture[Zapper1.textureStyle], ourShader, 6, model, proj);
        identify(model);
        Zapper1.check(Jetpack, Player);

        Zapper2.setModel(model, deltaTime*backgroundShiftSpeed, 0, 0);
        glUniform1f(glGetUniformLocation(ourShader.ID, "enableSmoothstep"), 
            Zapper2.enableSmoothstep);
        renderShape(zapperVAO[Zapper2.textureStyle], zapperTexture[Zapper2.textureStyle], ourShader, 6, model, proj);
        identify(model);
        Zapper2.check(Jetpack, Player);

        Zapper3.setModel(model, deltaTime*backgroundShiftSpeed, 0, 0);
        glUniform1f(glGetUniformLocation(ourShader.ID, "enableSmoothstep"), 
            Zapper3.enableSmoothstep);
        renderShape(zapperVAO[Zapper3.textureStyle], zapperTexture[Zapper3.textureStyle], ourShader, 6, model, proj);
        identify(model);
        Zapper3.check(Jetpack, Player);
        /*****************************************/

        // rendering coins
        /*****************************************/
        Coin1.setModel(model, deltaTime*backgroundShiftSpeed, 0, 0);
        glUniform1f(glGetUniformLocation(ourShader.ID, "enableSmoothstep"), 
            Coin1.enableSmoothstep);
        renderShape(coinVAO[Coin1.isExists], coinTexture, ourShader, 6, model, proj);
        identify(model);
        Coin1.check(Jetpack, Player);

        Coin2.setModel(model, deltaTime*backgroundShiftSpeed, 0, 0);
        glUniform1f(glGetUniformLocation(ourShader.ID, "enableSmoothstep"), 
            Coin2.enableSmoothstep);
        renderShape(coinVAO[Coin2.isExists], coinTexture, ourShader, 6, model, proj);
        identify(model);
        Coin2.check(Jetpack, Player);

        Coin3.setModel(model, deltaTime*backgroundShiftSpeed, 0, 0);
        glUniform1f(glGetUniformLocation(ourShader.ID, "enableSmoothstep"), 
            Coin3.enableSmoothstep);
        renderShape(coinVAO[Coin3.isExists], coinTexture, ourShader, 6, model, proj);
        identify(model);
        Coin3.check(Jetpack, Player);
        /*****************************************/

        fflush(stdout);

        // Checking for collisions
        /*****************************************/
        if (Jetpack.zapperCollision || Jetpack.isGameWon)
            break;
        /*****************************************/


        // Rendering text
        /*****************************************/
        RenderText(textShader, "Level: " + std::to_string(Jetpack.level), -0.95f, -0.9f, 0.001f, glm::vec3(1.0f, 1.0f, 1.0f));
        RenderText(textShader, "Completed: " + std::to_string((int)(Jetpack.curLengthTravelled*100)) + 
            "/"+std::to_string((int)(Jetpack.levelLength*100)), -0.95f, 0.8f, 0.001f, glm::vec3(1.0f, 1.0f, 1.0f));
        RenderText(textShader, "Score: " + std::to_string(Jetpack.score), -0.95f, 0.9f, 0.001f, glm::vec3(1.0f, 1.0f, 1.0f));
        /*****************************************/

        // updating distance travelled
        if (Jetpack.started)
            Jetpack.curLengthTravelled -= deltaTime*backgroundShiftSpeed;
        else
            RenderText(textShader, "Get Ready for level 1", -0.95f, 0.3f, 0.0015f, glm::vec3(1.0f, 1.0f, 1.0f));


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    if (Jetpack.zapperCollision){
        genTexture(&backgroundTexture, "../src/textures/gameover.png");
    } else {
        genTexture(&backgroundTexture, "../src/textures/gamewin.png");
    }
        while (!glfwWindowShouldClose(window))
        {
            processInput(window, Player);

            // render
            // ------
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            
            ourShader.use();
            renderShape(backgroundVAO, backgroundTexture, ourShader, 6, model, proj);

            // Rendering loss page
            /*****************************************/
            RenderText(textShader, "Final Score: " + std::to_string(Jetpack.score), -0.95f, -0.9f, 0.002f, glm::vec3(1.0f, 1.0f, 1.0f));
            /*****************************************/


            // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
            // -------------------------------------------------------------------------------
            glfwSwapBuffers(window);
            glfwPollEvents();
        }

    glfwTerminate();
    return 0;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window, Player& Player)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
        Player.fly(model);
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// render line of text
// -------------------
void RenderText(Shader &shader, std::string text, float x, float y, float scale, glm::vec3 color)
{
    // activate corresponding render state	
    shader.use();
    glUniform3f(glGetUniformLocation(shader.ID, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) 
    {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },            
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }           
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}