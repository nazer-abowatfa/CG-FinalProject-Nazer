#include <GL/glew.h>      
#include <GLFW/glfw3.h>  
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"    //  ÂÌ∆… «·»Ì∆…:  Õ„Ì· «·’Ê— ··Œ«„« 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <string>

// ‰Ÿ«„ «· Ÿ·Ì·: Vertex Shader - Õ”«» «·≈Õœ«ÀÌ«  Ê«·≈Õœ«ÀÌ«  «·‰”ÌÃÌ…
const char* vertexShaderSource = R"glsl(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
out vec2 TexCoord;
out vec3 FragPos;
uniform mat4 model, view, projection;
void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    TexCoord = aTexCoord;
    gl_Position = projection * view * vec4(FragPos, 1.0); // „’›Ê›«  «· ÕÊÌ·: Model/View/Projection
}
)glsl";

// ‰Ÿ«„ «· Ÿ·Ì·: Fragment Shader - «·Œ«„«  Ê«· √ÀÌ—«  «·»’—Ì… (≈÷«¡… + ÷»«»)
const char* fragmentShaderSource = R"glsl(
#version 330 core
out vec4 FragColor;
in vec2 TexCoord;
in vec3 FragPos;
uniform sampler2D texture1;                     // «·Œ«„« : —»ÿ Texture »«·Shader
uniform vec3 viewPos;
uniform bool isPlayer, isMoving, isCoin;
void main() {
    vec4 texColor = texture(texture1, TexCoord); // «·Œ«„« : «” Œœ«„ Linear Filtering «› —«÷Ì
    if(isPlayer) texColor = isMoving ? vec4(0.0, 0.8, 0.2, 1.0) : vec4(0.0, 0.4, 0.9, 1.0); //  €ÌÌ— ·Ê‰ «·‘Œ’Ì… Õ”» «·Õ—ﬂ…
    if(isCoin) texColor = vec4(1.0, 0.8, 0.0, 1.0); // «·⁄„·« : ·Ê‰ –Â»Ì
    float dist = length(viewPos - FragPos);
    float fog = clamp(exp(-0.25 * dist), 0.0, 1.0);                    //  √ÀÌ— »’—Ì: „⁄«œ·… «·÷»«» «·√”Ì…
    vec3 light = texColor.rgb * (1.5 / (1.0 + 0.1 * dist));            //  √ÀÌ— »’—Ì: ≈÷«¡… »”Ìÿ…  ⁄ „œ ⁄·Ï «·„”«›…
    FragColor = vec4(mix(vec3(0.05, 0.05, 0.05), light, fog), 1.0);   // œ„Ã «·÷»«» „⁄ «·≈÷«¡…
}
)glsl";

// «·‘Œ’Ì… Ê«·ﬂ«„Ì—«: „Ê«ﬁ⁄ «·»œ«Ì…
glm::vec3 playerPos = glm::vec3(1.5f, -0.2f, 5.0f), cameraPos = glm::vec3(1.5f, 1.0f, 7.0f);
// «·ﬂ«„Ì—«: “Ê«Ì« «·œÊ—«‰ (yaw ··œÊ—«‰ «·√›ﬁÌ° pitch ··⁄„ÊœÌ)
float yaw = -90.0f, pitch = 0.0f, lastX = 640, lastY = 360, deltaTime = 0.0f, lastFrame = 0.0f;
bool firstMouse = true;
//  ’„Ì„ «·„ «Â…: 0 = „„—° 1 = Ãœ«— (AABB ·· ’«œ„)
int maze[10][10] = { {1,1,1,1,1,1,1,1,1,1},{1,0,0,0,0,0,0,0,0,1},{1,0,1,1,1,0,1,1,0,1},{1,0,1,0,0,0,0,1,0,1},{1,0,0,0,0,0,0,0,0,1},{1,1,1,1,1,1,1,1,1,1} };

// „‰ÿﬁ «··⁄»…: ÂÌﬂ· «·⁄„·… „⁄ Õ«· Â« („Ã„Ê⁄… √„ ·«)
struct Coin { glm::vec3 pos; bool collected; };
std::vector<Coin> coins;
int score = 0;
bool gameWon = false;     // ≈œ«—… Õ«·«  «··⁄»…: Win/Lose
float startTime;

