from conan import ConanFile
from conan.tools.cmake import cmake_layout


class ExampleRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        self.requires("fmt/11.2.0", override=True)
        self.requires("glfw/3.4")
        self.requires("spdlog/1.15.3")
        self.requires("assimp/5.4.3")
        self.requires("stb/cci.20230920")
        self.requires("imgui/1.92.0")
        self.requires("ktx/4.3.2")
        self.requires("meshoptimizer/0.23")

    def layout(self):
        cmake_layout(self)