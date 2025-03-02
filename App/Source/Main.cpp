// MANDELBROT
#if 1
#include <iostream>
#include <ctime>
#include <glm/glm.hpp>
#include <filesystem>
#include "Shader.h"
#include "Renderer.h"
#include <vector>
#include <string>

#include <fstream>
#include <sstream>
#include <string>
#include <GL/gl.h> // or your GL loader header (e.g., glad.h)

// Helper function to load a shader file's contents into a std::string.
std::string LoadShaderSource(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << path << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Helper function to compile a shader of a given type (GL_VERTEX_SHADER or GL_FRAGMENT_SHADER).
GLuint CompileShader(GLenum shaderType, const char* source) {
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    // Check for compile errors.
    GLint isCompiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE) {
        GLint maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character.
        std::string errorLog(maxLength, ' ');
        glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);

        std::cerr << "Shader compilation error: " << errorLog << std::endl;

        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

// Helper function to create a shader program from vertex and fragment shader sources.
GLuint CreateShaderProgram(const char* vertexShaderSource, const char* fragmentShaderSource) {
    GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
    if (!vertexShader) return 0;
    
    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    if (!fragmentShader) {
        glDeleteShader(vertexShader);
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program);

    // Check for link errors.
    GLint isLinked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE) {
        GLint maxLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

        std::string infoLog(maxLength, ' ');
        glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

        std::cerr << "Shader program linking error: " << infoLog << std::endl;

        // Cleanup shaders.
        glDeleteProgram(program);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return 0;
    }

    // Shaders can be detached and deleted after successful linking.
    glDetachShader(program, vertexShader);
    glDetachShader(program, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

// Helper functions to load shader source from file
std::string LoadShaderSource(const std::filesystem::path& path);

static uint32_t s_ShaderProgram = 0;

// https://www.shadertoy.com/view/lsX3W4
//static const std::filesystem::path s_FragmentShaderPath = "C:/Users/jonas/Code2/C++/GPUCompute/App/Shaders/Compute6.glsl";
// https://www.shadertoy.com/view/MltXz2
//static const std::filesystem::path s_FragmentShaderPath = "C:/Users/jonas/Code2/C++/GPUCompute/App/Shaders/Compute7.glsl";
// https://www.shadertoy.com/view/4df3Rn
//static const std::filesystem::path s_FragmentShaderPath = "C:/Users/jonas/Code2/C++/GPUCompute/App/Shaders/Compute8.glsl";
// https://www.shadertoy.com/view/ldf3DN
//static const std::filesystem::path s_FragmentShaderPath = "C:/Users/jonas/Code2/C++/GPUCompute/App/Shaders/Compute9.glsl";
// Gravity
static const std::filesystem::path s_FragmentShaderPath = "C:/Users/jonas/Code2/C++/GPUCompute/App/Shaders/Compute10.glsl";

static const std::filesystem::path s_VertexShaderPath   = "C:/Users/jonas/Code2/C++/GPUCompute/App/Shaders/fullscreen_triangle.vert";

static void ErrorCallback(int error, const char* description)
{
    std::cerr << "Error: " << description << std::endl;
}

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        // Implement shader reload if desired.
    }
}

