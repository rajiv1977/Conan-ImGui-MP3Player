import os

from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps, CMake, cmake_layout
from conan.tools.build import check_min_cppstd
from conan.errors import ConanInvalidConfiguration
from conan.tools.files import copy

class mp3player(ConanFile):
    name = "mp3player"
    version = "1.0"
    
    author = "Rajiv Sithiravel"
    
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False], "optimized": [1, 2, 3]}
    default_options = {"shared": False, "fPIC": True, "optimized": 1}
    
    exports_sources = "CMakeLists.txt", "source/*", "include/*"
        
    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC
            
    def configure(self):
        if self.options.shared:
            del self.options.fPIC
            
    def validate(self):
        print(self.settings.os)
        if self.settings.os == "Windows":
            if self.settings.compiler == "msvc":
                check_min_cppstd(self, "20")
            elif self.settings.compiler == "intel-cc":
                check_min_cppstd(self, "gnu23")
                self.settings.compiler.mode = "dpcpp"      
                
    def validate(self):
        if self.settings.os == "Linux":
            if self.settings.compiler == "gcc":
                check_min_cppstd(self, "gnu23")
         
    status = 0;         
    def validate(self):
        if self.settings.os != "Windows":
            status = 1;
        elif self.settings.os != "Linux":
            status = 2;      
        if(status == 0):    
            raise ConanInvalidConfiguration("This package is only compatible with Windows or Linux")
                                                                    
    def requirements(self):
        self.requires("boost/1.83.0")
        self.requires("eigen/3.4.0")
        self.requires("ffmpeg/4.4.4")
        self.requires("freeglut/3.4.0")
        self.requires("glfw/3.3.8")
        self.requires("glew/2.2.0")
        self.requires("imgui/cci.20230105+1.89.2.docking") 
        self.requires("libjpeg/9e", override=True)
        self.requires("opengl/system")
        self.requires("opencv/4.8.1")
        self.requires("zlib/1.3")
           
    def build_requirements(self):
        self.tool_requires("cmake/3.28.1")
   
    def generate(self):
        copy(self, "*glfw*", os.path.join(self.dependencies["imgui"].package_folder,
             "res", "bindings"), os.path.join(self.source_folder, "bindings"))
        copy(self, "*opengl3*", os.path.join(self.dependencies["imgui"].package_folder,
             "res", "bindings"), os.path.join(self.source_folder, "bindings"))
               
    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()
        deps = CMakeDeps(self)
        deps.generate()
 
    def layout(self):
        cmake_layout(self)
    
    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()






        
    