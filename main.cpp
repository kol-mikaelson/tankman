#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <random> // For better random number generation

// GLEW - Must be included before GLFW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp> // For length2

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;

// Camera
glm::vec3 cameraPos   = glm::vec3(0.0f, 15.0f, 20.0f); // Initial position for perspective
glm::vec3 cameraFront = glm::vec3(0.0f, -1.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);
bool firstMouse = true;
float yaw   = -90.0f;
float pitch = -30.0f; // Initial pitch to look down slightly
float lastX =  WIDTH  / 2.0;
float lastY =  HEIGHT / 2.0;
float fov   =  45.0f;
bool perspectiveView = true;

// Timing
float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f;

// Game state
float enemySpawnTimer = 0.0f;
const float ENEMY_SPAWN_INTERVAL = 3.0f;
float heroBulletSpawnTimer = 0.0f;
const float HERO_BULLET_SPAWN_INTERVAL = 5.0f;
int heroScore = 0;
const int WIN_SCORE = 10;
bool gameOver = false;

// Random number generation
std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
std::uniform_real_distribution<float> dist(0.0f, 1.0f);

// Simple struct for a Tank
struct Tank {
    glm::vec3 position;
    glm::vec3 color;
    float size;
    bool isHero;
    float health; // Could be used for more complex gameplay
    glm::vec3 direction; // For enemy movement
    float speed;         // For enemy movement

    Tank(glm::vec3 pos, glm::vec3 col, float sz, bool hero = false)
        : position(pos), color(col), size(sz), isHero(hero), health(100.0f), direction(0.0f), speed(0.0f) {
        if (!isHero) {
            // Initialize random direction and speed for enemies
            float angle = dist(rng) * 2.0f * 3.1415926535f; // Use better random
            direction = glm::normalize(glm::vec3(cos(angle), 0.0f, sin(angle))); // Movement on XZ plane
            speed = 1.5f; // Fixed pace for enemies, adjust as needed
        }
    }
};

// Simple struct for a Bullet
struct Bullet {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 color;
    float size;
    bool active;
    float lifetime; // To remove old bullets

    Bullet(glm::vec3 pos, glm::vec3 vel, glm::vec3 col, float sz)
        : position(pos), velocity(vel), color(col), size(sz), active(true), lifetime(5.0f) {} // Bullet lives for 5 seconds
};

std::vector<Tank> enemyTanks;
Tank heroTank(glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(0.0f, 0.5f, 1.0f), 1.0f, true); // Hero tank at center
std::vector<Bullet> bullets;

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void do_movement();
GLuint compileShader(const char* shaderCode, GLenum shaderType);
GLuint createShaderProgram(const char* vertexShaderCode, const char* fragmentShaderCode);
void renderCube(GLuint shaderProgram, glm::vec3 position, glm::vec3 size, glm::vec3 color, const glm::mat4& view, const glm::mat4& projection);

// Shader sources (very basic)
const char* vertexShaderSource = R"glsl(
    #version 330 core
    layout (location = 0) in vec3 aPos;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main()
    {
        gl_Position = projection * view * model * vec4(aPos, 1.0);
    }
)glsl";

const char* fragmentShaderSource = R"glsl(
    #version 330 core
    out vec4 FragColor;

    uniform vec3 objectColor;

    void main()
    {
        FragColor = vec4(objectColor, 1.0);
    }
)glsl";

// Cube vertices
float cubeVertices[] = {
    // positions         
    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,

    -0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,

    -0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,

     0.5f,  0.5f,  0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,

    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f, -0.5f,

    -0.5f,  0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f,  0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f, -0.5f,
};
GLuint cubeVAO, cubeVBO;