int main()
{
    glfwSetErrorCallback(ErrorCallback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

    int width = 1280;
    int height = 720;

    GLFWwindow* window = glfwCreateWindow(width, height, "Fragment Shader", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, KeyCallback);
    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1);

    // Load shader sources
    std::string vertexSource = LoadShaderSource(s_VertexShaderPath);
    std::string fragmentSource = LoadShaderSource(s_FragmentShaderPath);
    if (vertexSource.empty() || fragmentSource.empty())
    {
        std::cerr << "Failed to load shader sources." << std::endl;
        return -1;
    }

    // Create shader program (assume CreateShaderProgram compiles and links both shaders)
    s_ShaderProgram = CreateShaderProgram(vertexSource.c_str(), fragmentSource.c_str());
    if (s_ShaderProgram == 0)
    {
        std::cerr << "Shader program creation failed" << std::endl;
        return -1;
    }

    // Create a fullscreen VAO (no VBO needed because the vertex shader uses gl_VertexID)
    GLuint vao;
    glGenVertexArrays(1, &vao);

    // For timing and frame counting
    float previousTime = static_cast<float>(glfwGetTime());
    int frameCounter = 0;

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        
        // Update uniforms
        glUseProgram(s_ShaderProgram);
        
        // Set iResolution uniform
        GLint loc = glGetUniformLocation(s_ShaderProgram, "iResolution");
        if (loc != -1)
            glUniform3f(loc, (float)width, (float)height, 1.0f);

        // Update iTime uniform
        float currentTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentTime - previousTime;
        previousTime = currentTime;
        frameCounter++;
        loc = glGetUniformLocation(s_ShaderProgram, "iTime");
        if (loc != -1)
            glUniform1f(loc, currentTime);

        // Set other uniforms as needed (iMouse, PASS3, PASS4, MAPRES, etc.)
        // For example, setting iMouse to zero:
        loc = glGetUniformLocation(s_ShaderProgram, "iMouse");
        if (loc != -1)
            glUniform4f(loc, 0.0f, 0.0f, 0.0f, 0.0f);
        // You can set PASS3, PASS4, MAPRES similarly.

        // Render to the default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw a fullscreen triangle
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}




