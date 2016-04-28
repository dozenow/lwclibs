.text
	
.globl lwccreate
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
   mov    $546,%eax
   mov    %rcx,%r10
   syscall
   jnc out_c
   mov    $-1,%rax
out_c:  
   pop    %rbp
   retq
	
.globl lwcsuspendswitch
lwcsuspendswitch:  
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
   mov    $547,%eax
   mov    %rcx,%r10
   syscall
   jnc out_swx
   mov    $-1,%rax
out_swx:  
   pop    %rbp
   retq

.globl lwcdiscardswitch
lwcdiscardswitch:  
   push   %rbp
   mov    %rsp,%rbp
	/*
   mov    %rdi,-0x10(%rbp)
   mov    %esi,-0x14(%rbp)
   mov    %rdx,-0x20(%rbp)
   */
   mov    $548,%eax
   mov    %rcx,%r10
   syscall
   jnc out_dswx
   mov    $-1,%rax
out_dswx:
   pop    %rbp
   retq

.globl lwcoverlay
lwcoverlay: 
   push   %rbp
   mov    %rsp,%rbp
	/*
   mov    %rdi,-0x10(%rbp)
   mov    %esi,-0x14(%rbp)
   mov    %rdx,-0x20(%rbp)
   */
   mov    $549,%eax
   mov    %rcx,%r10
   syscall
   jnc out_ovl
   mov    $-1,%rax
out_ovl:  
   pop    %rbp
   retq




