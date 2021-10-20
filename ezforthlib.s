.extern _exit
.extern _putchar

.text

ezforth_exit:
    call _exit

print_char:
    pushl %ebp
    mov %esp,%ebp

    pushl 8(%ebp)
    call _putchar

    leave
    ret

print_int:
    pushl   %ebp
    movl    %esp, %ebp

    subl    $24, %esp
    cmpl    $0, 8(%ebp)
    jns     .L3
    subl    $12, %esp
    pushl   $45
    call    print_char
    addl    $16, %esp
    negl    8(%ebp)
    jmp     .L4
.L3:
    cmpl    $0, 8(%ebp)
    jne     .L4
    subl    $12, %esp
    pushl   $48
    call    print_char
    addl    $16, %esp
    jmp     .L2
.L4:
    movl    $1000000000, -12(%ebp)
    jmp     .L6
.L7:
    movl    -12(%ebp), %ecx
    movl    $1717986919, %edx
    movl    %ecx, %eax
    imull   %edx
    movl    %edx, %eax
    sarl    $2, %eax
    sarl    $31, %ecx
    movl    %ecx, %edx
    subl    %edx, %eax
    movl    %eax, -12(%ebp)
.L6:
    movl    8(%ebp), %eax
    cltd
    idivl   -12(%ebp)
    testl   %eax, %eax
    je      .L7
    jmp     .L8
.L9:
    movl    8(%ebp), %eax
    cltd
    idivl   -12(%ebp)
    addl    $48, %eax
    movsbl  %al, %eax
    subl    $12, %esp
    pushl   %eax
    call    print_char
    addl    $16, %esp
    movl    8(%ebp), %eax
    cltd
    idivl   -12(%ebp)
    movl    %edx, 8(%ebp)
    movl    -12(%ebp), %ecx
    movl    $1717986919, %edx
    movl    %ecx, %eax
    imull   %edx
    movl    %edx, %eax
    sarl    $2, %eax
    sarl    $31, %ecx
    movl    %ecx, %edx
    subl    %edx, %eax
    movl    %eax, -12(%ebp)
.L8:
    cmpl    $0, -12(%ebp)
    jg      .L9
.L2:

    leave
    ret
