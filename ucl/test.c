static int g_a;
char g_ch;
struct g_s
{
	int a;
	int b;
} g_Struct;
char printf_hw(int a, int b)
{
	puts("hello world");
	return '0';
}

int main()
{
	int a;
	struct g_s s;
	a = 100;
	printf_hw(3, 4);
	return 1;
}
