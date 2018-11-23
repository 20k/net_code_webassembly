clang -emit-llvm --target=wasm32 -O3 test_1.c -c -o test_1.bc -IC:/cllvm2/include
llc -march=wasm32 -filetype=obj test_1.bc -o test_1.o
llc -march=wasm32 test_1.bc -o test_1.s
lld -flavor wasm --export-all --allow-undefined test_1.o -o test_1.wasm -LC:/cllvm2/lib -lc