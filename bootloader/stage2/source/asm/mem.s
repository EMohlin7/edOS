
;void memcpy(void* dest, void* src, uint64_t n)
global memcpy
memcpy:
    mov rcx, rdx
    rep movsb 
    ret

;void memset(void* dest, uint8_t val, uint64_t n)
global memset
memset:
    mov rcx, rdx
    mov rax, rsi
    rep stosb
    ret

;int memcmp(void* ptr1, void* ptr2, uint64_t n)
global memcmp
memcmp:
    mov rcx, rdx
    repe cmpsb
    dec rdi
    dec rsi
    movzx rax, byte [rdi]
    movzx rdx, byte [rsi]
    sub rax, rdx
    ret