// DIABLO
//#if 1
//#include <iostream>
//#include <ctime>
//#include <glm/glm.hpp>
//#include <filesystem>
//#include "Shader.h"      // Assumed to contain shader loading/compiling helpers
//#include "Renderer.h"    // Assumed to contain texture/framebuffer helper functions
//#include <vector>
//#include <string>
//
//#include <fstream>
//#include <sstream>
//#include <string>
//#include <GL/gl.h> // or your GL loader header (e.g., glad.h)
//
//// Helper function to load a shader file's contents into a std::string.
//std::string LoadShaderSource(const std::filesystem::path& path) {
//    std::ifstream file(path);
//    if (!file.is_open()) {
//        std::cerr << "Failed to open shader file: " << path << std::endl;
//        return "";
//    }
//    std::stringstream buffer;
//    buffer << file.rdbuf();
//    return buffer.str();
//}
//
//// Helper function to compile a shader of a given type (GL_VERTEX_SHADER or GL_FRAGMENT_SHADER).
//GLuint CompileShader(GLenum shaderType, const char* source) {
//    GLuint shader = glCreateShader(shaderType);
//    glShaderSource(shader, 1, &source, nullptr);
//    glCompileShader(shader);
//
//    // Check for compile errors.
//    GLint isCompiled = 0;
//    glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
//    if (isCompiled == GL_FALSE) {
//        GLint maxLength = 0;
//        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
//
//        // The maxLength includes the NULL character.
//        std::string errorLog(maxLength, ' ');
//        glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);
//
//        std::cerr << "Shader compilation error: " << errorLog << std::endl;
//
//        glDeleteShader(shader);
//        return 0;
//    }
//
//    return shader;
//}
//
//// Helper function to create a shader program from vertex and fragment shader sources.
//GLuint CreateShaderProgram(const char* vertexShaderSource, const char* fragmentShaderSource) {
//    GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
//    if (!vertexShader) return 0;
//    
//    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
//    if (!fragmentShader) {
//        glDeleteShader(vertexShader);
//        return 0;
//    }
//
//    GLuint program = glCreateProgram();
//    glAttachShader(program, vertexShader);
//    glAttachShader(program, fragmentShader);
//
//    glLinkProgram(program);
//
//    // Check for link errors.
//    GLint isLinked = 0;
//    glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
//    if (isLinked == GL_FALSE) {
//        GLint maxLength = 0;
//        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
//
//        std::string infoLog(maxLength, ' ');
//        glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);
//
//        std::cerr << "Shader program linking error: " << infoLog << std::endl;
//
//        // Cleanup shaders.
//        glDeleteProgram(program);
//        glDeleteShader(vertexShader);
//        glDeleteShader(fragmentShader);
//
//        return 0;
//    }
//
//    // Shaders can be detached and deleted after successful linking.
//    glDetachShader(program, vertexShader);
//    glDetachShader(program, fragmentShader);
//    glDeleteShader(vertexShader);
//    glDeleteShader(fragmentShader);
//
//    return program;
//}
//
//// Helper functions to load shader source from file
//std::string LoadShaderSource(const std::filesystem::path& path);
//
//static uint32_t s_ShaderProgram = 0;
//// https://www.shadertoy.com/view/llcSRf (DIABLO ORBS, not perfect)
//static const std::filesystem::path s_FragmentShaderPath = "C:/Users/jonas/Code2/C++/GPUCompute/App/Shaders/Compute5.glsl";
//static const std::filesystem::path s_VertexShaderPath   = "C:/Users/jonas/Code2/C++/GPUCompute/App/Shaders/fullscreen_triangle.vert";
//
//static void ErrorCallback(int error, const char* description)
//{
//    std::cerr << "Error: " << description << std::endl;
//}
//
//static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
//{
//    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
//        glfwSetWindowShouldClose(window, GLFW_TRUE);
//
//    if (key == GLFW_KEY_R && action == GLFW_PRESS)
//    {
//        // Implement shader reload if desired.
//    }
//}
//
//// Helper function to create a 1x1 texture with a given color.
//GLuint createSolidColorTexture(GLubyte r, GLubyte g, GLubyte b, GLubyte a) {
//    GLuint tex;
//    glGenTextures(1, &tex);
//    glBindTexture(GL_TEXTURE_2D, tex);
//    // A single pixel of color data.
//    GLubyte data[4] = { r, g, b, a };
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    return tex;
//}
//
//int main()
//{
//    glfwSetErrorCallback(ErrorCallback);
//
//    if (!glfwInit())
//        exit(EXIT_FAILURE);
//
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
//
//    int width = 1280;
//    int height = 720;
//
//    GLFWwindow* window = glfwCreateWindow(width, height, "Fragment Shader", nullptr, nullptr);
//    if (!window)
//    {
//        glfwTerminate();
//        exit(EXIT_FAILURE);
//    }
//
//    glfwSetKeyCallback(window, KeyCallback);
//    glfwMakeContextCurrent(window);
//    gladLoadGL(glfwGetProcAddress);
//    glfwSwapInterval(1);
//
//    // Load shader sources
//    std::string vertexSource = LoadShaderSource(s_VertexShaderPath);
//    std::string fragmentSource = LoadShaderSource(s_FragmentShaderPath);
//    if (vertexSource.empty() || fragmentSource.empty())
//    {
//        std::cerr << "Failed to load shader sources." << std::endl;
//        return -1;
//    }
//
//    // Create shader program (assume CreateShaderProgram compiles and links both shaders)
//    s_ShaderProgram = CreateShaderProgram(vertexSource.c_str(), fragmentSource.c_str());
//    if (s_ShaderProgram == 0)
//    {
//        std::cerr << "Shader program creation failed" << std::endl;
//        return -1;
//    }
//
//    // Create a fullscreen VAO (no VBO needed because the vertex shader uses gl_VertexID)
//    GLuint vao;
//    glGenVertexArrays(1, &vao);
//
//    // For timing and frame counting
//    float previousTime = static_cast<float>(glfwGetTime());
//    int frameCounter = 0;
//
//	GLuint tex0 = createSolidColorTexture(255, 0, 0, 255);   // iChannel0: red
//	GLuint tex1 = createSolidColorTexture(0, 255, 0, 255);   // iChannel1: green
//	GLuint tex2 = createSolidColorTexture(0, 0, 255, 255);   // iChannel2: blue
//	GLuint tex3 = createSolidColorTexture(255, 255, 255, 255); // iChannel3: white
//
//    // Main loop
//    while (!glfwWindowShouldClose(window))
//    {
//        glfwGetFramebufferSize(window, &width, &height);
//        glViewport(0, 0, width, height);
//        
//        // Update uniforms
//        glUseProgram(s_ShaderProgram);
//        
//        // Set iResolution uniform
//        GLint loc = glGetUniformLocation(s_ShaderProgram, "iResolution");
//        if (loc != -1)
//            glUniform3f(loc, (float)width, (float)height, 1.0f);
//
//        // Update iTime uniform
//        float currentTime = static_cast<float>(glfwGetTime());
//        float deltaTime = currentTime - previousTime;
//        previousTime = currentTime;
//        frameCounter++;
//        loc = glGetUniformLocation(s_ShaderProgram, "iTime");
//        if (loc != -1)
//            glUniform1f(loc, currentTime);
//
//		// Bind tex0 to texture unit 0
//		glActiveTexture(GL_TEXTURE0);
//		glBindTexture(GL_TEXTURE_2D, tex0);
//		glUniform1i(glGetUniformLocation(s_ShaderProgram, "iChannel0"), 0);
//
//		// Bind tex1 to texture unit 1
//		glActiveTexture(GL_TEXTURE1);
//		glBindTexture(GL_TEXTURE_2D, tex1);
//		glUniform1i(glGetUniformLocation(s_ShaderProgram, "iChannel1"), 1);
//
//		// Bind tex2 to texture unit 2
//		glActiveTexture(GL_TEXTURE2);
//		glBindTexture(GL_TEXTURE_2D, tex2);
//		glUniform1i(glGetUniformLocation(s_ShaderProgram, "iChannel2"), 2);
//
//		// Bind tex3 to texture unit 3
//		glActiveTexture(GL_TEXTURE3);
//		glBindTexture(GL_TEXTURE_2D, tex3);
//		glUniform1i(glGetUniformLocation(s_ShaderProgram, "iChannel3"), 3);
//
//        // Render to the default framebuffer
//        glBindFramebuffer(GL_FRAMEBUFFER, 0);
//        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
//        glClear(GL_COLOR_BUFFER_BIT);
//
//        // Draw a fullscreen triangle
//        glBindVertexArray(vao);
//        glDrawArrays(GL_TRIANGLES, 0, 3);
//
//        glfwSwapBuffers(window);
//        glfwPollEvents();
//    }
//
//    glDeleteVertexArrays(1, &vao);
//    glfwDestroyWindow(window);
//    glfwTerminate();
//    return 0;
//}






