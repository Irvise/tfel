# a simple macro
macro(tests_math_tensor test_arg)
  add_executable(${test_arg} EXCLUDE_FROM_ALL ${test_arg}.cxx)
  add_test(NAME ${test_arg} COMMAND ${test_arg})
  add_dependencies(check ${test_arg})
  if((CMAKE_HOST_WIN32) AND (NOT MSYS))
    set_property(TEST ${test_arg}
  	         PROPERTY ENVIRONMENT "PATH=$<TARGET_FILE_DIR:TFELMath>\;$<TARGET_FILE_DIR:TFELUtilities>\;$<TARGET_FILE_DIR:TFELException>\;$<TARGET_FILE_DIR:TFELTests>\;$ENV{PATH}")
  endif((CMAKE_HOST_WIN32) AND (NOT MSYS))
  target_link_libraries(${test_arg} TFELMath TFELUtilities TFELException TFELTests)
endmacro(tests_math_tensor)

tests_math_tensor(tensor)
tests_math_tensor(tensor2)
tests_math_tensor(tensor3)
tests_math_tensor(tensor4)
tests_math_tensor(TensorComputeJacobianDerivativeTest)
