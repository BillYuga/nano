build_dir="./build-debug-lsan"

mkdir -p ${build_dir}
cd ${build_dir}
rm -rf *

cmake_params=""
cmake_params=${cmake_params}" -DCMAKE_BUILD_TYPE=Debug"
cmake_params=${cmake_params}" -DNANOCV_WITH_ASAN=OFF"
cmake_params=${cmake_params}" -DNANOCV_WITH_LSAN=ON"
cmake_params=${cmake_params}" -DNANOCV_WITH_TSAN=OFF"
cmake_params=${cmake_params}" -DNANOCV_HAVE_CUDA=OFF"
cmake_params=${cmake_params}" -DNANOCV_HAVE_OPENCL=OFF"
cmake_params=${cmake_params}" -G Ninja"

cmake ${cmake_params} ../

ninja
cd ..