//#if 1
//#include <iostream>
//#include <ctime>
//#include <glm/glm.hpp>
//#include <filesystem>
//#include "Shader.h"      // Assumed to contain shader loading/compiling helpers
//#include "Renderer.h"    // Assumed to contain texture/framebuffer helper functions
//#include <vector>
//#include <string>
//
//#include <fstream>
//#include <sstream>
//#include <string>
//#include <GL/gl.h> // or your GL loader header (e.g., glad.h)
//
//// Helper function to load a shader file's contents into a std::string.
//std::string LoadShaderSource(const std::filesystem::path& path) {
//    std::ifstream file(path);
//    if (!file.is_open()) {
//        std::cerr << "Failed to open shader file: " << path << std::endl;
//        return "";
//    }
//    std::stringstream buffer;
//    buffer << file.rdbuf();
//    return buffer.str();
//}
//
//// Helper function to compile a shader of a given type (GL_VERTEX_SHADER or GL_FRAGMENT_SHADER).
//GLuint CompileShader(GLenum shaderType, const char* source) {
//    GLuint shader = glCreateShader(shaderType);
//    glShaderSource(shader, 1, &source, nullptr);
//    glCompileShader(shader);
//
//    // Check for compile errors.
//    GLint isCompiled = 0;
//    glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
//    if (isCompiled == GL_FALSE) {
//        GLint maxLength = 0;
//        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
//
//        // The maxLength includes the NULL character.
//        std::string errorLog(maxLength, ' ');
//        glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);
//
//        std::cerr << "Shader compilation error: " << errorLog << std::endl;
//
//        glDeleteShader(shader);
//        return 0;
//    }
//
//    return shader;
//}
//
//// Helper function to create a shader program from vertex and fragment shader sources.
//GLuint CreateShaderProgram(const char* vertexShaderSource, const char* fragmentShaderSource) {
//    GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
//    if (!vertexShader) return 0;
//    
//    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
//    if (!fragmentShader) {
//        glDeleteShader(vertexShader);
//        return 0;
//    }
//
//    GLuint program = glCreateProgram();
//    glAttachShader(program, vertexShader);
//    glAttachShader(program, fragmentShader);
//
//    glLinkProgram(program);
//
//    // Check for link errors.
//    GLint isLinked = 0;
//    glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
//    if (isLinked == GL_FALSE) {
//        GLint maxLength = 0;
//        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
//
//        std::string infoLog(maxLength, ' ');
//        glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);
//
//        std::cerr << "Shader program linking error: " << infoLog << std::endl;
//
//        // Cleanup shaders.
//        glDeleteProgram(program);
//        glDeleteShader(vertexShader);
//        glDeleteShader(fragmentShader);
//
//        return 0;
//    }
//
//    // Shaders can be detached and deleted after successful linking.
//    glDetachShader(program, vertexShader);
//    glDetachShader(program, fragmentShader);
//    glDeleteShader(vertexShader);
//    glDeleteShader(fragmentShader);
//
//    return program;
//}
//
//// Helper functions to load shader source from file
//std::string LoadShaderSource(const std::filesystem::path& path);
//
//static uint32_t s_ShaderProgram = 0;
//
//// https://www.shadertoy.com/view/XsXXDn
//static const std::filesystem::path s_FragmentShaderPath = "C:/Users/jonas/Code2/C++/GPUCompute/App/Shaders/Compute2.glsl";
//// https://www.shadertoy.com/view/XslGRr (not perfect)
////static const std::filesystem::path s_FragmentShaderPath = "C:/Users/jonas/Code2/C++/GPUCompute/App/Shaders/Compute3.glsl";
//// https://www.shadertoy.com/view/ldKGDh (no animation...)
////static const std::filesystem::path s_FragmentShaderPath = "C:/Users/jonas/Code2/C++/GPUCompute/App/Shaders/Compute4.glsl";
//
//static const std::filesystem::path s_VertexShaderPath   = "C:/Users/jonas/Code2/C++/GPUCompute/App/Shaders/fullscreen_triangle.vert";
//
//static void ErrorCallback(int error, const char* description)
//{
//    std::cerr << "Error: " << description << std::endl;
//}
//
//static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
//{
//    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
//        glfwSetWindowShouldClose(window, GLFW_TRUE);
//
//    if (key == GLFW_KEY_R && action == GLFW_PRESS)
//    {
//        // Implement shader reload if desired.
//    }
//}
//
//int main()
//{
//    glfwSetErrorCallback(ErrorCallback);
//
//    if (!glfwInit())
//        exit(EXIT_FAILURE);
//
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
//
//    int width = 1280;
//    int height = 720;
//
//    GLFWwindow* window = glfwCreateWindow(width, height, "Fragment Shader", nullptr, nullptr);
//    if (!window)
//    {
//        glfwTerminate();
//        exit(EXIT_FAILURE);
//    }
//
//    glfwSetKeyCallback(window, KeyCallback);
//    glfwMakeContextCurrent(window);
//    gladLoadGL(glfwGetProcAddress);
//    glfwSwapInterval(1);
//
//    // Load shader sources
//    std::string vertexSource = LoadShaderSource(s_VertexShaderPath);
//    std::string fragmentSource = LoadShaderSource(s_FragmentShaderPath);
//    if (vertexSource.empty() || fragmentSource.empty())
//    {
//        std::cerr << "Failed to load shader sources." << std::endl;
//        return -1;
//    }
//
//    // Create shader program (assume CreateShaderProgram compiles and links both shaders)
//    s_ShaderProgram = CreateShaderProgram(vertexSource.c_str(), fragmentSource.c_str());
//    if (s_ShaderProgram == 0)
//    {
//        std::cerr << "Shader program creation failed" << std::endl;
//        return -1;
//    }
//
//    // Create a fullscreen VAO (no VBO needed because the vertex shader uses gl_VertexID)
//    GLuint vao;
//    glGenVertexArrays(1, &vao);
//
//    // For timing and frame counting
//    float previousTime = static_cast<float>(glfwGetTime());
//    int frameCounter = 0;
//
//    // Main loop
//    while (!glfwWindowShouldClose(window))
//    {
//        glfwGetFramebufferSize(window, &width, &height);
//        glViewport(0, 0, width, height);
//        
//        // Update uniforms
//        glUseProgram(s_ShaderProgram);
//        
//        // Set iResolution uniform
//        GLint loc = glGetUniformLocation(s_ShaderProgram, "iResolution");
//        if (loc != -1)
//            glUniform3f(loc, (float)width, (float)height, 1.0f);
//
//        // Update iTime uniform
//        float currentTime = static_cast<float>(glfwGetTime());
//        float deltaTime = currentTime - previousTime;
//        previousTime = currentTime;
//        frameCounter++;
//        loc = glGetUniformLocation(s_ShaderProgram, "iTime");
//        if (loc != -1)
//            glUniform1f(loc, currentTime);
//
//        // Set other uniforms as needed (iMouse, PASS3, PASS4, MAPRES, etc.)
//        // For example, setting iMouse to zero:
//        loc = glGetUniformLocation(s_ShaderProgram, "iMouse");
//        if (loc != -1)
//            glUniform4f(loc, 0.0f, 0.0f, 0.0f, 0.0f);
//        // You can set PASS3, PASS4, MAPRES similarly.
//
//        // Render to the default framebuffer
//        glBindFramebuffer(GL_FRAMEBUFFER, 0);
//        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
//        glClear(GL_COLOR_BUFFER_BIT);
//
//        // Draw a fullscreen triangle
//        glBindVertexArray(vao);
//        glDrawArrays(GL_TRIANGLES, 0, 3);
//
//        glfwSwapBuffers(window);
//        glfwPollEvents();
//    }
//
//    glDeleteVertexArrays(1, &vao);
//    glfwDestroyWindow(window);
//    glfwTerminate();
//    return 0;
//}



