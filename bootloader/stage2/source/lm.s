%define KERNEL_ADDRESS 0xffffff0000000000

global LM
extern mapKernel
extern kernelSize
extern kernelAddress

BITS 64

LM:
    ;Set segment registers to data segment 
    mov eax, 16            
    mov ds, eax            
    mov es, eax 
    mov fs, eax   
    mov gs, eax 
    mov ss, eax    



    ;uint64_t mapKernel(uint64_t* PML4, uint32_t kernelSize, uint32_t kernelPhysicalAddrs)
    mov rdi, 0x1000
    mov esi, dword [kernelSize]
    mov edx, dword [kernelAddress]
    call mapKernel

    ;movzx rax, word [programClusters]
    ;mul byte [sectorsPerCluster]
    ;mul dword [sectorSize]
    ;mov rdi, rax
    mov rdi, 0
    rex.w mov rax, KERNEL_ADDRESS
    jmp rax



