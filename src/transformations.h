#ifndef _TRANSFORMATIONS_H_
#define _TRANSFORMATIONS_H_

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstdlib>
#include <cmath>

#include "shader.h"
#include "module.h"

void translate(glm::mat4& matrix, float x, float y, float z);

void rotate(glm::mat4& matrix, float degreeAngle);

class Game{
    public:
        unsigned int score;
        unsigned int level;
        float spriteDist;
        int spriteCount;
        bool zapperCollision;
        bool isGameWon;
        int numSpritesPerLevel;
        int curSpritesNum;
        float levelLength;
        float curLengthTravelled;
        bool started;

        float frameSpeeds[4];

        Game(const char* playerName){
            this->score = 0;
            this->level = 0;
            {
                this->frameSpeeds[0] = 0.45;
                this->frameSpeeds[1] = 0.50;
                this->frameSpeeds[2] = 0.70;
                this->frameSpeeds[3] = 0.80;
            }
            this->spriteDist = 0.8f;
            this->spriteCount = 3;
            this->zapperCollision = false;
            this->isGameWon = false;
            this->numSpritesPerLevel = 15;
            this->curSpritesNum = 0;
            this->levelLength = this->numSpritesPerLevel*this->spriteDist;
            this->curLengthTravelled = 0;
            this->started = false;
        }
};

class Sprite{
    public:
        glm::mat4 SpriteModel;
        glm::vec3 currentCoordinates;
        float enableSmoothstep;

        Sprite(){
            ; // does nothing
        }

        Sprite(glm::vec3 currentCoordinates){
            this->enableSmoothstep = 0.0;

            this->currentCoordinates = currentCoordinates;
            this->SpriteModel = glm::mat4(1.0f);
            translate(this->SpriteModel, this->currentCoordinates.x,
                this->currentCoordinates.y,
                this->currentCoordinates.z);
        }

        void SpriteTranslate(float x, float y, float z){
            translate(this->SpriteModel, x, y, z);
            this->currentCoordinates.x += x;
            this->currentCoordinates.y += y;
            this->currentCoordinates.z += z;
        }

        void SpriteRotate(float degreeAngle){
            glm::vec3 initPos = this->currentCoordinates;
            this->SpriteTranslate(-initPos.x,
                    -initPos.y, -initPos.z);
            rotate(this->SpriteModel, degreeAngle);
            this->SpriteTranslate(initPos.x,
                    initPos.y, initPos.z);
        }

        void setModel(glm::mat4& model, 
            float x, float y, float z){
            this->SpriteTranslate(x, y, z);
            model = this->SpriteModel;
        }
};

class Player: public Sprite{
    public:
        float ceilingHeight;
        float initFloor;
        float lastTime;
        int playerRunningIndex;
        float textureChangeTime;
        int playerFlyingIndex;
        bool isFlying;
        float playerSpeed;
        float gravityAcceleration;
        float verticalAcceleration;
        float timeInterval;
        float lastUpdatedTime;
        float playerAcceleration;

        Player(glm::vec3 currentCoordinates, float ceilingHeight){
            this->enableSmoothstep = 0.0;

            initFloor = currentCoordinates.y;
            this->currentCoordinates = currentCoordinates;
            this->SpriteModel = glm::mat4(1.0f);
            translate(this->SpriteModel, this->currentCoordinates.x,
                this->currentCoordinates.y,
                this->currentCoordinates.z);
            this->ceilingHeight = ceilingHeight;
            this->lastTime = glfwGetTime();
            this->playerRunningIndex = 0;
            this->textureChangeTime = 0.1;
            this->playerFlyingIndex = 2;
            this->isFlying = false;
            this->playerSpeed = 0.00f;
            this->verticalAcceleration = 9.0f;
            this->gravityAcceleration = -5.0f;
            this->timeInterval = 0.0f;
            this->lastUpdatedTime = glfwGetTime();
            this->playerAcceleration = 0.0f;
        }

        void setModel(glm::mat4& model, 
            float x, float y, float z){
            
            if (this->currentCoordinates.y +  y 
                > this->ceilingHeight){
                    this->playerSpeed = 0;
                    return;
            }

            if (this->currentCoordinates.y + y 
                < initFloor){
                    this->isFlying = false;
                    this->playerSpeed = 0;
                    return;
            }

            this->SpriteTranslate(x, y, z);
            model = this->SpriteModel;

            if (this->currentCoordinates.y <= this->initFloor)
                this->isFlying = false;
            else
                this->isFlying = true;
        }