int main() {
    // Init GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE); // Or GL_TRUE if you want resizable

    // Create a windowed mode window and its OpenGL context
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Tank Game", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Set the required callback functions
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // GLFW Options: Capture mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Init GLEW
    glewExperimental = GL_TRUE; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Define the viewport dimensions
    int VpWidth, VpHeight;
    glfwGetFramebufferSize(window, &VpWidth, &VpHeight);
    glViewport(0, 0, VpWidth, VpHeight);

    // OpenGL state
    glEnable(GL_DEPTH_TEST);

    // Compile and link shaders
    GLuint shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);
    if (shaderProgram == 0) {
        glfwTerminate();
        return -1;
    }

    // Setup cube VAO and VBO
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    glBindVertexArray(cubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind VBO
    glBindVertexArray(0); // Unbind VAO


    // Game loop
    while (!glfwWindowShouldClose(window)) {
        // Calculate delta time
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Input
        glfwPollEvents();
        if (perspectiveView) { // Only allow camera movement in perspective view
             do_movement();
        }

        // Update game logic
        if (!gameOver) {
            // Enemy spawning
            enemySpawnTimer += deltaTime;
            if (enemySpawnTimer >= ENEMY_SPAWN_INTERVAL) {
                enemySpawnTimer = 0.0f;
                float angle = dist(rng) * 2.0f * 3.1415926535f;
                float distance = 15.0f + dist(rng) * 5.0f; // Spawn 15-20 units away
                glm::vec3 enemyPos(cos(angle) * distance, 0.5f, sin(angle) * distance);
                enemyTanks.emplace_back(enemyPos, glm::vec3(1.0f, 0.0f, 0.0f), 1.0f); // Red enemies
            }

            // Hero bullet spawning
            heroBulletSpawnTimer += deltaTime;
            if (heroBulletSpawnTimer >= HERO_BULLET_SPAWN_INTERVAL && !enemyTanks.empty()) {
                heroBulletSpawnTimer = 0.0f;
                
                // Find closest enemy
                Tank* targetEnemy = nullptr;
                float minDistanceSq = FLT_MAX;
                for (auto& enemy : enemyTanks) {
                    if (enemy.isHero) continue; // Don't target self if hero was in this list by mistake
                    float distSq = glm::length2(enemy.position - heroTank.position);
                    if (distSq < minDistanceSq) {
                        minDistanceSq = distSq;
                        targetEnemy = &enemy;
                    }
                }

                if (targetEnemy) {
                    glm::vec3 direction = glm::normalize(targetEnemy->position - heroTank.position);
                    glm::vec3 bulletStartPos = heroTank.position + direction * (heroTank.size * 0.6f); // Start slightly in front of hero
                    bullets.emplace_back(bulletStartPos, direction * 10.0f, glm::vec3(0.0f, 1.0f, 1.0f), 0.2f); // Cyan bullets, speed 10
                }
            }

            // Update Enemy Tank positions
            for (auto& enemy : enemyTanks) {
                if (enemy.isHero) continue; 

                enemy.position += enemy.direction * enemy.speed * deltaTime;

                // Optional: Basic boundary check and direction change
                float boundary = 25.0f; // Define play area boundary
                if (glm::length(enemy.position) > boundary) {
                    // If an enemy goes too far, give it a new random direction
                    // And optionally move it back slightly to prevent getting stuck outside
                    enemy.position = glm::normalize(enemy.position) * (boundary - 0.1f); // Pull back a tiny bit
                    float angle = dist(rng) * 2.0f * 3.1415926535f;
                    enemy.direction = glm::normalize(glm::vec3(cos(angle), 0.0f, sin(angle)));
                }
            }

            // Update bullets
            for (auto it = bullets.begin(); it != bullets.end(); ) {
                if (!it->active) {
                    it = bullets.erase(it);
                    continue;
                }
                it->position += it->velocity * deltaTime;
                it->lifetime -= deltaTime;
                if (it->lifetime <= 0) {
                    it->active = false; // Mark for removal
                }
                ++it;
            }
            
            // Collision detection: Bullets vs Enemy Tanks
            for (auto& bullet : bullets) {
                if (!bullet.active) continue;
                for (auto enemyIt = enemyTanks.begin(); enemyIt != enemyTanks.end(); ) {
                    if (enemyIt->isHero) { // Bullets shouldn't hit the hero tank
                        ++enemyIt;
                        continue;
                    }
                    float dist = glm::length(bullet.position - enemyIt->position);
                    // A simple collision check: distance between centers < sum of half-sizes
                    // Assuming tanks are roughly cube-like, their "radius" is size/2
                    if (dist < (bullet.size / 2.0f + enemyIt->size / 2.0f) ) { 
                        bullet.active = false; // Bullet is used up
                        enemyIt = enemyTanks.erase(enemyIt); // Enemy destroyed
                        heroScore++;
                        std::cout << "Hit! Score: " << heroScore << std::endl;
                        if (heroScore >= WIN_SCORE) {
                            gameOver = true;
                            std::cout << "HERO WINS!" << std::endl;
                        }
                        break; // Bullet can only hit one enemy
                    } else {
                        ++enemyIt;
                    }
                }
            }
        }


        // Render
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Dark grey background
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // View and Projection matrices
        glm::mat4 view_matrix; // Renamed to avoid conflict with glm::view
        glm::mat4 projection_matrix; // Renamed to avoid conflict

        if (perspectiveView) {
            view_matrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
            projection_matrix = glm::perspective(glm::radians(fov), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
        } else { // Top-down view
            float orthoHeight = 25.0f; // Adjusted for better view of moving enemies
            float aspectRatio = (float)WIDTH / (float)HEIGHT;
            view_matrix = glm::lookAt(glm::vec3(0.0f, 30.0f, 0.01f), 
                               glm::vec3(0.0f, 0.0f, 0.0f), 
                               glm::vec3(0.0f, 1.0f, 0.0f)); 
            projection_matrix = glm::ortho(-aspectRatio * orthoHeight / 2.0f, aspectRatio * orthoHeight / 2.0f, 
                                    -orthoHeight / 2.0f, orthoHeight / 2.0f, 
                                    0.1f, 100.0f);
        }
        
        // Render Hero Tank
        renderCube(shaderProgram, heroTank.position, glm::vec3(heroTank.size), heroTank.color, view_matrix, projection_matrix);

        // Render Enemy Tanks
        for (const auto& enemy : enemyTanks) {
            renderCube(shaderProgram, enemy.position, glm::vec3(enemy.size), enemy.color, view_matrix, projection_matrix);
        }

        // Render Bullets
        for (const auto& bullet : bullets) {
            if (bullet.active) {
                renderCube(shaderProgram, bullet.position, glm::vec3(bullet.size), bullet.color, view_matrix, projection_matrix);
            }
        }
        
        // Render a simple ground plane (large thin cube)
        renderCube(shaderProgram, glm::vec3(0.0f, -0.1f, 0.0f), glm::vec3(50.0f, 0.2f, 50.0f), glm::vec3(0.3f, 0.3f, 0.3f), view_matrix, projection_matrix); // Made ground larger


        glBindVertexArray(0); // Unbind VAO

        // Swap the screen buffers
        glfwSwapBuffers(window);
    }

    // De-allocate all resources once they've outlived their purpose
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteProgram(shaderProgram);

    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();
    return 0;
}

void renderCube(GLuint shaderProgram, glm::vec3 position, glm::vec3 sizeVec, glm::vec3 color, const glm::mat4& view, const glm::mat4& projection) {
    glUseProgram(shaderProgram);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, sizeVec); 

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, glm::value_ptr(color));

    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}


