# Oceantoo
Oceantoo is an XOR/LFSR based encryption algorithm

## What is an LFSR?

A linear-feedback shift register (LFSR) is a method of manipulating a number (a register) by shifting it (moving all the bits to the left or right) and then adding a new bit in the space that was created (the feedback).

They are often used as pseudo-random numbers generators (as in this case).

The initial state is called the seed, and because an LFSR is deterministic it will always generate the same number sequence when starting from a given initial state.

The feedback works by taking certain bits (called taps) and combining them using XOR to generate the bit which will be inserted into the gap created by the sift.

### XOR refresher
Exclusive OR is true if, and only if, its arguments differ. So:

|A|B|XOR|
|-|-|---|
|0|0|0|
|0|1|1|
|1|0|1|
|1|1|0|


### Bit numbering
When reading about LFSRs there can be confusion about how the bits are numbered.

For example `1011` could be numbered as

|1|2|3|4|
|-|-|-|-|
|1|0|1|1|

or as 

|4|3|2|1|
|-|-|-|-|
|1|0|1|1|

The first method is the traditional way when talking about LFSRs.

### 4 bit LFSR
An example of a simple 4 bit LFSR is to use bits 3 and 4. These are combined using XOR and inserted into bit 1, after it has been shifted to the left.

|1|2|3|4|Notes|
|-|-|-|-|-----|
|1|0|0|1|Bit 3 (0) XOR Bit 4 (1) is 1|
|1|1|0|0| `1001` shifted right is `?100` and then add the carry from above to make `1100`|
|0|1|1|0||
|1|0|1|1||
|0|1|0|1||
|1|0|1|0||
|1|1|0|1||
|1|1|1|0||
|1|1|1|1|Bit 3 (1) XOR Bit 4 (1) is 0|
|0|1|1|1|`1111` shifted right is `?111` and then add the carry from above to make `0111`|
|0|0|1|1||
|0|0|0|1||
|1|0|0|0||
|0|1|0|0||
|0|0|1|0||

### 128 bit LFSR
Oceantoo uses a 128-bit LFSR with the taps of 128, 127, 126, 121. Such an LFSR has a maximum cycle, meaning it will generate 340,282,366,920,938,463,463,374,607,431,768,211,455 unique numbers, with no repeats!!!
