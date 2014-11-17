.global swexn_wrapper

.extern sys_swexn

swexn_wrapper:
	pushl 	12(%esi)
	pushl 	8(%esi)
	pushl 	4(%esi)
	pushl 	(%esi)


	call sys_swexn