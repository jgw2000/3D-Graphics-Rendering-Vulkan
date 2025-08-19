python deploy_deps.py

conan profile detect --force
conan install . --output-folder=. --build=missing --profile=profiles/debug
cmake --preset conan-default
cmake --build --preset conan-debug
conan install . --output-folder=. --build=missing --profile=profiles/release
cmake --preset conan-default
cmake --build --preset conan-release