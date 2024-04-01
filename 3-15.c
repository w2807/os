#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>
#include <sys/mman.h>
#include <fcntl.h>

void collatz(char *p, int n);

int main()
{
	int n;
	scanf("%d", &n);
	int shmfd = shm_open("/myshm", O_CREAT | O_RDWR, 0777);
	ftruncate(shmfd, 4096);
	char *p = mmap(0, 4096, PROT_WRITE, MAP_SHARED, shmfd, 0);
	pid_t pid = fork();
	if (pid == 0)
	{
		collatz(p, n);
	}
	else 
	{
		wait(NULL);
        printf("Sequence: %s\n", p);
	}
	munmap(p, 4096);
	shm_unlink("/myshm");
	return 0;
}

void collatz(char *p, int n)
{
	if (n == 1)
	{
		sprintf(p, "%d\0", n);
		return;
	}
	else if (n % 2 == 0)
	{
		int length = sprintf(p, "%d ", n);
		p += length;
		collatz(p, n / 2);
	}
	else if (n % 2 == 1)
	{
		int length = sprintf(p, "%d ", n);
		p += length;
		collatz(p, 3 * n + 1);
	}
}
