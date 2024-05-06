
;void memcpy(void* dest, void* src, uint64_t n)
memcpy:

    mov rcx, rdx
    rep movsb 
    ret

global memcpyConst
;void memcpyConst(void* dest, uint64_t n)
memcpyConst
    mov rcx, rsi
    rep stosb
    ret