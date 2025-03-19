#include <window.hpp>

window::window(int width, int height, std::string title)
{
    window::updateMemberVariables(width, height, title, nullptr);

    if (!glfwInit())
    {
        fprintf(stderr, "unable to initalize glfw");
        glfwTerminate();
        return;
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    pubWindow = glfwCreateWindow(m_width, m_height, pubTitle.data(), NULL, NULL);
}

void window::updateMemberVariables(int width, int height, std::string title, GLFWwindow *window)
{
    m_height = width;
    m_height = height;
    pubTitle = title;
    pubWindow = window;
}

int window::getHeight()
{
    return this->m_height;
}
int window::getWidth()
{
    return this->m_width;
}
void window::start()
{
    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(pubWindow, GLFW_STICKY_KEYS, GL_TRUE);

    do
    {
        // glClear(GL_COLOR_BUFFER_BIT);
        // Swap buffers
        glfwSwapBuffers(pubWindow);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while (glfwGetKey(pubWindow, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
           glfwWindowShouldClose(pubWindow) == 0);
}

window::~window()
{
    glfwDestroyWindow(pubWindow);
    glfwTerminate();
}