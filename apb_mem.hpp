#ifndef __APB_MEM__
#define __APB_MEM__

#include <cstdint>
#include<stdint.h>
#include <stdio.h>

#include<stdint.h>
#include <stdio.h>

template <typename ADDRT, typename DATAT>
class apb_mem {
public:
// apb4 signal list
    uint8_t* pclk     ;
    uint8_t* presetn  ;

    uint8_t* penable  ;
    uint8_t* pwrite   ;
    ADDRT* paddr      ;
    uint8_t* psel     ;
    DATAT* pwdata     ;
    uint8_t* pprot    ; // TODO: [2:0]
    uint8_t* pstrb    ; // TODO: 0 -> 0-7 | 1 -> 8 -> 15
    DATAT* prdata     ; // input
    uint8_t* pready   ; // input
    uint8_t* pslverr  ; // input

// signal about the memory
    uint8_t* mem;
    size_t mem_base_addr;

// transaction information
    ADDRT waddr;
    ADDRT raddr;

    size_t w_delay = 0 ;
    size_t r_delay = 0 ;

    size_t w_delay_count = 0;
    size_t r_delay_count = 0;

    typedef bool (*UNCACHED_WRITE) (ADDRT waddr, DATAT wdata, uint8_t wlen);
    typedef bool (*UNCACHED_READ) (ADDRT raddr, DATAT& rdata);
    UNCACHED_WRITE uncached_write;
    UNCACHED_READ uncached_read;

    apb_mem(/* args */) {};
    ~apb_mem() {};

    void init(const size_t w_delay , const size_t r_delay, const size_t mem_base_addr, uint8_t* const mem, UNCACHED_WRITE func_ptr0, UNCACHED_READ func_ptr1);
    void apb_signal_update();
};

template <typename ADDRT, typename DATAT>
void apb_mem<ADDRT, DATAT>::apb_signal_update()
{
    *pready = 0;
    *pslverr = 0; // TODO

    if (!this->presetn)
        return ;
    
    if (!this->pclk)
        return ;
    
    if (!*psel)
        return ;

    if (*pwrite) { // write transaction
        waddr = *paddr;

        if (!*penable)
            return ;

        if (w_delay_count) {
            w_delay_count--;

            return ;
        }
        else {
            #ifdef APB_LOG
                printf("[write] %x: %x\n", waddr, *pwdata); // TODO: %x?
            #endif

            if ((uncached_write == NULL) || (!uncached_write(waddr, *pwdata, 8))) // uncached mem write
                for (size_t i = 0; i < sizeof(DATAT); i++) { // write to the memory
                    mem[i + waddr - mem_base_addr] = (uint8_t)(*pwdata & 0xff);
                    *pwdata >>= 8;
                }

            w_delay_count = w_delay; // reset delay value
            r_delay_count = r_delay; // reset delay value
            *pready = 1;
        }
    }
    else { // read transaction
        raddr = *paddr;

        if (!*penable)
            return ;

        if (r_delay_count) {
            r_delay_count--;

            return ;
        }
        else {
            DATAT read_data = 0;

            if ((uncached_read == NULL) || (!uncached_read(raddr, read_data)))
                for (size_t i = 0; i < sizeof(DATAT); i++) {
                    read_data |= (((DATAT)mem[i + raddr - mem_base_addr]) << (i * 8));
                }

            *prdata = read_data;
            r_delay_count = r_delay; // reset delay value
            w_delay_count = w_delay; // reset delay value

            #ifdef APB_LOG
                printf("[read] %x: %x\n", raddr, read_data); // TODO: %x?
            #endif

            *pready = 1;
        }
    }

    return ;
}

template <typename ADDRT, typename DATAT>
void apb_mem<ADDRT, DATAT>::init(const size_t w_delay , const size_t r_delay, const size_t mem_base_addr, uint8_t* const mem, UNCACHED_WRITE func_ptr0, UNCACHED_READ func_ptr1)
{
    this->w_delay = w_delay;
    this->r_delay = r_delay;

    this->w_delay_count = w_delay;
    this->r_delay_count = r_delay;
    this->mem_base_addr = mem_base_addr;
    this->mem = mem;

    this->uncached_write = func_ptr0;
    this->uncached_read = func_ptr1;

    return ;
}

#endif