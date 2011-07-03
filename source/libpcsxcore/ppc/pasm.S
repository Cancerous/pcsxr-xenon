
#if defined (__ppc__) || defined (__ppc64__) || defined (__powerpc__) || (__powerpc64__) || defined __POWERPC__

#if defined (ELF) || defined (__linux__)
#define C(label) label
#else
#define C(label) _##label
#endif

#define OLD_REGISTER_OFFSET	(19*4)
#define SP_SIZE			(OLD_REGISTER_OFFSET+4+8)

/*asm void recRun(register void (*func)(), register u32 hw1, register u32 hw2)*/
.text
.align  4
.globl  C(recRun)
C(recRun):
	/* prologue code */
	mflr	%r0
	stmw	%r13, -(32-13)*4(%r1)
	stw	%r0, 4(%r1)
	stwu	%r1, -((32-13)*4+8)(%r1)
	
	/* execute code */
	mtctr	%r3
	mr	%r31, %r4
	mr	%r30, %r5
	bctrl
/*
}
asm void returnPC()
{*/
.text
.align  4
.globl  C(returnPC)
C(returnPC):
	// end code
	lwz		%r0, (32-13)*4+8+4(%r1)
	addi	%r1, %r1, (32-13)*4+8
	mtlr	%r0
	lmw		%r13, -(32-13)*4(%r1)
	blr
//}*/

#endif
