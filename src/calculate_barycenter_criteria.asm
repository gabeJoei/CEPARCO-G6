section .data
align 32
; Mask for loading 5 elements ONLY
mask5: dd -1, -1, -1, -1, -1, 0, 0, 0

section .text
global calculate_barycenter_criteria
default rel

calculate_barycenter_criteria:
    ; PARAMETERS: float *class1, float *classNeg1, size_t rows, float *X, size_t numFeatures, float *filtered
    ; ASSIGNEMNT: class1 (RCX), classNeg1 (RDX), rows (R8), *X (R9), numFeatures (rbp+32), *filtered (rbp+40) 

    push rbp ; create stack frame as callee
    mov rbp, rsp
    add rbp, 8+8

    mov rax, rcx ; class1
    mov rax, rdx ; classNeg1
    mov rax, r8 ; rows
    mov rax, r9 ; *X
    mov rax, [rbp + 32] ; numFeatures
    mov rax, [rbp + 40] ; *filtered

    ; make sure rows is not 0
    test rdx, rdx
    jz .break
    
    xor r11, r11        ; clear register r11 to be used for row index.
    vpxor ymm7, ymm7    ; clear register ymm4 to be used for caclulating mean.
    vpxor ymm6, ymm6    ; clear register ymm4 to be used for caclulating mean.
    vpxor ymm4, ymm4

    ; RECALL: YMM takes in 8 floats so we have 0 extra floats that we dont want
    vmovdqa ymm8, [mask5]   ; we create 5 element mask into ymm1 so we can mask the 8 elements later on
    
    ; rows = RDX is the vector size. 
    ; RECALL: indexing in C is X[row * NUM_FEATURES + col]

.loop_rows:
    ; move to computing mean if all features in each datapoint has been added
    cmp r11, RDX
    jae .calculate_mean

    ; calculate indexing offset X[row * NUM_FEATURES] since we only need datapoints
    mov r12, r11
    imul r12, rcx      ; rax = r11 * rcx (row * NUM_FEATURES)
    mov rax, r12

    ; load ONLY 5 floats x1..x5 of current row 

    ;*X
    vmaskmovps ymm0, ymm8, [r9 + rax * 4] ; RECALL: 4 bytes per float
    ;Class1
    vmaskmovps ymm1, ymm8, [rcx + rax * 4] ; RECALL: 4 bytes per float
    ;classNeg1
    vmaskmovps ymm2, ymm8, [rdx + rax * 4] ; RECALL: 4 bytes per float

    ; Add feature values of this datapoint to overall summed features
    vsubss ymm6, ymm0, ymm1 ; || x - Zbj ||
    vsubss ymm5, ymm1, ymm2 ; || Zbi -  Zbj ||

    mulps ymm6, ymm6
    mulps ymm5, ymm5
    
    vaddps ymm4, ymm4, ymm6
    vaddps ymm4, ymm4, ymm5
    inc r11 ; move to next rows
    jmp .loop_rows

.calculate_mean:
    cvtsi2ss xmm1, rdx ; convert # of rows to float first
    vbroadcastss ymm1, xmm1

    ; divide overall summed features by total rows: ymm4 = ymm4 / rows
    vdivps ymm4, ymm4, ymm1

    ; REGISTER CONTENTS: [ 0 | 0 | 0 | x5_bar | x4_bar | x3_bar | x2_bar | x1_bar ]
    vmovups [r9], xmm4          ; Store first 4 floats (x1..x4) from lower 128 bits in xmm4
    vextractf128 xmm0, ymm4, 1  ; Get 5th float from upper 128 bits of ymm4
    vmovss [r9 + 16], xmm0      ; then store 5th float (x5), recall: floats r 4 bytes & vmovss is scalar

.break:
    pop rbp
    ret