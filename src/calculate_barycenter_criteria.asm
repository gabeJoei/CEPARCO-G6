section .data
align 32
; Mask for loading 5 elements ONLY
mask5: dd -1, -1, -1, -1, -1, 0, 0, 0

section .text
global calculate_barycenter_criteria
default rel

calculate_barycenter_criteria:
    ; PARAMETERS: float *class1, float *classNeg1, float *X,  float *filtered, size_t numFeatures, size_t rows
    ; ASSIGNEMNT: class1 (RCX), classNeg1 (RDX), *X (R8), *filtered (R9) , numFeatures (rbp + 32), rows (rbp +40)

    push rbp ; create stack frame as callee
    mov rbp, rsp
    add rbp, 8+8

    mov rax, rcx ; class1
    mov rax, rdx ; classNeg1
    mov rax, r8 ; *X
    mov rax, r9 ; *filtered
    mov rax, [rbp + 32] ; numFeatures
    mov rax, [rbp + 40] ; numRows

    ; make sure rows is not 0
    test qword [rbp + 40], 0
    jz .break
    
    xor r11, r11   
    xor r13, r13     ; clear register r11 to be used for row index.
    vpxor ymm7, ymm7   
    vpxor ymm6, ymm6    
    vpxor ymm5, ymm5
    vpxor ymm4, ymm4

    ; RECALL: YMM takes in 8 floats so we have 0 extra floats that we dont want
    vmovdqa ymm8, [mask5]   ; we create 5 element mask into ymm1 so we can mask the 8 elements later on
    
    ; rows = RDX is the vector size. 
    ; RECALL: indexing in C is X[row * NUM_FEATURES + col]



.loop_rows:
    ; move to computing mean if all features in each datapoint has been added
    cmp r11, [rbp + 40]
    jae .break

    ; calculate indexing offset X[row * NUM_FEATURES] since we only need datapoints
    mov r12, r11
    imul r12, [rbp + 32]      ; rax = r11 * rcx (row * NUM_FEATURES)
    mov rax, r12

    ; load ONLY 5 floats x1..x5 of current row 

    ;*X
    vmaskmovps ymm0, ymm8, [r8 + rax * 4] 
    ;Class1
    vmaskmovps ymm1, ymm8, [rcx + rax * 4] 
    ;classNeg1
    vmaskmovps ymm2, ymm8, [rdx + rax * 4] 

    vcmpps ymm3,ymm1,ymm2,0
    vmovmskps eax, ymm3
    cmp eax, 255
    je .break
    
    ; Subtract the value of ymm0 to ymm1 vertically
    vsubps ymm6, ymm0, ymm1 ; || x - Zbj || = A
    vsubps ymm5, ymm1, ymm2 ; || Zbi -  Zbj || = B 

    ; Raise ymm6 and ymm5 by 2 or multiple ymm6 and ymm5 by itself
    vmulps ymm6, ymm6, ymm6 ; A * A
    vmulps ymm5, ymm5, ymm5 ; B * B
    
    ; Separately add elements 1-4 and 5th element because 
    ; vhaddps can only add 128 bits instead of 256 bits
    vextractf128 xmm0, ymm6, 1
    vaddps xmm6, xmm6, xmm0
    vhaddps xmm6, xmm6, xmm6 ; Summation of A element 
    vhaddps xmm6, xmm6, xmm6 ; Summation of A element 
    
    vextractf128 xmm0, ymm5, 1
    vaddps xmm5, xmm5, xmm0
    vhaddps xmm5, xmm5, xmm5 ; Summation of B element 
    vhaddps xmm5, xmm5, xmm5 ; Summation of B element 
    
    ; Store the sqrt value of ymm6 and ymm5 to r6 and r7 register
    vsqrtss xmm6, xmm6, xmm6 ; Sqrt of A, since the value we want to sqrt is a single val
    vsqrtss xmm5, xmm5, xmm5 ; Sqrt of B

    ; Main comparision, if the euclidean distance of r6 is greater than
    ; the euclidean distance of r5 then store the value else, move to the next row
    
    comiss xmm6, xmm5
    jb .store

    inc r11 ; move to next rows
    jmp .loop_rows

.store:
    ; Store valid data points into ymm4
    movss [r9 + r13 * 4 ], xmm6
 
    inc r11 ; move to next rows
    inc r13
    jmp .loop_rows

.break:
    pop rbp
    ret