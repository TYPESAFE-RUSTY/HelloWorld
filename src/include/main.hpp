#pragma once
#include <iostream>
#include <string>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class window
{
private:
    int m_width = 500;
    int m_height = 500;
    std::string m_title = "glfw window";
    GLFWwindow *m_window = nullptr;
    GLuint m_vertexBuffer;

public:
    window(int width, int height, std::string title);
    void updateMemberVariables(int width, int height, std::string title, GLFWwindow *window);
    void start();
    ~window();
};

window::window(int width, int height, std::string title)
{
    window::updateMemberVariables(width, height, title, nullptr);

    glewExperimental = true;

    if (!glfwInit())
    {
        fprintf(stderr, "unable to initalize glfw");
        glfwTerminate();
        return;
    }

    // telling glfw what flavour of opengl is required
    glfwWindowHint(GLFW_SAMPLES, 4);               // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);           // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL

    m_window = glfwCreateWindow(m_width, m_height, m_title.data(), NULL, NULL);
    if (m_window == NULL)
    {
        fprintf(stderr, "unable to start a window");
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(m_window); // Initialize GLEW
    glewExperimental = true;          // Needed in core profile
    if (glewInit() != GLEW_OK)
    {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return;
    }

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // An array of 3 vectors which represents 3 vertices
    static const GLfloat g_vertex_buffer_data[] = {
        -0.5f, // x
        -0.5f, // y
        0.0f,  // z
        0.5f,
        -0.5f,
        0.0f,
        0.0f,
        0.5f,
        0.0f,
    };

    glGenBuffers(1, &m_vertexBuffer);
    // The following commands will talk about our 'vertexbuffer' buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    // Give our vertices to OpenGL.
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
}

void window::updateMemberVariables(int width, int height, std::string title, GLFWwindow *window)
{
    m_width = width;
    m_height = height;
    m_title = title;
    m_window = window;
}

void window::start()
{
    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(m_window, GLFW_STICKY_KEYS, GL_TRUE);

    do
    {
        glClear(GL_COLOR_BUFFER_BIT);
        // 1st attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
        glVertexAttribPointer(
            0,        // attribute 0. No particular reason for 0, but must match the layout in the shader.
            3,        // size
            GL_FLOAT, // type
            GL_FALSE, // normalized?
            0,        // stride
            (void *)0 // array buffer offset
        );
        // Draw the triangle !
        glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
        glDisableVertexAttribArray(0);
        // Swap buffers
        glfwSwapBuffers(m_window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while (glfwGetKey(m_window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
           glfwWindowShouldClose(m_window) == 0);
}

window::~window()
{
    glfwDestroyWindow(m_window);
    glfwTerminate();
}
