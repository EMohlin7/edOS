#define VID_MEM (short*)0xB8000

int main(){
    int length = 4;
    short data[] = {0x0F48, 0x0F6F, 0x0F6F, 0x1F6F};
    volatile short* mem = VID_MEM;
    for(int i = 0; i< length; ++i){
        mem[i] = data[i];
    }
    
    while(1);
}