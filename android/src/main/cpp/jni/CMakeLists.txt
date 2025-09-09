cmake_minimum_required(VERSION 3.19.2)

project("jni" C)

add_library(jni SHARED ruby_vm_jni.c jni_ruby_info.c)
set_target_properties(jni PROPERTIES COMPILE_FLAGS "-Wall -Wextra -Werror")
target_link_libraries(jni ruby-vm assets logging
        ${log-lib} ${zlib-lib}
        ${libs_sfml_dependencies})
target_include_directories(jni PRIVATE ${IMPORT_DIR} ${LOGGING_DIR} ${RUBY_VM_DIR})