// «·ﬂ«„Ì—«: œ«·… «” Ã«»… «·„«Ê” ·Õ”«» yaw Ê pitch
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) { lastX = (float)xpos; lastY = (float)ypos; firstMouse = false; }
    yaw += (float)(xpos - lastX) * 0.15f;   // «·ﬂ«„Ì—«:  ÕœÌÀ «·œÊ—«‰ «·√›ﬁÌ
    pitch += (float)(lastY - ypos) * 0.15f; // «·ﬂ«„Ì—«:  ÕœÌÀ «·œÊ—«‰ «·⁄„ÊœÌ
    lastX = (float)xpos; lastY = (float)ypos;
    if (pitch > 45.0f) pitch = 45.0f; if (pitch < -30.0f) pitch = -30.0f; // „‰⁄ «‰ﬁ·«» «·„Õ«Ê—
}

// «· ’«œ„ AABB: ›Õ’  œ«Œ· ÕœÊœ «··«⁄» „⁄ «·Ãœ—«‰
bool checkCollision(glm::vec3 n) {
    for (int i = 0; i < 6; i++) for (int j = 0; j < 10; j++)
        if (maze[i][j] == 1 && n.x + 0.3f > i - 0.5f && n.x - 0.3f < i + 0.5f && n.z + 0.3f > j - 0.5f && n.z - 0.3f < j + 0.5f) return true;
    return false;
}

// œ«·… „”«⁄œ… ·—”„ √Ã“«¡ «·‘Œ’Ì… „⁄ „’›Ê›«  «· ÕÊÌ·
void drawPart(unsigned int shader, glm::mat4 base, glm::vec3 pos, glm::vec3 scale) {
    glm::mat4 m = glm::translate(base, pos);
    m = glm::scale(m, scale);
    glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, 0, glm::value_ptr(m));
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

