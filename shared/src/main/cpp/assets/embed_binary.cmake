function(embed_binary INPUT_PATH INPUT_FILE OUTPUT_OBJ_VAR)
  # Determine objcopy arch based on ANDROID_ABI
  if(${ANDROID_ABI} STREQUAL "armeabi-v7a")
    set(objcopy_arch "arm")
    set(objcopy_bits "32")
  elseif(${ANDROID_ABI} STREQUAL "arm64-v8a")
    set(objcopy_arch "aarch64")
    set(objcopy_bits "64")
  elseif(${ANDROID_ABI} STREQUAL "x86")
    set(objcopy_arch "i386")
    set(objcopy_bits "32")
  elseif(${ANDROID_ABI} STREQUAL "x86_64")
    set(objcopy_arch "x86-64")
    set(objcopy_bits "64")
  else()
    message(FATAL_ERROR "Unsupported ANDROID_ABI ${ANDROID_ABI}")
  endif()

  string(TOLOWER ${OUTPUT_OBJ_VAR} OUTPUT_FILE_NAME)
  # Derive output object file path inside build dir based on OUTPUT_OBJ_VAR
  set(output_obj "${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_FILE_NAME}.o")

  # Create the custom command to embed the file
  add_custom_command(
          OUTPUT ${output_obj}
          COMMAND ${CMAKE_OBJCOPY}
          ARGS -I binary
          -O elf${objcopy_bits}-${objcopy_arch}
          -B ${objcopy_arch}
          ${INPUT_FILE} ${output_obj}
          WORKING_DIRECTORY ${INPUT_PATH}
          COMMENT "Embedding ${INPUT_FILE} as ${output_obj} in ${INPUT_PATH}"
          VERBATIM
  )

  # Set OUTPUT_OBJ_VAR in parent scope to the generated object file
  set(${OUTPUT_OBJ_VAR} ${output_obj} PARENT_SCOPE)

  # Set symbol prefix variables for C/C++ code
  # set(${SYM_PREFIX}_START "_binary_${OUTPUT_OBJ_VAR}_start" PARENT_SCOPE)
  # set(${SYM_PREFIX}_END "_binary_${OUTPUT_OBJ_VAR}_end" PARENT_SCOPE)
endfunction()
