section .data
align 32
; Mask for loading 5 elements ONLY
mask5: dd -1, -1, -1, -1, -1, 0, 0, 0

section .text
global calculate_barycenter_criteria
default rel

calculate_barycenter_criteria:
    ; PARAMETERS: float *class1, float *classNeg1, float *X, size_t numFeatures, float *filtered
    ; ASSIGNEMNT: class1 (RCX), classNeg1 (RDX), *X (R8), numFeatures (R9), *filtered (rbp + 32) 

    push rbp ; create stack frame as callee
    mov rbp, rsp
    add rbp, 8+8

    mov rax, rcx ; class1
    mov rax, rdx ; classNeg1
    mov rax, r8 ; *X
    mov rax, r9 ; numFeatures
    mov rax, [rbp + 32] ; *filtered

    ; make sure rows is not 0
    test rdx, rdx
    jz .break
    
    xor r11, r11        ; clear register r11 to be used for row index.
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
    cmp r11, rdx
    jae .break

    ; calculate indexing offset X[row * NUM_FEATURES] since we only need datapoints
    mov r12, r11
    imul r12, rcx      ; rax = r11 * rcx (row * NUM_FEATURES)
    mov rax, r12

    ; load ONLY 5 floats x1..x5 of current row 

    ;*X
    vmaskmovps ymm0, ymm8, [r8 + rax * 4] 
    ;Class1
    vmaskmovps ymm1, ymm8, [rcx + rax * 4] 
    ;classNeg1
    vmaskmovps ymm2, ymm8, [rdx + rax * 4] 

    cmp ymm1, ymm2
    je .break
    
    ; Subtract the value of ymm0 to ymm1 vertically
    vsubps ymm6, ymm0, ymm1 ; || x - Zbj || = A
    vsubps ymm5, ymm1, ymm2 ; || Zbi -  Zbj || = B 

    ; Raise ymm6 and ymm5 by 2 or multiple ymm6 and ymm5 by itself
    mulps ymm6, ymm6 ; A * A
    mulps ymm5, ymm5 ; B * B
    
    
    vaddps ymm6, ymm6, ymm6 ; Summation of A element 
    vaddps ymm5, ymm5, ymm5 ; Summation of B element
    
    ; Store the sqrt value of ymm6 and ymm5 to r6 and r7 register
    sqrtss r6, ymm6 ; Sqrt of A
    sqrtss r5, ymm5 ; Sqrt of B

    ; Main comparision, if the euclidean distance of r6 is greater than
    ; the euclidean distance of r5 then store the value else, move to the next row
    cmp r6, r5
    jb .store

    inc r11 ; move to next rows
    jmp .loop_rows

.store
    ; Store valid data points into ymm4
    movss [ymm4 + r11 * 4 ], r6
 
    inc r11 ; move to next rows
    jmp .loop_rows

.break:
    vmovups [rbp + 32], ymm4 
    pop rbp
    ret