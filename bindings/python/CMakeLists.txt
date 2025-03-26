pybind11_add_module(jetpwmon_py MODULE
    jetpwmon_pybind.cpp
)

target_link_libraries(jetpwmon_py PRIVATE jetpwmon_static pybind11::headers)

# 设置输出名称
set_target_properties(jetpwmon_py PROPERTIES
    PREFIX ""
    OUTPUT_NAME "jetpwmon"
)

# 安装 Python 模块
if(SKBUILD)
    install(TARGETS jetpwmon_py DESTINATION ${PYTHON_INSTALL_DIR})
else()
    install(TARGETS jetpwmon_py DESTINATION ${Python_SITEARCH})
endif()
