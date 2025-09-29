# JIT/AOT Compilers course HW

## Description

This is a toy IR written in C++.
WIP.

## Usage

```
make
./build/testIR.elf
```

## Example

```
testBB:
        $in1[F16] = ParamOp[F16]
        $in2[F16] = ParamOp[F16]
        $add[F16] = AddOp ($in1[F16], $in2[F16])
        $jmp[] = Jmp to testBB3

testBB2:
        $in1[F16] = ParamOp[F16]
        $in2[F16] = ParamOp[F16]
        $add[F16] = AddOp ($in1[F16], $in2[F16])
        $jmp[] = Jmp to testBB3

testBB3:
        $Phi[F16] = PhiNode (testBB.add, testBB2.add)
```
