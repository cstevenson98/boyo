from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps


class BoyoConan(ConanFile):
    name = "boyo"
    version = "1.0.0"
    settings = "os", "compiler", "build_type", "arch"
    exports_sources = "CMakeLists.txt", "src/*"

    def requirements(self):
        self.requires("cpp-cli/1.0.0")

    def layout(self):
        build_type = str(self.settings.build_type)
        self.folders.build = f"build/{build_type}"
        self.folders.generators = f"build/{build_type}/generators"

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.variables["CMAKE_EXPORT_COMPILE_COMMANDS"] = "ON"
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()



