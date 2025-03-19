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

public:
    GLFWwindow *pubWindow = nullptr;
    std::string pubTitle = "glfw window";

    window(int width, int height, std::string title);
    void updateMemberVariables(int width, int height, std::string title, GLFWwindow *window);
    int getHeight();
    int getWidth();
    void start();
    ~window();
};
