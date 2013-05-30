###README###

Sample kernel extension that demonstrates how to hide from kextstat and friends
by modifying the corresponding OSArray containing OSKext objects and the
*\_kmod* list. Both *\_kmod* and *_sLoadedKexts* are located during runtime.
*\_kmod* and *lookupKextWithLoadTag* are resolved on startup. The address of
*_sLoadedKexts* is extracted from *lookupKextWithLoadTag* as shown in the gdb
listing below:

	gdb$ x/32i $loadKextWithTag
	0xffffff80111f5ef0:  55                            push   rbp
	0xffffff80111f5ef1:  48 89 e5                      mov    rbp,rsp
	0xffffff80111f5ef4:  41 57                         push   r15
	0xffffff80111f5ef6:  41 56                         push   r14
	0xffffff80111f5ef8:  41 55                         push   r13
	0xffffff80111f5efa:  41 54                         push   r12
	0xffffff80111f5efc:  53                            push   rbx
	0xffffff80111f5efd:  50                            push   rax
	0xffffff80111f5efe:  89 fb                         mov    ebx,edi
	0xffffff80111f5f00:  48 8b 3d 99 71 2b 00          mov    rdi,QWORD PTR [rip+0x2b7199] # 0xffffff80114ad0a0
	0xffffff80111f5f07:  e8 94 ce 02 00                call   0xffffff8011222da0
	0xffffff80111f5f0c:  48 8b 3d b5 71 2b 00          mov    rdi,QWORD PTR [rip+0x2b71b5] # 0xffffff80114ad0c8 <-- hit!
	0xffffff80111f5f13:  48 8b 07                      mov    rax,QWORD PTR [rdi]
	0xffffff80111f5f16:  ff 90 30 01 00 00             call   QWORD PTR [rax+0x130]
	<snip>


Bugz/ToDo:

* better solution for hardcoded offsets?!

