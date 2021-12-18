struct Data
{
	int a;
	int b;
} dt;
struct Data12
{
	int a;
	int b;
	int c;
} dt12;
struct Data Test8()
{
	struct Data dt;
	return dt;
}
struct Data12 Test12()
{
	struct Data12 dt;
	return dt;
}
void TestStructParam(struct Data dt) 
{
	struct Data dt1;
	dt1 = dt;
	return ;
}
int main(int a, char b)
{
	dt = Test8();
	dt12 = Test12();
	TestStructParam(dt);
	return 0;
}
