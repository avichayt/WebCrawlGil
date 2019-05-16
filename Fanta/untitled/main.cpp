#include <stdio.h>
#include <stdlib.h>
#include <link.h>
#include <dlfcn.h>
#include <string.h>
#include <unistd.h>



static int
callback(struct dl_phdr_info *info, size_t size, void *data)
{
    const char *type;
    int p_type, j;

    printf("Name: \"%s\" (%d segments)\n", info->dlpi_name,
           info->dlpi_phnum);

    for (j = 0; j < info->dlpi_phnum; j++) {
        p_type = info->dlpi_phdr[j].p_type;
        type =  (p_type == PT_LOAD) ? "PT_LOAD" :
                (p_type == PT_DYNAMIC) ? "PT_DYNAMIC" :
                (p_type == PT_INTERP) ? "PT_INTERP" :
                (p_type == PT_NOTE) ? "PT_NOTE" :
                (p_type == PT_INTERP) ? "PT_INTERP" :
                (p_type == PT_PHDR) ? "PT_PHDR" :
                (p_type == PT_TLS) ? "PT_TLS" :
                (p_type == PT_GNU_EH_FRAME) ? "PT_GNU_EH_FRAME" :
                (p_type == PT_GNU_STACK) ? "PT_GNU_STACK" :
                (p_type == PT_GNU_RELRO) ? "PT_GNU_RELRO" : NULL;

        printf("    %2d: [%14p; memsz:%7x] flags: 0x%x; ", j,
               (void *) (info->dlpi_addr + info->dlpi_phdr[j].p_vaddr),
               info->dlpi_phdr[j].p_memsz,
               info->dlpi_phdr[j].p_flags);
        if (type != NULL)
            printf("%s\n", type);
        else
            printf("[other (0x%x)]\n", p_type);
    }

    return 0;
}

static void* getStackBase()
{
    unsigned long esp;

    __asm__(
    "mov %%esp, %0"
    : "=r"(esp) : :
    );

    unsigned long offset, base;
    base = esp & ( 0xffffffff << 12 );
    offset = esp - base;
    if (offset != 0)
        base += 0x1000;
    return ((void*) base);

}


int main(int argc, char** argv) {
    void *p, *b, *nb;

    unsigned long esp;

    __asm__(
    "mov %%esp, %0"
    : "=r"(esp) : :
    );

    esp = esp & ( 0xffffffff << 12 );
    printf("stack base: %p\n", getStackBase());


    off_t load_offset;
    for (Elf32_Dyn *dyn = _DYNAMIC; dyn->d_tag != DT_NULL; ++dyn) {
        if (dyn->d_tag == DT_DEBUG) {
            struct r_debug *r_debug = (struct r_debug *) dyn->d_un.d_ptr;
            struct link_map *link_map = r_debug->r_map;
            while (link_map) {
                printf("name: %s\n", link_map->l_name);
                printf("addr: %lx\n", (off_t)link_map->l_addr);
                if (strcmp(link_map->l_name, "/lib32/libc.so.6") == 0) {
                    load_offset = (off_t)link_map->l_addr;
                    break;
                }
                link_map = link_map->l_next;
            }
            break;
        }
    }

    dl_iterate_phdr(callback, NULL);
    getchar();


    return 0;
}