        void updateTime(){
            this->timeInterval = 
                glfwGetTime() - this->lastUpdatedTime;
            this->lastUpdatedTime = glfwGetTime();
            // std::cout<<timeInterval<<std::endl;
        }

        void fly(glm::mat4& model){
            this->playerAcceleration = this->verticalAcceleration;
            this->enableSmoothstep = 1.0f;
        }

        void activateDrop(glm::mat4& model){
            this->updateTime();

            if (this->playerAcceleration < 0){
                this->enableSmoothstep = 0.0f;
            }

            this->playerSpeed += 
                this->timeInterval*this->playerAcceleration;
            this->setModel(model, 0,
                    this->playerSpeed*this->timeInterval
                        + 0.5*this->playerAcceleration*this->timeInterval*this->timeInterval,
            0);
        }
};

void identify(glm::mat4& matrix);

class Zapper: public Sprite{
    public:
        int textureStyle;
        /*
            0 -> vertical
            1 -> vertical and moving up and down
            2 -> horizontal
            3 -> diagonal
        */
        bool goingUpwards; 
        float verticalSpeed;

        void genInitPos(){
            float rand01 = rand() / static_cast<float>(RAND_MAX);
            rand01 = 2*rand01 - 1;
            rand01 *= 0.75;
            this->currentCoordinates.y = rand01;
        }

        Zapper(glm::vec3 currentCoordinates){
            this->enableSmoothstep = 1.0;

            this->currentCoordinates = currentCoordinates;
            this->SpriteModel = glm::mat4(1.0f);

            this->textureStyle = rand()%4;
            this->genInitPos();

            translate(this->SpriteModel, this->currentCoordinates.x,
                this->currentCoordinates.y,
                this->currentCoordinates.z);

            // only for textureStyle == 1
            this->goingUpwards = true;
            this->verticalSpeed = 0.0075f;
        }

        void genAgain(){
            this->textureStyle = rand()%4;
            this->genInitPos();

            this->SpriteModel = glm::mat4(1.0f);

            translate(this->SpriteModel, this->currentCoordinates.x,
                this->currentCoordinates.y,
                this->currentCoordinates.z);
        }

        void doVerticalTranslation(){
            if (this->goingUpwards){
                this->SpriteTranslate(0, this->verticalSpeed, 0);
            } else {
                this->SpriteTranslate(0, -this->verticalSpeed, 0);
            }

            if (this->currentCoordinates.y >= 0.75){
                this->goingUpwards = false;
            } 
            
            if (this->currentCoordinates.y <= -0.75){
                this->goingUpwards = true;
            }
        }

        void checkCollision(Game& game, const Player& player){

            if (this->textureStyle == 1 || this->textureStyle == 0){
                if (fabs(player.currentCoordinates.x - 
                    currentCoordinates.x) < 0.075f
                    &&
                    fabs(player.currentCoordinates.y - 
                    currentCoordinates.y) < 0.40f){
                    std::cout<<"Collision!"<<std::endl;
                    game.zapperCollision = true;
                }
            }

            if (this->textureStyle == 2){
                if (fabs(player.currentCoordinates.x - 
                    currentCoordinates.x) < 0.245
                    &&
                    fabs(player.currentCoordinates.y - 
                    currentCoordinates.y) < 0.11){
                    std::cout<<"Collision!"<<std::endl;
                    game.zapperCollision = true;
                }
            }

            if (this->textureStyle == 3){
                float leftX = this->currentCoordinates.x - 0.2f;
                float rightX = this->currentCoordinates.x + 0.2f;

                float leftY = this->currentCoordinates.y - 0.26;
                float rightY = this->currentCoordinates.y + 0.26;

                float zapperLength = 
                    2*sqrt(0.2*0.2 + 0.26*0.26);

                float playerDistance = 
                    sqrt(
                        (player.currentCoordinates.x - leftX)*
                            (player.currentCoordinates.x - leftX) + 
                        (player.currentCoordinates.y - leftY)*
                            (player.currentCoordinates.y - leftY)
                    ) + 
                    sqrt(
                        (player.currentCoordinates.x - rightX)*
                            (player.currentCoordinates.x - rightX) + 
                        (player.currentCoordinates.y - rightY)*
                            (player.currentCoordinates.y - rightY)
                    );

                if (fabs(playerDistance - zapperLength) < 0.05){
                    std::cout<<"Collision!"<<std::endl;
                    game.zapperCollision = true;
                }
            }     
        }