int main() {
    //  ÂÌ∆… «·»Ì∆…: ≈⁄œ«œ GLFW Ê GLAD/GLEW
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Nazer Game", NULL, NULL);
    glfwMakeContextCurrent(window); glewInit(); glEnable(GL_DEPTH_TEST);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); glfwSetCursorPosCallback(window, mouse_callback);

    // „‰ÿﬁ «··⁄»…:  Ê“Ì⁄ 6 ⁄„·«  ›Ì «·„ «Â…
    coins.push_back({ glm::vec3(1.0f, -0.2f, 3.0f), false });
    coins.push_back({ glm::vec3(1.0f, -0.2f, 1.0f), false });
    coins.push_back({ glm::vec3(4.0f, -0.2f, 1.0f), false });
    coins.push_back({ glm::vec3(4.0f, -0.2f, 4.0f), false });
    coins.push_back({ glm::vec3(1.0f, -0.2f, 7.0f), false });
    coins.push_back({ glm::vec3(4.0f, -0.2f, 7.0f), false }); unsigned int v = glCreateShader(GL_VERTEX_SHADER); glShaderSource(v, 1, &vertexShaderSource, 0); glCompileShader(v);
    unsigned int f = glCreateShader(GL_FRAGMENT_SHADER); glShaderSource(f, 1, &fragmentShaderSource, 0); glCompileShader(f);
    unsigned int shader = glCreateProgram(); glAttachShader(shader, v); glAttachShader(shader, f); glLinkProgram(shader);
    // »‰«¡ «·„‘Âœ:  ⁄—Ì› «·—ƒÊ” („Êﬁ⁄ + ≈Õœ«ÀÌ«  ‰”ÌÃ) ··„ﬂ⁄»
    float vertices[] = { -0.5f,-0.5f,-0.5f,0,0, 0.5f,-0.5f,-0.5f,1,0, 0.5f,0.5f,-0.5f,1,1, 0.5f,0.5f,-0.5f,1,1, -0.5f,0.5f,-0.5f,0,1, -0.5f,-0.5f,-0.5f,0,0, -0.5f,-0.5f,0.5f,0,0, 0.5f,-0.5f,0.5f,1,0, 0.5f,0.5f,0.5f,1,1, 0.5f,0.5f,0.5f,1,1, -0.5f,0.5f,0.5f,0,1, -0.5f,-0.5f,0.5f,0,0, -0.5f,0.5f,0.5f,1,0, -0.5f,0.5f,-0.5f,1,1, -0.5f,-0.5f,-0.5f,0,1, -0.5f,-0.5f,-0.5f,0,1, -0.5f,-0.5f,0.5f,0,0, -0.5f,0.5f,0.5f,1,0, 0.5f,0.5f,0.5f,1,0, 0.5f,0.5f,-0.5f,1,1, 0.5f,-0.5f,-0.5f,0,1, 0.5f,-0.5f,-0.5f,0,1, 0.5f,-0.5f,0.5f,0,0, 0.5f,0.5f,0.5f,1,0, -0.5f,-0.5f,-0.5f,0,1, 0.5f,-0.5f,-0.5f,1,1, 0.5f,-0.5f,0.5f,1,0, 0.5f,-0.5f,0.5f,1,0, -0.5f,-0.5f,0.5f,0,0, -0.5f,-0.5f,-0.5f,0,1, -0.5f,0.5f,-0.5f,0,1, 0.5f,0.5f,-0.5f,1,1, 0.5f,0.5f,0.5f,1,0, 0.5f,0.5f,0.5f,1,0, -0.5f,0.5f,0.5f,0,0, -0.5f,0.5f,-0.5f,0,1 };
    // »‰«¡ «·„‘Âœ: ≈⁄œ«œ VAO Ê VBO (Œÿ √‰«»Ì» «·—”„ «·ÕœÌÀ)
    unsigned int VBO, VAO; glGenVertexArrays(1, &VAO); glGenBuffers(1, &VBO); glBindVertexArray(VAO); glBindBuffer(GL_ARRAY_BUFFER, VBO); glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, 0, 5 * 4, 0); glEnableVertexAttribArray(0);  // «·„Êﬁ⁄
    glVertexAttribPointer(1, 2, GL_FLOAT, 0, 5 * 4, (void*)12); glEnableVertexAttribArray(1); // «·≈Õœ«ÀÌ«  «·‰”ÌÃÌ…

    // «·Œ«„« :  Õ„Ì· ‰”ÌÃ «·Ãœ—«‰ Ê«·√—÷Ì…
    unsigned int t1, t2; int w, h, n; stbi_set_flip_vertically_on_load(1);
    glGenTextures(1, &t1); glBindTexture(GL_TEXTURE_2D, t1);
    unsigned char* d1 = stbi_load("assets/Wall.jpg", &w, &h, &n, 0); if (d1) { glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, d1); glGenerateMipmap(GL_TEXTURE_2D); stbi_image_free(d1); }
    glGenTextures(1, &t2); glBindTexture(GL_TEXTURE_2D, t2);
    unsigned char* d2 = stbi_load("assets/ground.jpg", &w, &h, &n, 0); if (d2) { glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, d2); glGenerateMipmap(GL_TEXTURE_2D); stbi_image_free(d2); }

    startTime = (float)glfwGetTime();

    // «·Õ·ﬁ… «·√”«”Ì…: Game Loop - ›’·  ÕœÌÀ «·„‰ÿﬁ ⁄‰ «·—”„
    while (!glfwWindowShouldClose(window)) {
        float cf = (float)glfwGetTime(); deltaTime = cf - lastFrame; lastFrame = cf; // Õ”«» «·“„‰ »Ì‰ «·≈ÿ«—« 

        // HUD Ê«·Ê«Ã»« : ⁄—÷ «·‰ﬁ«ÿ Ê«·Êﬁ  ›Ì ⁄‰Ê«‰ «·‰«›–…
        if (!gameWon) {
            float currentTime = cf - startTime;
            std::string title = "Score: " + std::to_string(score) + " / 4 | Time: " + std::to_string((int)currentTime) + "s";
            glfwSetWindowTitle(window, title.c_str());
        }
        else {
            glfwSetWindowTitle(window, "YOU WIN! Collected 4 Coins!"); // Õ«·… «·›Ê“
        }

        // «·‘Œ’Ì… Ê«· Õﬂ„: ﬁ—«¡… «·„œŒ·«  ÊÕ”«» «·Õ—ﬂ…
        bool isMoving = false; glm::vec3 nP = playerPos; float s = 3.5f * deltaTime;
        glm::vec3 fwd = glm::normalize(glm::vec3(cos(glm::radians(yaw)), 0, sin(glm::radians(yaw)))), rgt = glm::normalize(glm::cross(fwd, { 0,1,0 })); // „ ÃÂ«  «·Õ—ﬂ… «·‰”»Ì…

        if (!gameWon) {
            if (glfwGetKey(window, GLFW_KEY_W)) { nP += s * fwd; isMoving = true; }   // «· Õﬂ„:  ﬁœ„if (glfwGetKey(window, GLFW_KEY_S)) { nP -= s * fwd; isMoving = true; }   // «· Õﬂ„: —ÃÊ⁄
            if (glfwGetKey(window, GLFW_KEY_A)) { nP -= s * rgt; isMoving = true; }   // «· Õﬂ„: Ì”«—
            if (glfwGetKey(window, GLFW_KEY_D)) { nP += s * rgt; isMoving = true; }   // «· Õﬂ„: Ì„Ì‰
            if (!checkCollision(nP)) playerPos = nP; // «· ’«œ„: «· Õﬁﬁ ﬁ»·  ÿ»Ìﬁ «·Õ—ﬂ…
        }

        // „‰ÿﬁ «··⁄»…: ¬·Ì… ﬂ‘› Ã„⁄ «·⁄„·« 
        for (auto& coin : coins) {
            if (!coin.collected && glm::distance(playerPos, coin.pos) < 0.6f) {
                coin.collected = true;
                score++;
                if (score >= 4) gameWon = true; // ‘—ÿ «·›Ê“: Ã„⁄ 4 ⁄„·« 
            }
        }

        // «·ﬂ«„Ì—«: Õ”«» „ ÃÂ«  Forward/Right/Up · ÕœÌœ „’›Ê›… View
        glm::vec3 cDir(cos(glm::radians(yaw)) * cos(glm::radians(pitch)), sin(glm::radians(pitch)), sin(glm::radians(yaw)) * cos(glm::radians(pitch)));
        cameraPos = glm::mix(cameraPos, playerPos - fwd * 2.0f + glm::vec3(0, 1.2f, 0), 8.0f * deltaTime); // ﬂ«„Ì—« ”·”…   »⁄ «··«⁄»
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shader);
        glUniform3fv(glGetUniformLocation(shader, "viewPos"), 1, glm::value_ptr(cameraPos));
        glUniform1i(glGetUniformLocation(shader, "isMoving"), isMoving);
        // «·ﬂ«„Ì—«: „’›Ê›… «·≈”ﬁ«ÿ «·„‰ŸÊ—Ì
        glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, 0, glm::value_ptr(glm::perspective(glm::radians(45.0f), 1280.f / 720.f, 0.1f, 100.f)));
        // «·ﬂ«„Ì—«: „’›Ê›… View »«” Œœ«„ lookAt
        glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, 0, glm::value_ptr(glm::lookAt(cameraPos, playerPos + cDir * 5.0f, { 0,1,0 })));

        // «·‘Œ’Ì…: —”„ ‰„Ê–Ã À·«ÀÌ «·√»⁄«œ „‰ „ﬂ⁄»«  „ ⁄œœ…
        glBindVertexArray(VAO); glUniform1i(glGetUniformLocation(shader, "isPlayer"), 1);
        glUniform1i(glGetUniformLocation(shader, "isCoin"), 0);
        glm::mat4 m = glm::rotate(glm::translate(glm::mat4(1.0f), playerPos), glm::radians(-yaw - 90), { 0,1,0 });
        drawPart(shader, m, { 0, 0, 0 }, { 0.4f, 0.6f, 0.2f });       // Ã”„ «·‘Œ’Ì…
        drawPart(shader, m, { 0, 0.45f, 0 }, { 0.2f, 0.2f, 0.2f });   // —√”
        drawPart(shader, m, { -0.3f, 0, 0 }, { 0.15f, 0.5f, 0.15f }); // –—«⁄ Ì”«—
        drawPart(shader, m, { 0.3f, 0, 0 }, { 0.15f, 0.5f, 0.15f });  // –—«⁄ Ì„Ì‰
        drawPart(shader, m, { -0.15f, -0.5f, 0 }, { 0.15f, 0.5f, 0.15f }); // —Ã· Ì”«—
        drawPart(shader, m, { 0.15f, -0.5f, 0 }, { 0.15f, 0.5f, 0.15f });  // —Ã· Ì„Ì‰

        // «·⁄„·« : —”„ «·⁄„·«  €Ì— «·„Ã„Ê⁄… ›ﬁÿ
        glUniform1i(glGetUniformLocation(shader, "isPlayer"), 0);
        glUniform1i(glGetUniformLocation(shader, "isCoin"), 1);
        for (const auto& coin : coins) {
            if (!coin.collected) {
                glm::mat4 model = glm::translate(glm::mat4(1.0f), coin.pos);
                model = glm::scale(model, glm::vec3(0.4f));
                glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, 0, glm::value_ptr(model));
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
        }

        // «·Ãœ—«‰: —”„ Œ·«Ì« «·„ «Â…
        glUniform1i(glGetUniformLocation(shader, "isCoin"), 0);
        glBindTexture(GL_TEXTURE_2D, t1);
        for (int i = 0; i < 6; i++) for (int j = 0; j < 10; j++) if (maze[i][j]) {
            glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, 0, glm::value_ptr(glm::translate(glm::mat4(1.0f), glm::vec3(i, 0, j)))); glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        // «·√—÷Ì…: —”„  Õ  ﬂ· Œ·Ì…
        glBindTexture(GL_TEXTURE_2D, t2);
        for (int i = 0; i < 6; i++) for (int j = 0; j < 10; j++) {
            glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, 0, glm::value_ptr(glm::translate(glm::mat4(1.0f), glm::vec3(i, -0.6f, j)))); glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glfwSwapBuffers(window); glfwPollEvents();
    }
    glfwTerminate(); return 0;
}