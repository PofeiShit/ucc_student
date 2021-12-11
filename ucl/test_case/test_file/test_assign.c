extern int printf(const char *fmt, ...);
int *f(void)
{
	static int number;
	printf("int * f(void) \n");
	return &number;
}
int main(int argc, char *argv[])
{
	int a;
	a = 1;
	*f() += 3;
	*f() += *f() + 3;
	return 0;
}
