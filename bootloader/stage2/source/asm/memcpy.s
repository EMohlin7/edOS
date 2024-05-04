
global memcpy
;void memcpy(void* dest, void* src, uint64_t n)
memcpy:

    mov rcx, rdx
    rep movsb 
    ret