//#if 1
//#include <iostream>
//#include <ctime>
//#include <glm/glm.hpp>
//#include <filesystem>
//#include "Shader.h"
//#include "Renderer.h"
//#include <glm/gtc/type_precision.hpp>
//
//static uint32_t s_ComputeShader = -1;
//// https://www.shadertoy.com/view/tsXBzS
//static const std::filesystem::path s_ComputeShaderPath = "C:/Users/jonas/Code2/C++/GPUCompute/App/Shaders/Compute.glsl";
//
//static void ErrorCallback(int error, const char* description)
//{
//    std::cerr << "Error: " << description << std::endl;
//}
//
//static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
//{
//    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
//        glfwSetWindowShouldClose(window, GLFW_TRUE);
//
//    if (key == GLFW_KEY_R)
//        s_ComputeShader = ReloadComputeShader(s_ComputeShader, s_ComputeShaderPath);
//}
//
//int main()
//{
//    glfwSetErrorCallback(ErrorCallback);
//
//    if (!glfwInit())
//        exit(EXIT_FAILURE);
//
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
//
//    int width = 1280;
//    int height = 720;
//
//    GLFWwindow* window = glfwCreateWindow(width, height, "Compute", NULL, NULL);
//    if (!window)
//    {
//        glfwTerminate();
//        exit(EXIT_FAILURE);
//    }
//
//    glfwSetKeyCallback(window, KeyCallback);
//    glfwMakeContextCurrent(window);
//    gladLoadGL(glfwGetProcAddress);
//    glfwSwapInterval(1);
//
//    s_ComputeShader = CreateComputeShader(s_ComputeShaderPath);
//    // Fragment shader
//    //s_ComputeShader = glCreateShader(GL_FRAGMENT_SHADER);
//
//    if (s_ComputeShader == -1)
//    {
//        std::cerr << "Compute shader failed\n";
//        return -1;
//    }
//
//    Texture computeShaderTexture = CreateTexture(width, height);
//    Framebuffer fb = CreateFramebufferWithTexture(computeShaderTexture);
//
//    // For timing and frame counting
//    float previousTime = (float)glfwGetTime();
//    int frameCounter = 0;
//
//    while (!glfwWindowShouldClose(window))
//    {
//        glfwGetFramebufferSize(window, &width, &height);
//
//        // Resize texture if necessary
//        if (width != computeShaderTexture.Width || height != computeShaderTexture.Height)
//        {
//            glDeleteTextures(1, &computeShaderTexture.Handle);
//            computeShaderTexture = CreateTexture(width, height);
//            AttachTextureToFramebuffer(fb, computeShaderTexture);
//        }
//
//        // Update uniforms before compute dispatch
//        glUseProgram(s_ComputeShader);
//
//        // Update iResolution uniform (viewport resolution)
//        GLint loc_iResolution = glGetUniformLocation(s_ComputeShader, "iResolution");
//        if (loc_iResolution != -1)
//            glUniform3f(loc_iResolution, (float)width, (float)height, 1.0f);
//
//        // Compute current time and delta time
//        float currentTime = (float)glfwGetTime();
//        float deltaTime = currentTime - previousTime;
//        previousTime = currentTime;
//        frameCounter++;
//
//        // Update iTime (time in seconds)
//        GLint loc_iTime = glGetUniformLocation(s_ComputeShader, "iTime");
//        if (loc_iTime != -1)
//            glUniform1f(loc_iTime, currentTime);
//
//        // Update iTimeDelta (time between frames)
//        GLint loc_iTimeDelta = glGetUniformLocation(s_ComputeShader, "iTimeDelta");
//        if (loc_iTimeDelta != -1)
//            glUniform1f(loc_iTimeDelta, deltaTime);
//
//        // Update iFrameRate (frames per second)
//        GLint loc_iFrameRate = glGetUniformLocation(s_ComputeShader, "iFrameRate");
//        if (loc_iFrameRate != -1)
//            glUniform1f(loc_iFrameRate, (deltaTime > 0.0f ? 1.0f / deltaTime : 0.0f));
//
//        // Update iFrame (current frame count)
//        GLint loc_iFrame = glGetUniformLocation(s_ComputeShader, "iFrame");
//        if (loc_iFrame != -1)
//            glUniform1i(loc_iFrame, frameCounter);
//
//        // Update iChannelTime[4] (set all channels to currentTime as an example)
//        GLint loc_iChannelTime = glGetUniformLocation(s_ComputeShader, "iChannelTime");
//        if (loc_iChannelTime != -1)
//        {
//            float channelTime[4] = { currentTime, currentTime, currentTime, currentTime };
//            glUniform1fv(loc_iChannelTime, 4, channelTime);
//        }
//
//        // Update iChannelResolution[4] (using viewport size for all channels)
//        GLint loc_iChannelResolution = glGetUniformLocation(s_ComputeShader, "iChannelResolution");
//        if (loc_iChannelResolution != -1)
//        {
//            // Each channel is a vec3, so we need 12 floats (4 channels * 3 components)
//            float channelRes[12] = {
//                (float)width, (float)height, 1.0f,
//                (float)width, (float)height, 1.0f,
//                (float)width, (float)height, 1.0f,
//                (float)width, (float)height, 1.0f
//            };
//            glUniform3fv(loc_iChannelResolution, 4, channelRes);
//        }
//
//        // Update iMouse (for now set to zero; you can update this from a mouse callback)
//        GLint loc_iMouse = glGetUniformLocation(s_ComputeShader, "iMouse");
//        if (loc_iMouse != -1)
//            glUniform4f(loc_iMouse, 0.0f, 0.0f, 0.0f, 0.0f);
//
//        // Update iDate (year, month, day, seconds since midnight)
//        std::time_t now = std::time(nullptr);
//        std::tm* localTime = std::localtime(&now);
//        int year = localTime->tm_year + 1900;
//        int month = localTime->tm_mon + 1;
//        int day = localTime->tm_mday;
//        int secondsSinceMidnight = localTime->tm_hour * 3600 + localTime->tm_min * 60 + localTime->tm_sec;
//        GLint loc_iDate = glGetUniformLocation(s_ComputeShader, "iDate");
//        if (loc_iDate != -1)
//            glUniform4f(loc_iDate, (float)year, (float)month, (float)day, (float)secondsSinceMidnight);
//
//        // Update iSampleRate (for example, 44100)
//        GLint loc_iSampleRate = glGetUniformLocation(s_ComputeShader, "iSampleRate");
//        if (loc_iSampleRate != -1)
//            glUniform1f(loc_iSampleRate, 44100.0f);
//
//        // If you have textures for iChannel0..iChannel3, bind them and update the sampler uniforms here
//
//        // Compute pass
//        glBindImageTexture(0, fb.ColorAttachment.Handle, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
//
//        const GLuint workGroupSizeX = 16;
//        const GLuint workGroupSizeY = 16;
//        GLuint numGroupsX = (width + workGroupSizeX - 1) / workGroupSizeX;
//        GLuint numGroupsY = (height + workGroupSizeY - 1) / workGroupSizeY;
//        glDispatchCompute(numGroupsX, numGroupsY, 1);
//
//        // Ensure all writes to the image are complete
//        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
//
//        // Blit computed texture to the swapchain
//        BlitFramebufferToSwapchain(fb);
//
//        glfwSwapBuffers(window);
//        glfwPollEvents();
//    }
//
//    glfwDestroyWindow(window);
//    glfwTerminate();
//    return 0;
//}