GLuint compileShader(const char* shaderCode, GLenum shaderType) {
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderCode, NULL);
    glCompileShader(shader);

    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
        return 0;
    }
    return shader;
}

GLuint createShaderProgram(const char* vertexShaderCode, const char* fragmentShaderCode) {
    GLuint vertexShader = compileShader(vertexShaderCode, GL_VERTEX_SHADER);
    if (vertexShader == 0) return 0;
    GLuint fragmentShader = compileShader(fragmentShaderCode, GL_FRAGMENT_SHADER);
    if (fragmentShader == 0) {
        glDeleteShader(vertexShader);
        return 0;
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    GLint success;
    GLchar infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return shaderProgram;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_T) {
            perspectiveView = false;
        }
        if (key == GLFW_KEY_P) {
            perspectiveView = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            firstMouse = true; 
        }
    }
}

// bool keys[1024]; // This global array is not used, can be removed.

void do_movement() {
    float cameraSpeed = 5.0f * deltaTime; 
    if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (!perspectiveView) { 
        // When not in perspective view, make cursor normal to allow clicking outside, etc.
        // However, for this game, we might want to keep it disabled or handle it differently
        // if we add UI elements. For now, just return if not in perspective.
        // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // Example if you want to release
        return;
    }
    // Ensure cursor is captured for perspective view if it was released
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


    if (firstMouse) {
        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);
        firstMouse = false;
    }

    float xoffset = static_cast<float>(xpos) - lastX;
    float yoffset = lastY - static_cast<float>(ypos); 
    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);

    float sensitivity = 0.1f; 
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (perspectiveView) { 
        fov -= (float)yoffset;
        if (fov < 1.0f)
            fov = 1.0f;
        if (fov > 60.0f) // Increased max FOV slightly
            fov = 60.0f;
    }
}

