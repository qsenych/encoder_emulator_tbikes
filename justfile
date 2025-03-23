alias g := generate
alias b := build
build:
  make -j$(nproc)

generate:
  python generate.py
  clang-format Core/Src/main.c -i

