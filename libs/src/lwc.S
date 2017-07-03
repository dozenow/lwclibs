.macro DO_ERR
   mov   %rax,%r12
   callq __error@plt
   mov   %r12, (%rax)
   mov    $-1,%rax
.endm

.text

	/*
global lwccreate:function, lwcswitch:function, 	lwcoverlay:function
global lwcrestrict:function, lwcsyscall:function, lwcgetlwc:function
global lwcjoin:function, lwcleave:function, lwctrycommit:function
*/


.globl lwccreate
.type lwccreate, "function"
lwccreate:
   push   %rbp
   mov    %rsp,%rbp
    /*
   mov    %rdi,-0x10(%rbp)
   mov    %esi,-0x14(%rbp)
   mov    %rdx,-0x20(%rbp)
   mov    %rcx,-0x28(%rbp)
   mov    %r8,-0x30(%rbp)
   mov    %r9d,-0x34(%rbp)
   */
   mov    $550,%eax
   mov    %rcx,%r10
   syscall
   jnc out_c
   DO_ERR
out_c:  
   pop    %rbp
   retq
    
.globl lwcswitch
.type lwcswitch, "function"
lwcswitch:  
   push   %rbp
   mov    %rsp,%rbp
    /*
   mov    %rdi,-0x10(%rbp)
   mov    %esi,-0x14(%rbp)
   mov    %rdx,-0x20(%rbp)
   mov    %rcx,-0x28(%rbp)
   mov    %r8,-0x30(%rbp)
   mov    %r9d,-0x34(%rbp)
   */
   mov    $551,%eax
   mov    %rcx,%r10
   syscall
   jnc out_swx
   DO_ERR
out_swx:  
   pop    %rbp
   retq

.globl lwcoverlay
.type lwcoverlay, "function"
lwcoverlay: 
   push   %rbp
   mov    %rsp,%rbp
    /*
   mov    %rdi,-0x10(%rbp)
   mov    %esi,-0x14(%rbp)
   mov    %rdx,-0x20(%rbp)
   */
   mov    $552,%eax
   mov    %rcx,%r10
   syscall
   jnc out_ovl
   DO_ERR
out_ovl:  
   pop    %rbp
   retq

.globl lwcrestrict
.type lwcrestrict, "function"
lwcrestrict: 
   push   %rbp
   mov    %rsp,%rbp
   /*
   mov    %rdi,-0x10(%rbp)
   mov    %esi,-0x14(%rbp)
   mov    %rdx,-0x20(%rbp)
   */
   mov    $553,%eax
   mov    %rcx,%r10
   syscall
   jnc out_res
   DO_ERR
out_res:  
   pop    %rbp
   retq


.globl lwcsyscall
.type lwcsyscall, "function"
lwcsyscall: 
   push   %rbp
   mov    %rsp,%rbp
   /* four arguments */
   mov    $554,%eax
   mov    %rcx,%r10
   syscall
   jnc out_lwsy
   DO_ERR
out_lwsy:  
   pop    %rbp
   retq

   
.globl lwcgetlwc
.type lwcgetlwc, "function"
lwcgetlwc: 
   push   %rbp
   mov    %rsp,%rbp
   /* one argument */
   mov    $555,%eax
   mov    %rcx,%r10
   syscall
   jnc out_lwget
   DO_ERR
out_lwget:  
   pop    %rbp
   retq


.globl lwcjoin
.type lwcjoin, "function"
lwcjoin: 
   push   %rbp
   mov    %rsp,%rbp
   /* one argument */
   mov    $556,%eax
   mov    %rcx,%r10
   syscall
   jnc out_lwjoin
   DO_ERR
out_lwjoin:
   pop    %rbp
   retq

.globl lwcleave
.type lwcleave, "function"
lwcleave: 
   push   %rbp
   mov    %rsp,%rbp
   /* two arguments */
   mov    $557,%eax
   mov    %rcx,%r10
   syscall
   jnc out_lwlv
   DO_ERR
out_lwlv:
   pop    %rbp
   retq

.globl lwctrycommit
.type lwctrycommit, "function"
lwctrycommit:  
   push   %rbp
   mov    %rsp,%rbp
   /* two arguments */
   mov    $558,%eax
   mov    %rcx,%r10
   syscall
   jnc out_lwtc
   DO_ERR
out_lwsc:
   pop    %rbp
   retq

