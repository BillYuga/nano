list(APPEND libnano_sources
        ${CMAKE_CURRENT_SOURCE_DIR}/synth_bitstream_vera_sans_mono_bold.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/synth_bitstream_vera_sans_mono.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/synth_dejavu_sans_mono_bold.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/synth_dejavu_sans_mono.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/synth_droid_sans_mono.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/synth_liberation_mono_bold.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/synth_liberation_mono.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/synth_nimbus_mono_bold.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/synth_nimbus_mono.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/synth_oxygen_mono.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/task_charset.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/task_cifar100.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/task_cifar10.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/task_mnist.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/task_affine.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/task_stl10.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/task_svhn.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/task_sign.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/task_matmul.cpp)

set(libnano_sources "${libnano_sources}" PARENT_SCOPE)