        void check(Game& game, const Player& player){
            if (this->currentCoordinates.x <= -1.15f){
                this->SpriteTranslate(
                    game.spriteCount*game.spriteDist
                    , 0, 0);

                this->genAgain();
            }

            if (this->textureStyle == 1){
                this->doVerticalTranslation();
            }

            this->checkCollision(game, player);
        }
};

class Coin: public Sprite{
    public:
        bool goingUpwards;
        float verticalSpeed;
        float translationProbability;
        bool isExists;
        float xBias;
        float randGenFloat;

        void genInitPos(){
            float rand01 = rand() / static_cast<float>(RAND_MAX);
            rand01 = 2*rand01 - 1;
            rand01 *= 0.75;
            this->currentCoordinates.y = rand01;

            this->xBias = genRand(this->randGenFloat);
            this->currentCoordinates.x += xBias;
        }

        Coin(glm::vec3 currentCoordinates){
            this->enableSmoothstep = 0.0;

            this->randGenFloat = 0.2;

            this->currentCoordinates = currentCoordinates;
            this->xBias = genRand(this->randGenFloat);
            this->currentCoordinates.x += xBias;


            this->SpriteModel = glm::mat4(1.0f);

            this->genInitPos();

            translate(this->SpriteModel, this->currentCoordinates.x,
                this->currentCoordinates.y,
                this->currentCoordinates.z);

            this->translationProbability = rand() / static_cast<float>(RAND_MAX);
            this->goingUpwards = true;
            this->verticalSpeed = 0.0075f;
            this->isExists = true;
        }

        void checkCollision(Game& game, const Player& player){
            if (!isExists)
                return;

            float ellipseA = 0.03f;
            float ellipseB = 0.1f;

            float playerDistance = 
                (player.currentCoordinates.x - 
                    this->currentCoordinates.x) *
                (player.currentCoordinates.x - 
                    this->currentCoordinates.x)
                /ellipseA   +
                (player.currentCoordinates.y - 
                    this->currentCoordinates.y) *
                (player.currentCoordinates.y - 
                    this->currentCoordinates.y)
                /ellipseB;

            if (playerDistance < 1){
                game.score++;
                this->isExists = false;
            }
        }

        void genAgain(){
            this->genInitPos();

            this->SpriteModel = glm::mat4(1.0f);

            translate(this->SpriteModel, this->currentCoordinates.x,
                this->currentCoordinates.y,
                this->currentCoordinates.z);

            this->translationProbability = rand() / static_cast<float>(RAND_MAX);
            this->isExists = true;
        }

        void doVerticalTranslation(){
            if (this->goingUpwards){
                this->SpriteTranslate(0, this->verticalSpeed, 0);
            } else {
                this->SpriteTranslate(0, -this->verticalSpeed, 0);
            }

            if (this->currentCoordinates.y >= 0.75){
                this->goingUpwards = false;
            } 
            
            if (this->currentCoordinates.y <= -0.75){
                this->goingUpwards = true;
            }
        }

        void check(Game& game, const Player& player){
            if (this->currentCoordinates.x <= -1.15f){
                this->SpriteTranslate(
                    game.spriteCount*game.spriteDist
                    , 0, 0);

                this->currentCoordinates.x -= xBias;
                this->genAgain();
            }

            if (this->translationProbability > 0.7)
                this->doVerticalTranslation();

            this->checkCollision(game, player);
        }
};

class levelChanger: public Sprite{
    public:
        bool levelChanged;

        levelChanger(glm::vec3 currentCoordinates){
            this->enableSmoothstep = 0.0;
            
            this->currentCoordinates = currentCoordinates;
            this->SpriteModel = glm::mat4(1.0f);
            translate(this->SpriteModel, this->currentCoordinates.x,
                this->currentCoordinates.y,
                this->currentCoordinates.z);
            this->levelChanged = false;
        }

        void check(Game& game, const Player& player){
            if (fabs(this->currentCoordinates.x - player.currentCoordinates.x) < 0.1f){
                game.started = true;
                game.curLengthTravelled = 0;

                if (!this->levelChanged){
                    game.level++;
                    if (game.level == 4){
                        game.level = 3;
                        game.isGameWon = true;
                    }
                    this->levelChanged = true;
                }
            }

            if (this->currentCoordinates.x <= -1.15f){
                this->SpriteTranslate(
                    game.numSpritesPerLevel*game.spriteDist
                    , 0, 0);
                this->levelChanged = false;
            }
        }
};

#endif