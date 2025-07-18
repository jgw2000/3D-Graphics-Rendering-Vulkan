#include "BaseApp.h"

namespace jgw
{
    void BaseApp::Run()
    {
        glfwInit();
        GLFWwindow* window = glfwCreateWindow(640, 480, "GLFW window", NULL, NULL);

        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
        }

        glfwDestroyWindow(window);
        glfwTerminate();

        return;
    }
}