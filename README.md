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

When generating random number sequences the bit that is shifted out, that is bit 1, the least significant bit (LSB) is used to generate random bits which are used as random numbers (depending on how many bits are needed).

### 128 bit LFSR
Oceantoo uses a 128-bit LFSR with the taps of 128, 127, 126, 121. Such an LFSR has a maximum cycle, meaning it will generate 340,282,366,920,938,463,463,374,607,431,768,211,455 unique numbers, with no repeats!!!

### LFSR Weakness
Given a sufficient amount of data there are ways to determine "crack" the LFSR and get its initial state and simulate the numbers it generates. The Berlekamp–Massey algorithm is an algorithm that will find the shortest linear feedback shift register (LFSR) for a given binary output sequence.

To overcome this weakness there are various methods which can be used. One method is to use a multiplexed LFSR. These were first mentioned in the PhD thesis of S. M. Jennings in 1980.

Multiplexed LFSRs and Shrinking Generators use multiple LFSRs. In the case of the latter one LFSR controls how the other is used. So if you have LFSR1 and LFSR2, when the LSB of LSFR1 is 1, then the output of the sequence is the LSB of LFSR2. When LFSR1 is 0, the LSB of LFSR2 is discarded. The algorithm then loops, until LFSR1 generates a 1, and so LFSR2 is used.

Other variations include alternating between LFSR1 and LFSR2, etc.

### A note on FISH and PIKE

FISH (FIbonacci SHrinking) is an encryption algorithm published by Siemens in 1993. According to Ross Anderson showed that FISH can be broken with just a few thousand bits of known plaintext (see On Fibonacci KEystream Generators).

In the same paper Anderson suggested PIKE. PIKE improves on FISH. Anderson notes that "had the control bits been the carry bits rather than the least significant bits, then our attack would have been much harder".

Therefore Oceantoo uses the carry bits of an LFSR to shrink the sequence (see below).

PIKE also used XOR for the bit stream, as does Oceantoo.

### Oceantoo's LFSRs
In the paper "Statistical Weaknessof Multiplexed Sequences" Jovan Dj. Golic et al showed that there are some inherent weaknesses in multiplexed LFSRs. The paper also suggested some remedies. Namely to decimate (shrink) the output of the LFSR (similar to the shrinking generator) and to XOR the output of the two LFSRs.

Oceantoo's uses 3 LFSRS. All 128-bit.

LFSR3 is used to decimate the output. If the carry bit of LFSR3 is 1 then the LSB of LFSR1 is ignored for 1 bit, while the LSB of LFSR2 is ignored for 2 bits. However the algorithm doesn't loop back around. This is an arbitrary action taken whenever the carry bit of LFSR3 is 1.

Then the output of the random number sequence generator is LFSR1 XOR LFSR2.

The idea is that the state of LFSR1 and LFSR2 can't be determined since the sequence is an XOR of the generated bits. Furthermore, LFSR1 and LFSR2 aren't in step since the shrinking of LFSR1 and LFSR2 is different and only occurs when the carry bit of LFSR3 is 1.

### Seed
Each LFSR needs an initial state, in fact 128-bits of initial state. Oceantoo initializes the LFSRs by taking a plaintext password and generating a SHA256 hash for the string. The first 128 bits of that hash are used to initialise an LFSR, called the LFSR Captain. The captain is then used as a standard LFSR to generate 3x128bits of data, that are used to initialize LFSR1-3. LFSR Captain is then discarded. For decode, the initial state of the three LFSRs will be the same if the same password is supplied.

Oceantoo also has the option to move along its sequence by `n` bytes before starting the encode or decode. This means that to encode a file you need to supply a password and an optional offset. To decode you need the same password and the same offset, otherwise decode will fail.

### Example usage

* `oceantoo -p mypassword1 secret_designs.doc secret_designs.doc.oct` - This will encode the Word document `secret_designs.doc` with the key `mypassword1` and write the output to `secret_designs.doc.oct'
* 'oceantoo -p mypassword1 secret_designs.doc.oct secret_designs.doc` - Is the reverse (i.e. decode) of above.
* `oceantoo -p mypassword1 -n 187172 secret_designs.doc secret_designs.doc.oct` - This will encode the Word document `secret_designs.doc` with the key `mypassword1` and offset of 187172 and write the output to `secret_designs.doc.oct'
* `oceantoo -p mypassword1 -n 187172 secret_designs.doc.oct secret_designs.doc` - Is the reverse (i.e. decode) of above.
* `oceantoo -p mypassword1 -n 9854213 -r -l 50` - Will print 50 random bytes (0-255) from the Oceantoo's LFSRs based on the password and offset.
