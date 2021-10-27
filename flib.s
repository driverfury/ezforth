_start:
    jmp main

exit:
    movl $1,%eax
    popl %ebx
    syscall

putc:
    pushl %ebp
    movl %esp,%ebp
    movl $4,%eax
    movl $1,%ebx
    movl %ebp,%ecx
    addl $8,%ecx
    movl $1,%edx
    syscall
    leave
    ret

putint:
    pushl %ebp
    movl %esp,%ebp

    movl $1000000000,%ebx
    pushl %ebx
    movl 8(%ebp),%eax
    cmpl $0,%eax
    je .pizero
    jl .pineg
.piloop0:
    movl 8(%ebp),%eax
    cdq
    popl %ebx
    pushl %ebx

    idivl %ebx
    cmpl $0,%eax
    jg .piloop1
    popl %ebx
    movl $10,%ecx
    movl %ebx,%eax
    cdq
    idivl %ecx
    movl %eax,%ebx
    pushl %ebx
    jmp .piloop0
.piloop1:
    movl 8(%ebp),%eax
    cdq
    popl %ebx
    pushl %ebx
    idivl %ebx
    movl %edx,8(%ebp)

    addl $48,%eax
    pushl %eax
    call putc
    addl $4,%esp

    movl $10,%ecx
    popl %ebx
    pushl %ebx
    movl %ebx,%eax
    cdq
    idivl %ecx
    pushl %eax
    cmpl $0,%eax
    jg .piloop1
    jmp .piend
.pineg:
    pushl $45
    call putc
    addl $4,%esp

    movl 8(%ebp),%ecx
    negl %ecx
    movl %ecx,8(%ebp)
    jmp .piloop0
.pizero:
    pushl $48
    call putc
    addl $4,%esp
.piend:
    leave
    ret

rstack_start:
    .zero $400
rstack_end:

rstackp:
    .zero $4
