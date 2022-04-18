# Experimental LLVM Backend for Intel 8085 architecture

For working examples, current status of this backend, Please refer this [target's readme](llvm/lib/Target/I8085).

## Build

```
cd llvm 
mkdir build
cmake -G "Ninja" -DLLVM_TARGETS_TO_BUILD="I8085" -DCMAKE_BUILD_TYPE="Debug" -DLLVM_ENABLE_ASSERTIONS=On ../
ninja
```

For using other build systems and methods, please refer official LLVM Build Docs.