#else
#include <iostream>

#include <glm/glm.hpp>

#include "Shader.h"
#include "Renderer.h"

static uint32_t s_ComputeShader = -1;
static const std::filesystem::path s_ComputeShaderPath = "C:/Users/jonas/Code2/C++/GPUCompute/App/Shaders/Compute.glsl";

static void ErrorCallback(int error, const char* description)
{
	std::cerr << "Error: " << description << std::endl;
}

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

	if (key == GLFW_KEY_R)
		s_ComputeShader = ReloadComputeShader(s_ComputeShader, s_ComputeShaderPath);
}

int main()
{
	glfwSetErrorCallback(ErrorCallback);

	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

	int width = 1280;
	int height = 720;

	GLFWwindow* window = glfwCreateWindow(width, height, "Compute", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetKeyCallback(window, KeyCallback);

	glfwMakeContextCurrent(window);
	gladLoadGL(glfwGetProcAddress);
	glfwSwapInterval(1);

	s_ComputeShader = CreateComputeShader(s_ComputeShaderPath);
	if (s_ComputeShader == -1)
	{
		std::cerr << "Compute shader failed\n";
		return -1;
	}

	Texture computeShaderTexture = CreateTexture(width, height);
	Framebuffer fb = CreateFramebufferWithTexture(computeShaderTexture);
	
	while (!glfwWindowShouldClose(window))
	{
		glfwGetFramebufferSize(window, &width, &height);

		// Resize texture
		if (width != computeShaderTexture.Width || height != computeShaderTexture.Height)
		{
			glDeleteTextures(1, &computeShaderTexture.Handle);
			computeShaderTexture = CreateTexture(width, height);
			AttachTextureToFramebuffer(fb, computeShaderTexture);
		}

		// Compute
		{
			glUseProgram(s_ComputeShader);
			glBindImageTexture(0, fb.ColorAttachment.Handle, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

			const GLuint workGroupSizeX = 16;
			const GLuint workGroupSizeY = 16;

			GLuint numGroupsX = (width + workGroupSizeX - 1) / workGroupSizeX;
			GLuint numGroupsY = (height + workGroupSizeY - 1) / workGroupSizeY;

			glDispatchCompute(numGroupsX, numGroupsY, 1);

			// Ensure all writes to the image are complete
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		}

		// Blit
		{
			BlitFramebufferToSwapchain(fb);
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);

	glfwTerminate();
}
#endif
