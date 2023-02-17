
function(CREATE_TEST TEST_NAME)
    add_executable(${TEST_NAME} ${TEST_NAME}.c test_shared.c)
    target_link_libraries(${TEST_NAME} ${LIBZEROXML} ${EXTRALIBS})
endfunction()

function(CREATE_TEST_CPP TEST_NAME EXTRALIBS)
    add_executable(${TEST_NAME} ${TEST_NAME}.cpp test_shared.c)
    target_link_libraries(${TEST_NAME} ${LIBZEROXML} ${EXTRALIBS})
endfunction()



CREATE_TEST(test_mmap)
CREATE_TEST(test_buffer)
CREATE_TEST(test_errors)
CREATE_TEST(test_functions)

CREATE_TEST_CPP(test_fuzzing -rdynamic)

