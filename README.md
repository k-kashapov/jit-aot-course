# JIT/AOT Compilers course HW

## Description

This is a toy IR written in C++.
WIP.

## Usage

```
mkdir build && cd build
cmake .. && make -j
./test/loop_test
```

## Example

```
start (Preds: loopBB)  (Succs: exitBB, loopBB):
        $0<UI32> = ParamOp <UI32>
        $1<UI32> = Const (1)
        $2<BOOL> = EqOp ($0<UI32>, $1<UI32>)
        $3<UI32> = Const (0)
        $4<BOOL> = EqOp ($0<UI32>, $3<UI32>)
        $5<BOOL> = OrOp ($4<BOOL>, $2<BOOL>)
        $6<UI32> = PhiNode (start.0, loopBB.0)
        $7<> = Jmp to exitBB if $5<BOOL>

loopBB (Preds: start)  (Succs: start, none):
        ...
```
