section .text
global calculate_barycenter
default rel

calculate_barycenter:
    ; PARAMETERS: size_t features, size_t rows, float *X, float *outBarycenter
    ; ASSIGNEMNT: features (RCX), rows (RDX), *X (R8), *outBarycenter (R9)
    
    xor r11, r11    ; clear register r11 to be used for row index.
    vpxor xmm4, xmm4  ; clear register xmm4 to be used for caclulating mean.

    ; rows = RDX is the vector size. 
    ; RECALL: indexing in C is X[row * NUM_FEATURES + col]

.loop_rows
    ; move to computing mean if all features in each datapoint has been added
    cmp r11, RDX
    jae .calculate_mean

    ; calculate indexing offset X[row * NUM_FEATURES] since we only need datapoints
    imul rax, r11, rcx      ; rax = r11 * rcx (row * NUM_FEATURES)

    ; load 8 floats x1..x5 of current row + start of next row (which is a problem...)
    vmovups ymm0, [r8 + rax * 4] ; RECALL: 4 bytes per float

    ; Add feature values of this datapoint to overall summed features
    vaddps ymm4, ymm4, ymm0
    
    ; TO DO: code for featureSum += X[j * features + i];

    inc r11 ; move to next rows
    jmp .loop_row

.calculate_mean:
    cvtsi2ss xmm1, rdx ; convert # of rows to float first
    vbroadcastss ymm1, xmm1

    ; divide overall summed features by total rows: ymm4 = ymm4 / rows
    vdivps ymm4, ymm4, ymm1

    ; REGISTER CONTENTS: [ 0 | 0 | 0 | x5_bar | x4_bar | x3_bar | x2_bar | x1_bar ]
    ; RECALL: YMM takes in 8 floats so we have 0 extra floats that we dont want

    vmovups [r9], xmm4          ; Store first 4 floats (x1..x4) from lower 128 bits in xmm4
    vextractf128 xmm0, ymm4, 1  ; Get 5th float from upper 128 bits of ymm4
    vmovss [r9 + 16], xmm0      ; then store 5th float (x5), recall: floats r 4 bytes & vmovss is scalar

.break:
    ret