#pragma once
#include <iostream>
#include <string>
#include <stdlib.h>
#include <GLFW/glfw3.h>

class window
{
private:
    int m_width = 500;
    int m_height = 500;
    std::string m_title = "glfw window";
    GLFWwindow *m_window = nullptr;

public:
    window(int width, int height, std::string title);
    void updateMemberVariables(int width, int height, std::string title, GLFWwindow *window);
    void start();
    ~window();
};

window::window(int width, int height, std::string title)
{
    window::updateMemberVariables(width, height, title, nullptr);

    if (!glfwInit())
    {
        fprintf(stderr, "unable to initalize glfw");
        glfwTerminate();
        return;
    }
    m_window = glfwCreateWindow(m_width, m_height, m_title.data(), NULL, NULL);
}

void window::updateMemberVariables(int width, int height, std::string title, GLFWwindow *window)
{
    m_height = width;
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
        // glClear(GL_COLOR_BUFFER_BIT);
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
