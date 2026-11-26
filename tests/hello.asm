; Minimal test program: prints a line and exits with code 42.
; Build: nasm -f elf64 hello.asm -o hello.o && ld hello.o -o hello
        global _start
        section .data
msg:    db "hello from the test program", 10
len     equ $ - msg

        section .text
_start:
        mov     rax, 1          ; write
        mov     rdi, 1          ; stdout
        lea     rsi, [rel msg]
        mov     rdx, len
        syscall

        mov     rax, 60         ; exit
        mov     rdi, 42
        syscall
