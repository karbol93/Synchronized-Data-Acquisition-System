# Available at setup time due to pyproject.toml
from pybind11.setup_helpers import Pybind11Extension, build_ext
from setuptools import setup, Extension
from os import getcwd,listdir

ext_modules = [
    Pybind11Extension(
        name="kespp",
        sources=[
            "src/kespp_py.cpp",
            # "src/include/frametypes.hpp",
            # "src/include/kespp_client.hpp"
        ],
        # Example: passing in the version to the compiled code
        # define_macros = [('VERSION_INFO', __version__)],
        # library_dirs=["include"],
        # headers=["include/kespp_client.hpp"],
        # depends=["include/kespp_client.hpp", "include/frametypes.hpp"],
        # extra_objects=["include/kespp_client.hpp", "include/frametypes.hpp"],
        # extra_objects=["include/kespp_client.hpp", "include/frametypes.hpp"],
        include_dirs=["include"],
    ),
    # Extension(
    #     name="",
    #     sources="",
    #     depends=[],
    #     extra_objects=[],

    # )
]

# print('-----------------------------------------------')
# print(getcwd())
# print(listdir(getcwd()), sep='\n')
# print('-----------------------------------------------')

setup(
    ext_modules=ext_modules,
    # Currently, build_ext only provides an optional "highest supported C++
    # level" feature, but in the future it may provide more features.
    cmdclass={"build_ext": build_ext},
)
